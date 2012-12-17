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

#include "StatusNotifierItem.h"

#include <PkStrings.h>
#include <PkIcons.h>
#include <Enum.h>

#include <KLocale>
#include <KNotification>
#include <KActionCollection>
#include <KMenu>
#include <KToolInvocation>

#include <KDebug>

#include <Daemon>

#define UPDATES_ICON "system-software-update"
#define SYSTEM_READY "system_ready"

using namespace PackageKit;

UpdateIcon::UpdateIcon(QObject* parent) :
    QObject(parent),
    m_getUpdatesT(0),
    m_statusNotifierItem(0)
{
}

UpdateIcon::~UpdateIcon()
{
}

void UpdateIcon::setConfig(const QVariantHash &configs)
{
    m_configs = configs;
}

void UpdateIcon::showSettings()
{
    KToolInvocation::startServiceByDesktopName("apper_settings");
}

void UpdateIcon::checkForUpdates(bool system_ready)
{
    kDebug() << "-------------checkForUpdates-------------" << system_ready;
    // This is really necessary to don't bother the user with
    // tons of popups
    if (m_getUpdatesT) {
        return;
    }

    uint interval = m_configs["interval"].value<uint>();
    uint updateType = m_configs["autoUpdate"].value<uint>();

    // get updates if we should display a notification or automatic update the system
    if (interval != Enum::Never || updateType == Enum::All || updateType == Enum::Security) {
        m_updateList.clear();
        m_importantList.clear();
        m_securityList.clear();
        m_getUpdatesT = new Transaction(this);
        m_getUpdatesT->setProperty(SYSTEM_READY, system_ready);
        connect(m_getUpdatesT, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                this, SLOT(packageToUpdate(PackageKit::Transaction::Info,QString,QString)));
        connect(m_getUpdatesT, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
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
}

void UpdateIcon::packageToUpdate(Transaction::Info info, const QString &packageID, const QString &summary)
{
    Q_UNUSED(summary)

    switch (info) {
    case Transaction::InfoBlocked:
        // Blocked updates are not instalable updates so there is no
        // reason to show/count them
        return;
    case Transaction::InfoImportant:
        m_importantList << packageID;
        break;
    case Transaction::InfoSecurity:
        m_securityList << packageID;
        break;
    default:
        break;
    }
    m_updateList << packageID;
}

void UpdateIcon::updateStatusNotifierIcon(UpdateType type)
{
//    if (!m_statusNotifierItem) {
//        m_statusNotifierItem = new StatusNotifierItem(this);
//        // Setup a menu with some actions
//        KMenu *menu = new KMenu;
//        menu->addTitle(KIcon(UPDATES_ICON), i18n("Apper"));
//        QAction *action;
//        action = menu->addAction(i18n("Review Updates"));
//        connect(action, SIGNAL(triggered(bool)),
//                this, SLOT(showUpdates()));
//        action = menu->addAction(i18n("Configure"));
//        connect(action, SIGNAL(triggered(bool)),
//                this, SLOT(showSettings()));
//        menu->addSeparator();
//        action = menu->addAction(i18n("Hide"));
//        connect(action, SIGNAL(triggered(bool)),
//                this, SLOT(removeStatusNotifierItem()));
//        m_statusNotifierItem->setContextMenu(menu);
//        // Show updates on the left click
//        connect(m_statusNotifierItem, SIGNAL(activateRequested(bool,QPoint)),
//                this, SLOT(showUpdates()));
//    }

//    QString text;
//    text = i18np("You have one update", "You have %1 updates", m_updateList.size());
//    m_statusNotifierItem->setToolTip(UPDATES_ICON, text, QString());

//    QString icon;
//    if (type == Important) {
//        icon = "kpackagekit-important";
//    } else if (type == Security) {
//        icon = "kpackagekit-security";
//    } else {
//        icon = "kpackagekit-updates";
//    }
//    m_statusNotifierItem->setIconByName(icon);
}

void UpdateIcon::getUpdateFinished()
{
    m_getUpdatesT = 0;
    if (!m_updateList.isEmpty()) {
        // Store all the security updates
        UpdateType type = Normal;
        if (!m_securityList.isEmpty()) {
            type = Security;
        } else if (!m_importantList.isEmpty()) {
            type = Important;
        }

        bool systemReady;
        uint updateType = m_configs["autoUpdate"].value<uint>();
        systemReady = sender()->property(SYSTEM_READY).toBool();
        if (!systemReady &&
                (updateType == Enum::All || (updateType == Enum::Security && !m_securityList.isEmpty()))) {
            kDebug() << "Not auto updating packages updates, as we might be on battery or mobile connection";
        }

        if (systemReady && updateType == Enum::All) {
            // update all
            Transaction *t = new Transaction(this);
            connect(t, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                    this, SLOT(autoUpdatesFinished(PackageKit::Transaction::Exit)));
            t->setProperty(SYSTEM_READY, systemReady);
            t->updatePackages(m_updateList, Transaction::TransactionFlagOnlyTrusted);
            if (!t->error()) {
                // Force the creation of a transaction Job
                emit watchTransaction(t->tid(), true);

                //autoUpdatesInstalling(t);
                KNotification *autoInstallNotify = new KNotification("AutoInstallingUpdates");
                autoInstallNotify->setText(i18n("Updates are being automatically installed."));
                // use of QSize does the right thing
                autoInstallNotify->setPixmap(KIcon("plasmagik").pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
                autoInstallNotify->sendEvent();

                removeStatusNotifierItem();
                return;
            }
        } else if (systemReady && updateType == Enum::Security && !m_securityList.isEmpty()) {
            // Defaults to security
            Transaction *t = new Transaction(this);
            connect(t, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                    this, SLOT(autoUpdatesFinished(PackageKit::Transaction::Exit)));
            t->setProperty(SYSTEM_READY, systemReady);
            t->updatePackages(m_securityList, Transaction::TransactionFlagOnlyTrusted);
            if (!t->error()) {
                // Force the creation of a transaction Job
                emit watchTransaction(t->tid(), true);

                //autoUpdatesInstalling(t);
                KNotification *autoInstallNotify = new KNotification("AutoInstallingUpdates");
                autoInstallNotify->setText(i18n("Security updates are being automatically installed."));
                // use of QSize does the right thing
                autoInstallNotify->setPixmap(KIcon(UPDATES_ICON).pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
                autoInstallNotify->sendEvent();

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
    KNotification *notify = new KNotification("UpdatesComplete");
    if (status == Transaction::ExitSuccess) {
        KIcon icon("task-complete");
        // use of QSize does the right thing
        notify->setPixmap(icon.pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
        notify->setText(i18n("System update was successful."));
        notify->sendEvent();
        
        // run get-updates again so that non auto-installed updates can be displayed
        checkForUpdates(sender()->property(SYSTEM_READY).toBool());
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
    KToolInvocation::startServiceByDesktopName("apper_updates");
}

void UpdateIcon::removeStatusNotifierItem()
{
    if (m_statusNotifierItem) {
        m_statusNotifierItem->deleteLater();
        m_statusNotifierItem = 0;
    }
}
