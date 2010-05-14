/***************************************************************************
 *   Copyright (C) 2008 by Trever Fischer                                  *
 *   wm161@wm161.net                                                       *
 *   Copyright (C) 2008-2010 by Daniel Nicoletti                           *
 *   dantti85-pk@yahoo.com.br                                              *
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

#include "KpkUpdateIcon.h"

#include <KpkIcons.h>
#include <KpkEnum.h>
#include <KpkMacros.h>

#include <KLocale>
#include <KNotification>
#include <KConfigGroup>
#include <KActionCollection>
#include <KMenu>

#include <KDebug>

#define SECURITY_ICON "tools-wizard"
#define UPDATES_ICON "games-solve"

using namespace PackageKit;

KpkUpdateIcon::KpkUpdateIcon(QObject* parent)
    : KpkAbstractIsRunning(parent),
      m_getingUpdates(false),
      m_statusNotifierItem(0)
{
    connect(Client::instance(), SIGNAL(updatesChanged()), this, SLOT(update()));
}

KpkUpdateIcon::~KpkUpdateIcon()
{
    removeStatusNotifierItem();
}

void KpkUpdateIcon::showSettings()
{
    QProcess::execute("kpackagekit", QStringList() << "--settings");
}

// refresh the cache and try to update,
// if it can't automatically update show
// a notification about updates available
void KpkUpdateIcon::refreshAndUpdate(bool refresh)
{
    // This is really necessary to don't bother the user with
    // tons of popups
    if (refresh) {
        if (!isRunning()) {
            SET_PROXY
            Transaction *t = Client::instance()->refreshCache(true);
            if (!t->error()) { // ignore if there is an error
                // Be silent! don't bother the user if the cache couldn't be refreshed
                connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)), this, SLOT(update()));
                increaseRunning();
            }
        }
    } else {
        update();
    }
}

void KpkUpdateIcon::update()
{
    if (m_getingUpdates) {
        return;
    }

    increaseRunning();
    KConfig config("KPackageKit");
    KConfigGroup notifyGroup(&config, "Notify");
    if (Qt::Checked == static_cast<Qt::CheckState>(notifyGroup.readEntry("notifyUpdates", static_cast<int>(Qt::Checked)))) {
        m_updateList.clear();
        Transaction *t = Client::instance()->getUpdates();
        if (!t->error()) {
            m_getingUpdates = false;
            connect(t, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                    this, SLOT(packageToUpdate(QSharedPointer<PackageKit::Package>)));
            connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                    this, SLOT(getUpdateFinished()));
            return;
        }
    } else {
        removeStatusNotifierItem();
    }
    decreaseRunning();
}

void KpkUpdateIcon::packageToUpdate(QSharedPointer<PackageKit::Package> package)
{
    // Blocked updates are not instalable updates so there is no
    // reason to show/count them
    if (package->info() != Enum::InfoBlocked) {
        m_updateList.append(package);
    }
}

void KpkUpdateIcon::updateStatusNotifierIcon(bool security)
{
    KConfig config("KPackageKit");
    KConfigGroup checkUpdateGroup(&config, "Notify");
    Qt::CheckState updateStatusNotifierIcon = static_cast<Qt::CheckState>(checkUpdateGroup.readEntry("notifyUpdates", static_cast<int>(Qt::Checked)));
    if (updateStatusNotifierIcon == Qt::Unchecked) {
        return;
    }

    QString iconName(security ? SECURITY_ICON : UPDATES_ICON);

    if (!m_statusNotifierItem) {
        m_statusNotifierItem = new KStatusNotifierItem(this);
        m_statusNotifierItem->setCategory(KStatusNotifierItem::SystemServices);
        m_statusNotifierItem->setStatus(KStatusNotifierItem::Active);
        // Remove the EXIT button
        KActionCollection *actions = m_statusNotifierItem->actionCollection();
        actions->removeAction(actions->action(KStandardAction::name(KStandardAction::Quit)));
        // Setup a menu with some actions
        KMenu *menu = new KMenu;
        menu->addTitle(KIcon(iconName), i18n("KPackageKit"));
        QAction *action;
        action = menu->addAction(i18n("Show Updates View"));
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
        connect(m_statusNotifierItem, SIGNAL(activateRequested(bool, const QPoint &)),
                this, SLOT(showUpdates()));
    }

    QString text;
    text = i18np("You have one update", "You have %1 updates", m_updateList.size());
    m_statusNotifierItem->setToolTip(iconName, text, QString());
    m_statusNotifierItem->setIconByName(iconName);

    increaseRunning();
}

void KpkUpdateIcon::getUpdateFinished()
{
    m_getingUpdates = false;
    decreaseRunning();
    if (!m_updateList.isEmpty()) {
        // Store all the security updates
        QList<QSharedPointer<PackageKit::Package> > securityUpdateList;
        foreach(const QSharedPointer<PackageKit::Package> p, m_updateList) {
            if (p->info() > Enum::InfoSecurity) {
                securityUpdateList.append(p);
            }
        }

        KConfig config("KPackageKit");
        KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
        uint updateType = static_cast<uint>(checkUpdateGroup.readEntry("autoUpdate", KpkEnum::AutoUpdateDefault));
        if (updateType == KpkEnum::All) {
            // update all
            SET_PROXY
            // TODO this might be evil
            Transaction *t = Client::instance()->updateSystem(true);
            if (!t->error()) {
                connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                        this, SLOT(autoUpdatesFinished(PackageKit::Enum::Exit)));
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
        } else if (updateType == KpkEnum::Security && !securityUpdateList.isEmpty()) {
            // Defaults to security
            SET_PROXY
            Transaction *t = Client::instance()->updatePackages(true, securityUpdateList);
            if (!t->error()) {
                connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                        this, SLOT(autoUpdatesFinished(PackageKit::Enum::Exit)));
                // don't be interactive to not upset an idle user
                emit watchTransaction(t->tid(), false);
                //autoUpdatesInstalling(t);
                KNotification *autoInstallNotify = new KNotification("AutoInstallingUpdates");
                autoInstallNotify->setText(i18n("Security updates are being automatically installed."));
                // use of QSize does the right thing
                autoInstallNotify->setPixmap(KIcon(SECURITY_ICON).pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
                autoInstallNotify->sendEvent();
                increaseRunning();
                removeStatusNotifierItem();
                return;
            }
        }
        // all failed let's update our icon
        updateStatusNotifierIcon(!securityUpdateList.isEmpty());
    } else {
        removeStatusNotifierItem();
    }
}

void KpkUpdateIcon::autoUpdatesFinished(PackageKit::Enum::Exit status)
{
    // decrease first only because we want to check for updates again
    decreaseRunning();
    KNotification *notify = new KNotification("UpdatesComplete");
    if (status == Enum::ExitSuccess) {
        KIcon icon("task-complete");
        // use of QSize does the right thing
        notify->setPixmap(icon.pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
        notify->setText(i18n("System update was successful."));
        notify->sendEvent();
    } else {
        KIcon icon("dialog-cancel");
        // use of QSize does the right thing
        notify->setPixmap(icon.pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
        notify->setText(i18n("The automated software update failed."));
        notify->sendEvent();
    }
}

void KpkUpdateIcon::showUpdates()
{
    QProcess::execute("kpackagekit", QStringList() << "--updates");
}

void KpkUpdateIcon::removeStatusNotifierItem()
{
    if (m_statusNotifierItem) {
        m_statusNotifierItem->deleteLater();
        m_statusNotifierItem = 0;
    }
}
