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

#include <Daemon>

#include <PkStrings.h>
#include <PkIcons.h>
#include <Enum.h>

#include <QDBusServiceWatcher>
#include <QDBusMessage>

#include <KLocalizedString>
#include <KNotification>
#include <KActionCollection>
#include <KToolInvocation>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(APPER_DAEMON)

#define UPDATES_ICON "system-software-update"

using namespace PackageKit;

Updater::Updater(QObject* parent) :
    QObject(parent),
    m_getUpdatesT(0)
{
    // in case registration fails due to another user or application running
    // keep an eye on it so we can register when available
    auto watcher = new QDBusServiceWatcher(QLatin1String("org.kde.ApperUpdaterIcon"),
                                           QDBusConnection::sessionBus(),
                                           QDBusServiceWatcher::WatchForOwnerChange,
                                           this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &Updater::serviceOwnerChanged);

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
    m_getUpdatesT = Daemon::getUpdates();
    connect(m_getUpdatesT, &Transaction::package, this, &Updater::packageToUpdate);
    connect(m_getUpdatesT, &Transaction::finished, this, &Updater::getUpdateFinished);
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
        auto transaction = qobject_cast<Transaction*>(sender());

        bool different = false;
        if (m_oldUpdateList.size() != m_updateList.size()) {
            different = true;
        } else {
            // The lists have the same size let's make sure
            // all the packages are the same
            const QStringList updates = m_updateList;
            for (const QString &packageId : updates) {
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

        uint updateType = m_configs[QLatin1String(CFG_AUTO_UP)].value<uint>();
        if (m_systemReady && updateType == Enum::All) {
            // update all
            bool ret;
            ret = updatePackages(m_updateList,
                                 false,
                                 QLatin1String("plasmagik"),
                                 i18n("Updates are being automatically installed."));
            if (ret) {
                return;
            }
        } else if (m_systemReady && updateType == Enum::Security && !m_securityList.isEmpty()) {
            // Defaults to security
            bool ret;
            ret = updatePackages(m_securityList,
                                 false,
                                 QLatin1String(UPDATES_ICON),
                                 i18n("Security updates are being automatically installed."));
            if (ret) {
                return;
            }
        } else if (m_systemReady && updateType == Enum::DownloadOnly) {
            // Download all updates
            bool ret;
            ret = updatePackages(m_updateList,
                                 true,
                                 QLatin1String("download"),
                                 i18n("Updates are being automatically downloaded."));
            if (ret) {
                return;
            }
        } else if (!m_systemReady &&
                   (updateType == Enum::All ||
                    updateType == Enum::DownloadOnly ||
                    (updateType == Enum::Security && !m_securityList.isEmpty()))) {
            qCDebug(APPER_DAEMON) << "Not auto updating or downloading, as we might be on battery or mobile connection";
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
    auto notify = new KNotification(QLatin1String("UpdatesComplete"));
    notify->setComponentName(QLatin1String("apperd"));
    if (status == PkTransaction::Success) {
        if (sender()->property("DownloadOnly").toBool()) {
            // We finished downloading show the updates to the user
            showUpdatesPopup();
        } else {
            QIcon icon = QIcon::fromTheme(QLatin1String("task-complete"));
            // use of QSize does the right thing
            notify->setPixmap(icon.pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
            notify->setText(i18n("System update was successful."));
            notify->sendEvent();
        }
    } else {
        QIcon icon = QIcon::fromTheme(QLatin1String("dialog-cancel"));
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
        qCWarning(APPER_DAEMON) << "Message did not receive a reply";
    }

    // This must be called from the main thread...
    KToolInvocation::startServiceByDesktopName(QLatin1String("apper_updates"));
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

    auto notify = new KNotification(QLatin1String("ShowUpdates"), 0, KNotification::Persistent);
    notify->setComponentName(QLatin1String("apperd"));
    connect(notify, &KNotification::action1Activated, this, &Updater::reviewUpdates);
    connect(notify, &KNotification::action2Activated, this, &Updater::installUpdates);
    notify->setTitle(i18np("There is one new update", "There are %1 new updates", m_updateList.size()));
    QString text;
    const QStringList updates = m_updateList;
    for (const QString &packageId : updates) {
        const QString packageName = Transaction::packageName(packageId);
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
    notify->setPixmap(QIcon::fromTheme(QLatin1String("system-software-update")).pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
    notify->sendEvent();
}

bool Updater::updatePackages(const QStringList &packages, bool downloadOnly, const QString &icon, const QString &msg)
{
    m_oldUpdateList = m_updateList;

    // Defaults to security
    auto transaction = new PkTransaction;
    transaction->setProperty("DownloadOnly", downloadOnly);
    transaction->enableJobWatcher(true);
    transaction->updatePackages(packages, downloadOnly);
    connect(transaction, &PkTransaction::finished, this, &Updater::autoUpdatesFinished);
    if (!icon.isNull()) {
        KNotification *notify;
        if (downloadOnly) {
            notify = new KNotification(QLatin1String("DownloadingUpdates"));
        } else {
            notify = new KNotification(QLatin1String("AutoInstallingUpdates"));
        }
        notify->setComponentName(QLatin1String("apperd"));
        notify->setText(msg);
        // use of QSize does the right thing
        notify->setPixmap(QIcon::fromTheme(icon).pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
        notify->sendEvent();
    }

    return true;
}

#include "moc_Updater.cpp"
