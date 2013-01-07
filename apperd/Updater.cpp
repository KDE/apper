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
#include <QDBusMessage>

#include <KLocale>
#include <KNotification>
#include <KActionCollection>
#include <KMenu>
#include <KToolInvocation>

#include <KDebug>

#define UPDATES_ICON "system-software-update"

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

void Updater::setSystemReady()
{
    // System ready changed, maybe we can auto
    // install some updates
    m_systemReady = true;
    getUpdateFinished();
}

void Updater::checkForUpdates(bool systemReady)
{
    m_systemReady = systemReady;

    // Skip the check if one is already running or
    // the plasmoid is in Icon form and the auto update type is None
    if (m_getUpdatesT) {
        return;
    }

    m_updateList.clear();
    m_importantList.clear();
    m_securityList.clear();
    m_getUpdatesT = new Transaction(this);
    connect(m_getUpdatesT, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this, SLOT(packageToUpdate(PackageKit::Transaction::Info,QString,QString)));
    connect(m_getUpdatesT, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(getUpdateFinished()));
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

void Updater::getUpdateFinished()
{
    m_getUpdatesT = 0;
    if (!m_updateList.isEmpty()) {
        Transaction *transaction = qobject_cast<Transaction*>(sender());

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

        // sender is not a transaction when we systemReady has changed
        // if the lists are the same don't show
        // a notification or try to upgrade again
        if (transaction && !different) {
            return;
        }

        uint updateType = m_configs[CFG_AUTO_UP].value<uint>();
        if (m_systemReady && updateType == Enum::All) {
            // update all
            bool ret;
            ret = updatePackages(m_updateList,
                                 false,
                                 "plasmagik",
                                 i18n("Updates are being automatically installed."));
            if (ret) {
                return;
            }
        } else if (m_systemReady && updateType == Enum::Security && !m_securityList.isEmpty()) {
            // Defaults to security
            bool ret;
            ret = updatePackages(m_securityList,
                                 false,
                                 UPDATES_ICON,
                                 i18n("Security updates are being automatically installed."));
            if (ret) {
                return;
            }
        } else if (m_systemReady && updateType == Enum::DownloadOnly) {
            // Download all updates
            bool ret;
            ret = updatePackages(m_updateList,
                                 true,
                                 "download",
                                 i18n("Updates are being automatically downloaded."));
            if (ret) {
                return;
            }
        } else if (!m_systemReady &&
                   (updateType == Enum::All ||
                    updateType == Enum::DownloadOnly ||
                    (updateType == Enum::Security && !m_securityList.isEmpty()))) {
            kDebug() << "Not auto updating or downloading, as we might be on battery or mobile connection";
        }

        // If an erro happened to create the auto update
        // transaction show the update list
        if (transaction) {
            // The transaction is not valid if the systemReady changed
            showUpdatesPopup();
        }
    } else {
        m_oldUpdateList.clear();
    }
}

void Updater::autoUpdatesFinished(PkTransaction::ExitStatus status)
{
    KNotification *notify = new KNotification("UpdatesComplete");
    notify->setComponentData(KComponentData("apperd"));
    if (status == PkTransaction::Success) {
        if (sender()->property("DownloadOnly").toBool()) {
            // We finished downloading show the updates to the user
            showUpdatesPopup();
        } else {
            KIcon icon("task-complete");
            // use of QSize does the right thing
            notify->setPixmap(icon.pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
            notify->setText(i18n("System update was successful."));
            notify->sendEvent();
        }
    } else {
        KIcon icon("dialog-cancel");
        // use of QSize does the right thing
        notify->setPixmap(icon.pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
        notify->setText(i18n("The software update failed."));
        notify->sendEvent();

        // show updates popup
        showUpdatesPopup();
    }
}

void Updater::reviewUpdates()
{
    if (m_hasAppletIconified) {
        QDBusMessage message;
        message = QDBusMessage::createMethodCall(QLatin1String("org.kde.ApperUpdaterIcon"),
                                                 QLatin1String("/"),
                                                 QLatin1String("org.kde.ApperUpdaterIcon"),
                                                 QLatin1String("ReviewUpdates"));
        QDBusMessage reply = QDBusConnection::sessionBus().call(message);
        if (reply.type() == QDBusMessage::ReplyMessage) {
            return;
        }
        kWarning() << "Message did not receive a reply";
    }

    // This must be called from the main thread...
    KToolInvocation::startServiceByDesktopName("apper_updates");
}

void Updater::installUpdates()
{
    bool ret;
    ret = updatePackages(m_updateList, false);
    if (ret) {
        return;
    }

    reviewUpdates();
}

void Updater::serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(service)
    Q_UNUSED(oldOwner)

    m_hasAppletIconified = !newOwner.isEmpty();
}

void Updater::showUpdatesPopup()
{
    m_oldUpdateList = m_updateList;

    KNotification *notify = new KNotification("ShowUpdates", 0, KNotification::Persistent);
    notify->setComponentData(KComponentData("apperd"));
    connect(notify, SIGNAL(action1Activated()), this, SLOT(reviewUpdates()));
    connect(notify, SIGNAL(action2Activated()), this, SLOT(installUpdates()));
    notify->setTitle(i18np("There is one new update", "There are %1 new updates", m_updateList.size()));
    QString text;
    foreach (const QString &packageId, m_updateList) {
        QString packageName = Transaction::packageName(packageId);
        if (text.length() + packageName.length() > 150) {
            text.append(QLatin1String(" ..."));
            break;
        } else if (!text.isNull()) {
            text.append(QLatin1String(", "));
        }
        text.append(packageName);
    }
    notify->setText(text);

    QStringList actions;
    actions << i18n("Review");
    if (m_hasAppletIconified) {
        actions << i18n("Install");
    }
    notify->setActions(actions);

    // use of QSize does the right thing
    notify->setPixmap(KIcon("system-software-update").pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
    notify->sendEvent();
}

bool Updater::updatePackages(const QStringList &packages, bool downloadOnly, const QString &icon, const QString &msg)
{
    m_oldUpdateList = m_updateList;

    // Defaults to security
    PkTransaction *transaction = new PkTransaction;
    transaction->enableJobWatcher(true);
    connect(transaction, SIGNAL(finished(PkTransaction::ExitStatus)),
            this, SLOT(autoUpdatesFinished(PkTransaction::ExitStatus)));
    transaction->setProperty("DownloadOnly", downloadOnly);
    transaction->updatePackages(packages, downloadOnly);
    if (!transaction->error()) {
        if (!icon.isNull()) {
            KNotification *notify;
            if (downloadOnly) {
                notify = new KNotification("DownloadingUpdates");
            } else {
                notify = new KNotification("AutoInstallingUpdates");
            }
            notify->setComponentData(KComponentData("apperd"));
            notify->setText(msg);
            // use of QSize does the right thing
            notify->setPixmap(KIcon(icon).pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
            notify->sendEvent();
        }

        return true;
    }
    return false;
}
