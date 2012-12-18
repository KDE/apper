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

#include "Updater.h"

#include "ApperdThread.h"

#include <PkStrings.h>
#include <PkIcons.h>
#include <Enum.h>

#include <QDBusServiceWatcher>

#include <KLocale>
#include <KNotification>
#include <KActionCollection>
#include <KMenu>
#include <KToolInvocation>

#include <KDebug>

#define UPDATES_ICON "system-software-update"
#define SYSTEM_READY "system_ready"

using namespace PackageKit;

Updater::Updater(QObject* parent) :
    QObject(parent),
    m_getUpdatesT(0)
{
    // in case registration fails due to another user or application running
    // keep an eye on it so we can register when available
    QDBusServiceWatcher *watcher;
    watcher = new QDBusServiceWatcher(QLatin1String("org.kde.ApperUpdaterIcon"),
                                      QDBusConnection::sessionBus(),
                                      QDBusServiceWatcher::WatchForOwnerChange,
                                      this);
    connect(watcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            this, SLOT(serviceOwnerChanged(QString,QString,QString)));

    m_hasAppletIconified = ApperdThread::nameHasOwner(QLatin1String("org.kde.ApperUpdaterIcon"),
                                                      QDBusConnection::sessionBus());
}

Updater::~Updater()
{
}

void Updater::setConfig(const QVariantHash &configs)
{
    m_configs = configs;
}

void Updater::checkForUpdates(bool system_ready)
{
    uint updateType = m_configs[CFG_AUTO_UP].value<uint>();

    kDebug() << "-------------checkForUpdates-------------" << system_ready;
    // Skip the check if one is already running or
    // the plasmoid is in Icon form and the auto update type is None
    if (m_getUpdatesT || (m_hasAppletIconified && updateType == Enum::None)) {
        kDebug() << "-------------ignoring check-------------" << m_getUpdatesT << m_hasAppletIconified << updateType;
        return;
    }

    m_updateList.clear();
    m_importantList.clear();
    m_securityList.clear();
    m_getUpdatesT = new Transaction(this);
    m_getUpdatesT->setProperty(SYSTEM_READY, system_ready);
    connect(m_getUpdatesT, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this, SLOT(packageToUpdate(PackageKit::Transaction::Info,QString,QString)));
    connect(m_getUpdatesT, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(getUpdateFinished(PackageKit::Transaction::Exit)));
    m_getUpdatesT->getUpdates();
    if (m_getUpdatesT->error()) {
        m_getUpdatesT = 0;
    }
}

void Updater::packageToUpdate(Transaction::Info info, const QString &packageID, const QString &summary)
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

void Updater::getUpdateFinished(PackageKit::Transaction::Exit exit)
{
    m_getUpdatesT = 0;
    if (!m_updateList.isEmpty()) {
        bool different = false;
        if (m_oldUpdateList.size() != m_updateList.size()) {
            different = true;
        } else {
            // The lists have the same size let's make sure
            // all the packages are the same
            foreach (const QString &packageId, m_updateList) {
                if (!m_oldUpdateList.contains(packageId)) {
                    different = true;
                    break;
                }
            }
        }

        // if the lists are the same don't show
        // a notification or try to upgrade again
        if (!different) {
            return;
        }
        m_oldUpdateList = m_updateList;

        // Determine the update type
        UpdateType type = Normal;
        if (!m_securityList.isEmpty()) {
            type = Security;
        } else if (!m_importantList.isEmpty()) {
            type = Important;
        }

        bool systemReady = false;
        uint updateType = m_configs[CFG_AUTO_UP].value<uint>();
        if (exit == Transaction::ExitSuccess) {
            // if get updates failed the system is not ready
            systemReady = sender()->property(SYSTEM_READY).toBool();
        }
        if (!systemReady &&
                (updateType == Enum::All || (updateType == Enum::Security && !m_securityList.isEmpty()))) {
            kDebug() << "Not auto updating packages updates, as we might be on battery or mobile connection";
        }

        if (systemReady && updateType == Enum::All) {
            // update all
            PkTransaction *t = new PkTransaction;
            t->enableJobWatcher(true);
            connect(t, SIGNAL(finished(PkTransaction::ExitStatus)),
                    this, SLOT(autoUpdatesFinished(PkTransaction::ExitStatus)));
            t->setProperty(SYSTEM_READY, systemReady);
            t->updatePackages(m_updateList);
            if (!t->error()) {
                // Force the creation of a transaction Job
                emit watchTransaction(t->tid(), true);

                //autoUpdatesInstalling(t);
                KNotification *notify = new KNotification("AutoInstallingUpdates");
                notify->setComponentData(KComponentData("apperd"));
                notify->setText(i18n("Updates are being automatically installed."));
                // use of QSize does the right thing
                notify->setPixmap(KIcon("plasmagik").pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
                notify->sendEvent();

                emit closeNotification();
                return;
            }
        } else if (systemReady && updateType == Enum::Security && !m_securityList.isEmpty()) {
            // Defaults to security
            PkTransaction *t = new PkTransaction;
            t->enableJobWatcher(true);
            connect(t, SIGNAL(finished(PkTransaction::ExitStatus)),
                    this, SLOT(autoUpdatesFinished(PkTransaction::ExitStatus)));
            t->setProperty(SYSTEM_READY, systemReady);
            t->updatePackages(m_securityList);
            if (!t->error()) {
                // Force the creation of a transaction Job
                emit watchTransaction(t->tid(), true);

                //autoUpdatesInstalling(t);
                KNotification *notify = new KNotification("AutoInstallingUpdates");
                notify->setComponentData(KComponentData("apperd"));
                notify->setText(i18n("Security updates are being automatically installed."));
                // use of QSize does the right thing
                notify->setPixmap(KIcon(UPDATES_ICON).pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
                notify->sendEvent();

                emit closeNotification();
                return;
            }
        }

        // If an erro happened to creathe the auto update
        // transactions show the update list
        QString icon;
        if (type == Important) {
            icon = "security-medium";
        } else if (type == Security) {
            icon = "security-low";
        } else {
            icon = "system-software-update";
        }

        KNotification *notify = new KNotification("ShowUpdates", 0, KNotification::Persistent);
        notify->setComponentData(KComponentData("apperd"));
        connect(notify, SIGNAL(action1Activated()), this, SLOT(showUpdates()));
        connect(this, SIGNAL(closeNotification()), notify, SLOT(close()));
        notify->setTitle(i18np("There is one new update", "There are %1 new updates", m_updateList.size()));
        QStringList names;
        foreach (const QString &packageId, m_updateList) {
            names << Transaction::packageName(packageId);
        }
        QString text = names.join(QLatin1String(", "));
        notify->setText(text);

        QStringList actions;
        actions << i18n("Review");
        notify->setActions(actions);

        // use of QSize does the right thing
        notify->setPixmap(KIcon(icon).pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
        notify->sendEvent();
    } else {
        m_oldUpdateList.clear();
        emit closeNotification();
    }
}

void Updater::autoUpdatesFinished(PkTransaction::ExitStatus status)
{
    KNotification *notify = new KNotification("UpdatesComplete");
    notify->setComponentData(KComponentData("apperd"));
    if (status == PkTransaction::Success) {
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

void Updater::showUpdates()
{
    // This must be called from the main thread...
    KToolInvocation::startServiceByDesktopName("apper_updates");
}

void Updater::serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(service)
    Q_UNUSED(oldOwner)
    m_hasAppletIconified = !newOwner.isEmpty();
}
