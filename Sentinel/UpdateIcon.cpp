/***************************************************************************
 *   Copyright (C) 2008 by Trever Fischer                                  *
 *   wm161@wm161.net                                                       *
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "UpdateIcon.h"

#include <PkStrings.h>
#include <PkIcons.h>
#include <Macros.h>
#include <Enum.h>

#include <KLocale>
#include <KNotification>
#include <KConfigGroup>
#include <KActionCollection>
#include <KMenu>
#include <KToolInvocation>

#include <Solid/PowerManagement>

#include <KDebug>

#include <Daemon>

#define UPDATES_ICON "system-software-update"

using namespace PackageKit;

UpdateIcon::UpdateIcon(QObject* parent)
    : AbstractIsRunning(parent),
      m_getUpdatesT(0),
      m_statusNotifierItem(0)
{
    connect(Daemon::global(), SIGNAL(updatesChanged()), this, SLOT(update()));
}

UpdateIcon::~UpdateIcon()
{
    removeStatusNotifierItem();
}

void UpdateIcon::showSettings()
{
    KToolInvocation::startServiceByDesktopName("Apper", QStringList() << "--settings");
}

void UpdateIcon::refresh(bool update)
{
    if (!systemIsReady(true)) {
        kDebug() << "Not checking for updates, as we might be on battery or mobile connection";
        return;
    }

    if (!isRunning()) {
        SET_PROXY
        increaseRunning();
        Transaction *t = new Transaction(this);
        // ignore if there is an error
        // Be silent! don't bother the user if the cache couldn't be refreshed
        if (update) {
            connect(t, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
                    this, SLOT(update()));
        }
        connect(t, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
                this, SLOT(decreaseRunning()));
        t->refreshCache(true);
        if (t->error()) {
            KNotification *notify = new KNotification("TransactionError", 0);
            notify->setText(PkStrings::daemonError(t->error()));
            notify->setPixmap(KIcon("dialog-error").pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
            notify->sendEvent();
        }
    }
}
    
// refresh the cache and try to update,
// if it can't automatically update show
// a notification about updates available
void UpdateIcon::refreshAndUpdate(bool doRefresh)
{
    // This is really necessary to don't bother the user with
    // tons of popups
    if (doRefresh) {
        // Force an update after refresh cache
        refresh(true);
    } else {
        update();
    }
}

void UpdateIcon::update()
{
    if (m_getUpdatesT) {
        return;
    }

    increaseRunning();
    KConfig config("apper");
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    uint interval = static_cast<uint>(checkUpdateGroup.readEntry("interval", Enum::TimeIntervalDefault));
    uint updateType = static_cast<uint>(checkUpdateGroup.readEntry("autoUpdate", Enum::AutoUpdateDefault));

    // get updates if we should display a notification or automatic update the system
    if (interval != Enum::Never || updateType == Enum::All || updateType == Enum::Security) {
        m_updateList.clear();
        m_getUpdatesT = new Transaction(this);
        connect(m_getUpdatesT, SIGNAL(package(PackageKit::Package)),
                this, SLOT(packageToUpdate(PackageKit::Package)));
        connect(m_getUpdatesT, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
                this, SLOT(getUpdateFinished()));
        m_getUpdatesT->getUpdates();
        if (m_getUpdatesT->error()) {
            m_getUpdatesT = 0;
        } else {
            return;
        }
    } else {
        removeStatusNotifierItem();
    }
    decreaseRunning();
}

void UpdateIcon::packageToUpdate(const Package &package)
{
    // Blocked updates are not instalable updates so there is no
    // reason to show/count them
    if (package.info() != Package::InfoBlocked) {
        m_updateList.append(package);
    }
}

void UpdateIcon::updateStatusNotifierIcon(UpdateType type)
{
    if (!m_statusNotifierItem) {
        m_statusNotifierItem = new KStatusNotifierItem(this);
        m_statusNotifierItem->setCategory(KStatusNotifierItem::SystemServices);
        m_statusNotifierItem->setStatus(KStatusNotifierItem::Active);
        // Remove the EXIT button
        KActionCollection *actions = m_statusNotifierItem->actionCollection();
        actions->removeAction(actions->action(KStandardAction::name(KStandardAction::Quit)));
        // Setup a menu with some actions
        KMenu *menu = new KMenu;
        menu->addTitle(KIcon(UPDATES_ICON), i18n("Apper"));
        QAction *action;
        action = menu->addAction(i18n("Review Updates"));
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(showUpdates()));
        action = menu->addAction(i18n("Configure"));
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(showSettings()));
        menu->addSeparator();
        action = menu->addAction(i18n("Hide"));
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(removeStatusNotifierItem()));
        m_statusNotifierItem->setContextMenu(menu);
        // Show updates on the left click
        connect(m_statusNotifierItem, SIGNAL(activateRequested(bool, QPoint)),
                this, SLOT(showUpdates()));
    }

    QString text;
    text = i18np("You have one update", "You have %1 updates", m_updateList.size());
    m_statusNotifierItem->setToolTip(UPDATES_ICON, text, QString());

    QString icon;
    if (type == Important) {
        icon = "kpackagekit-important";
    } else if (type == Security) {
        icon = "kpackagekit-security";
    } else {
        icon = "kpackagekit-updates";
    }
    m_statusNotifierItem->setIconByName(icon);

    increaseRunning();
}

void UpdateIcon::getUpdateFinished()
{
    m_getUpdatesT = 0;
    decreaseRunning();
    if (!m_updateList.isEmpty()) {
        // Store all the security updates
        UpdateType type = Normal;
        QList<Package> securityUpdateList;
        foreach(const Package &p, m_updateList) {
            if (p.info() == Package::InfoSecurity) {
                securityUpdateList.append(p);
                type = Security;
            } else if (type == Normal && p.info() == Package::InfoImportant) {
                type = Important;
            }
        }

        KConfig config("apper");
        KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
        uint updateType = static_cast<uint>(checkUpdateGroup.readEntry("autoUpdate", Enum::AutoUpdateDefault));
        bool systemReady = systemIsReady(true);
        if (!systemReady && (updateType == Enum::All || (updateType == Enum::Security && !securityUpdateList.isEmpty()))) {
            kDebug() << "Not auto updating packages updates, as we might be on battery or mobile connection";
        }

        if (systemReady && updateType == Enum::All) {
            // update all
            SET_PROXY
            Transaction *t = new Transaction(this);
            connect(t, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
                    this, SLOT(autoUpdatesFinished(PackageKit::Transaction::Exit)));
            t->updatePackages(m_updateList, true);
            if (!t->error()) {
                // don't be interactive to not upset an idle user
                emit watchTransaction(t->tid(), false);
                //autoUpdatesInstalling(t);
                KNotification *autoInstallNotify = new KNotification("AutoInstallingUpdates");
                autoInstallNotify->setText(i18n("Updates are being automatically installed."));
                // use of QSize does the right thing
                autoInstallNotify->setPixmap(KIcon("plasmagik").pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
                autoInstallNotify->sendEvent();

                increaseRunning();
                removeStatusNotifierItem();
                return;
            }
        } else if (systemReady && updateType == Enum::Security && !securityUpdateList.isEmpty()) {
            // Defaults to security
            SET_PROXY
            Transaction *t = new Transaction(this);
            connect(t, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
                    this, SLOT(autoUpdatesFinished(PackageKit::Transaction::Exit)));
            t->updatePackages(securityUpdateList, true);
            if (!t->error()) {
                // don't be interactive to not upset an idle user
                emit watchTransaction(t->tid(), false);
                //autoUpdatesInstalling(t);
                KNotification *autoInstallNotify = new KNotification("AutoInstallingUpdates");
                autoInstallNotify->setText(i18n("Security updates are being automatically installed."));
                // use of QSize does the right thing
                autoInstallNotify->setPixmap(KIcon(UPDATES_ICON).pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
                autoInstallNotify->sendEvent();

                increaseRunning();
                removeStatusNotifierItem();
                return;
            }
        }

        // all failed let's update our icon
        updateStatusNotifierIcon(type);
    } else {
        removeStatusNotifierItem();
    }
}

void UpdateIcon::autoUpdatesFinished(PackageKit::Transaction::Exit status)
{
    // decrease first only because we want to check for updates again
    decreaseRunning();
    KNotification *notify = new KNotification("UpdatesComplete");
    if (status == Transaction::ExitSuccess) {
        KIcon icon("task-complete");
        // use of QSize does the right thing
        notify->setPixmap(icon.pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
        notify->setText(i18n("System update was successful."));
        notify->sendEvent();
        
        // run get-updates again so that non auto-installed updates can be displayed
        update();
    } else {
        KIcon icon("dialog-cancel");
        // use of QSize does the right thing
        notify->setPixmap(icon.pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
        notify->setText(i18n("The automated software update failed."));
        notify->sendEvent();
    }
}

void UpdateIcon::showUpdates()
{
    KToolInvocation::startServiceByDesktopName("Apper", QStringList() << "--updates");
}

void UpdateIcon::removeStatusNotifierItem()
{
    if (m_statusNotifierItem) {
        m_statusNotifierItem->deleteLater();
        m_statusNotifierItem = 0;
    }
}

bool UpdateIcon::systemIsReady(bool checkUpdates)
{
    Daemon::Network networkState = Daemon::networkState();

    // test whether network is connected
    if (networkState == Daemon::NetworkOffline || networkState == Daemon::UnknownNetwork) {
        kDebug() << "nerwork state" << networkState;
        return false;
    }

    KConfig config("apper");
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    bool ignoreBattery;
    if (checkUpdates) {
        ignoreBattery = checkUpdateGroup.readEntry("checkUpdatesOnBattery", false);
    } else {
        ignoreBattery = checkUpdateGroup.readEntry("installUpdatesOnBattery", false);
    }

    kDebug() << "conserve" <<  Solid::PowerManagement::appShouldConserveResources();
    // THIS IS NOT working on my computer
    // check how applications should behave (e.g. on battery power)
//    if (!ignoreBattery && Solid::PowerManagement::appShouldConserveResources()) {
//        return false;
//    }
    
    bool ignoreMobile;
    if (checkUpdates) {
        ignoreMobile = checkUpdateGroup.readEntry("checkUpdatesOnMobile", false);
    } else {
        ignoreMobile = checkUpdateGroup.readEntry("installUpdatesOnMobile", false);
    }

    // check how applications should behave (e.g. on battery power)
    if (!ignoreMobile && networkState == Daemon::NetworkMobile) {
        return false;
    } 

    return true;
}
