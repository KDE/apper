/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti <dantti12@gmail.com>           *
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

#include "ApperdThread.h"

#include "RefreshCacheTask.h"
#include "Updater.h"
#include "DistroUpgrade.h"
#include "TransactionWatcher.h"
#include "DBusInterface.h"
#include "RebootListener.h"

#include <Enum.h>
#include <Daemon>

#include <QStandardPaths>
#include <KConfig>
#include <KConfigGroup>
#include <KDirWatch>
#include <KProtocolManager>
#include <KLocalizedString>
//#include <Solid/PowerManagement>
#include <KFormat>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusReply>
#include <QDBusServiceWatcher>

#include <limits.h>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(APPER_DAEMON)

#define FIVE_MIN 360000
#define ONE_MIN   72000

/*
 * What we need:
 * - Refresh the package cache periodicaly (implies listenning to PK's updates changed)
 * - Update or Display Package Updates
 * - Display Distro Upgrades
 * - Start Sentinel to keep an eye on running transactions
 */

using namespace PackageKit;
//using namespace Solid;

ApperdThread::ApperdThread(QObject *parent) :
    QObject(parent),
    m_proxyChanged(true),
    m_AptRebootListener(new AptRebootListener(this))
{
}

ApperdThread::~ApperdThread()
{
}

void ApperdThread::init()
{
//    connect(PowerManagement::notifier(), SIGNAL(appShouldConserveResourcesChanged(bool)),
//            this, SLOT(appShouldConserveResourcesChanged()));

    // This timer keeps polling to see if it has
    // to refresh the cache
    m_qtimer = new QTimer(this);
    m_qtimer->setInterval(FIVE_MIN);
    connect(m_qtimer, &QTimer::timeout, this, &ApperdThread::poll);
    m_qtimer->start();

    //check if any changes to the file occour
    //this also prevents from reading when a checkUpdate happens
    auto confWatch = new KDirWatch(this);
    confWatch->addFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1String("/apper"));
    connect(confWatch, SIGNAL(dirty(QString)), this, SLOT(configFileChanged()));
    connect(confWatch, SIGNAL(created(QString)), this, SLOT(configFileChanged()));
    connect(confWatch, SIGNAL(deleted(QString)), this, SLOT(configFileChanged()));
    confWatch->startScan();

    // Watch for changes in the KDE proxy settings
    auto proxyWatch = new KDirWatch(this);
    confWatch->addFile(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1String("/kioslaverc"));
    connect(proxyWatch, SIGNAL(dirty(QString)), this, SLOT(proxyChanged()));
    connect(proxyWatch, SIGNAL(created(QString)), this, SLOT(proxyChanged()));
    connect(proxyWatch, SIGNAL(deleted(QString)), this, SLOT(proxyChanged()));
    proxyWatch->startScan();

    Daemon::global()->setHints(QLatin1String("locale=") + QLocale::system().name() + QLatin1String(".UTF-8"));

    connect(Daemon::global(), &Daemon::updatesChanged, this, &ApperdThread::updatesChanged);

    m_interface = new DBusInterface(this);

    m_refreshCache = new RefreshCacheTask(this);
    connect(m_interface, &DBusInterface::refreshCache, m_refreshCache, &RefreshCacheTask::refreshCache);

    m_updater = new Updater(this);

    m_distroUpgrade = new DistroUpgrade(this);

    // read the current settings
    configFileChanged();

    // In case PackageKit is not running watch for it's registration to configure proxy
    auto watcher = new QDBusServiceWatcher(QLatin1String("org.freedesktop.PackageKit"),
                                           QDBusConnection::systemBus(),
                                           QDBusServiceWatcher::WatchForRegistration,
                                           this);
    connect(watcher, &QDBusServiceWatcher::serviceRegistered, this, &ApperdThread::setProxy);

    // if PackageKit is running check to see if there are running transactons already
    bool packagekitIsRunning = nameHasOwner(QLatin1String("org.freedesktop.PackageKit"),
                                            QDBusConnection::systemBus());

    m_transactionWatcher = new TransactionWatcher(packagekitIsRunning, this);

    // connect the watch transaction coming from the updater icon to our watcher
    connect(m_interface, &DBusInterface::watchTransaction, m_transactionWatcher, &TransactionWatcher::watchTransactionInteractive);

     // listen to Debian/Apt reboot signals from other sources (apt)
    connect(m_AptRebootListener, &AptRebootListener::requestReboot, m_transactionWatcher, &TransactionWatcher::showRebootNotificationApt);
    QTimer::singleShot(2 /*minutes*/ * 60 /*seconds*/ * 1000 /*msec*/, m_AptRebootListener, SLOT(checkForReboot()));

    if (packagekitIsRunning) {
        // PackageKit is running set the session Proxy
        setProxy();

        // If packagekit is already running go check
        // for updates
        updatesChanged();
    } else {
        // Initial check for updates
        QTimer::singleShot(ONE_MIN, this, SLOT(updatesChanged()));
    }
}

// This is called every 5 minutes
void ApperdThread::poll()
{
    if (m_lastRefreshCache.isNull()) {
        // This value wasn't set
        // convert this to QDateTime
        m_lastRefreshCache = getTimeSinceRefreshCache();
    }

    // If check for updates is active
    if (m_configs[QLatin1String(CFG_INTERVAL)].value<uint>() != Enum::Never) {
        // Find out how many seconds passed since last refresh cache
        qint64 msecsSinceLastRefresh = (QDateTime::currentDateTime().toMSecsSinceEpoch() - m_lastRefreshCache.toMSecsSinceEpoch()) / 1000;

        // If lastRefreshCache is null it means that the cache was never refreshed
        if (m_lastRefreshCache.isNull() || msecsSinceLastRefresh > m_configs[QLatin1String(CFG_INTERVAL)].value<uint>()) {
            bool ignoreBattery = m_configs[QLatin1String(CFG_CHECK_UP_BATTERY)].value<bool>();
            bool ignoreMobile = m_configs[QLatin1String(CFG_CHECK_UP_MOBILE)].value<bool>();
            if (isSystemReady(ignoreBattery, ignoreMobile)) {
                m_refreshCache->refreshCache();
            }

            // Invalidate the last time the cache was refreshed
            m_lastRefreshCache = QDateTime();
        }
    }
}

void ApperdThread::configFileChanged()
{
    KConfig config(QLatin1String("apper"));
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    m_configs[QLatin1String(CFG_CHECK_UP_BATTERY)] = checkUpdateGroup.readEntry(CFG_CHECK_UP_BATTERY, DEFAULT_CHECK_UP_BATTERY);
    m_configs[QLatin1String(CFG_CHECK_UP_MOBILE)] = checkUpdateGroup.readEntry(CFG_CHECK_UP_MOBILE, DEFAULT_CHECK_UP_MOBILE);
    m_configs[QLatin1String(CFG_INSTALL_UP_BATTERY)] = checkUpdateGroup.readEntry(CFG_INSTALL_UP_BATTERY, DEFAULT_INSTALL_UP_BATTERY);
    m_configs[QLatin1String(CFG_INSTALL_UP_MOBILE)] = checkUpdateGroup.readEntry(CFG_INSTALL_UP_MOBILE, DEFAULT_INSTALL_UP_MOBILE);
    m_configs[QLatin1String(CFG_AUTO_UP)] = checkUpdateGroup.readEntry(CFG_AUTO_UP, Enum::AutoUpdateDefault);
    m_configs[QLatin1String(CFG_INTERVAL)] = checkUpdateGroup.readEntry(CFG_INTERVAL, Enum::TimeIntervalDefault);
    m_configs[QLatin1String(CFG_DISTRO_UPGRADE)] = checkUpdateGroup.readEntry(CFG_DISTRO_UPGRADE, Enum::DistroUpgradeDefault);
    m_updater->setConfig(m_configs);
    m_distroUpgrade->setConfig(m_configs);

    KDirWatch *confWatch = qobject_cast<KDirWatch*>(sender());
    if (confWatch) {
        // Check for updates again since the config changed
        updatesChanged();
    }
}

void ApperdThread::proxyChanged()
{
    // We must reparse the configuration since the values are all cached
    KProtocolManager::reparseConfiguration();

    QHash<QString, QString> proxyConfig;
    if (KProtocolManager::proxyType() == KProtocolManager::ManualProxy) {
        proxyConfig[QLatin1String("http")] = KProtocolManager::proxyFor(QLatin1String("http"));
        proxyConfig[QLatin1String("https")] = KProtocolManager::proxyFor(QLatin1String("https"));
        proxyConfig[QLatin1String("ftp")] = KProtocolManager::proxyFor(QLatin1String("ftp"));
        proxyConfig[QLatin1String("socks")] = KProtocolManager::proxyFor(QLatin1String("socks"));
    }

    // Check if the proxy settings really changed to avoid setting them twice
    if (proxyConfig != m_proxyConfig) {
        m_proxyConfig = proxyConfig;
        m_proxyChanged = true;
        setProxy();
    }
}

void ApperdThread::setProxy()
{
    if (!m_proxyChanged) {
        return;
    }

    // If we were called by the watcher it is because PackageKit is running
    bool packagekitIsRunning = true;
    auto watcher = qobject_cast<QDBusServiceWatcher*>(sender());
    if (!watcher) {
        packagekitIsRunning = nameHasOwner(QLatin1String("org.freedesktop.PackageKit"),
                                           QDBusConnection::systemBus());
    }

    if (packagekitIsRunning) {
        // Apply the proxy changes only if packagekit is running
        // use value() to not insert items on the hash
        Daemon::global()->setProxy(m_proxyConfig.value(QLatin1String("http")),
                                   m_proxyConfig.value(QLatin1String("https")),
                                   m_proxyConfig.value(QLatin1String("ftp")),
                                   m_proxyConfig.value(QLatin1String("socks")),
                                   QString(),
                                   QString());
        m_proxyChanged = false;
    }
}

void ApperdThread::updatesChanged()
{
    // update the last time the cache was refreshed
    QDateTime lastCacheRefresh;
    lastCacheRefresh = getTimeSinceRefreshCache();
    if (lastCacheRefresh != m_lastRefreshCache) {
        m_lastRefreshCache = lastCacheRefresh;
    }

    bool ignoreBattery = m_configs[QLatin1String(CFG_INSTALL_UP_BATTERY)].value<bool>();
    bool ignoreMobile = m_configs[QLatin1String(CFG_INSTALL_UP_MOBILE)].value<bool>();

    // Make sure the user sees the updates
    m_updater->checkForUpdates(isSystemReady(ignoreBattery, ignoreMobile));
    m_distroUpgrade->checkDistroUpgrades();
}

void ApperdThread::appShouldConserveResourcesChanged()
{
    bool ignoreBattery = m_configs[QLatin1String(CFG_INSTALL_UP_BATTERY)].value<bool>();
    bool ignoreMobile = m_configs[QLatin1String(CFG_INSTALL_UP_MOBILE)].value<bool>();

    if (isSystemReady(ignoreBattery, ignoreMobile)) {
        m_updater->setSystemReady();
    }
}

QDateTime ApperdThread::getTimeSinceRefreshCache() const
{
    uint value = Daemon::global()->getTimeSinceAction(Transaction::RoleRefreshCache);

    // When the refresh cache value was not yet defined UINT_MAX is returned
    if (value == UINT_MAX) {
        return QDateTime();
    } else {
        // Calculate the last time the cache was refreshed by
        // subtracting the seconds from the current time
        return QDateTime::currentDateTime().addSecs(value * -1);
    }
}

bool ApperdThread::nameHasOwner(const QString &name, const QDBusConnection &connection)
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.DBus"),
                                             QLatin1String("/"),
                                             QLatin1String("org.freedesktop.DBus"),
                                             QLatin1String("NameHasOwner"));
    message << qVariantFromValue(name);
    QDBusReply<bool> reply = connection.call(message);
    return reply.value();
}

bool ApperdThread::isSystemReady(bool ignoreBattery, bool ignoreMobile) const
{
    // First check if we should conserve resources
    // check how applications should behave (e.g. on battery power)
//    if (!ignoreBattery && Solid::PowerManagement::appShouldConserveResources()) {
        qCDebug(APPER_DAEMON) << "System is not ready, application should conserve resources";
        // This was fixed for KDElibs 4.8.5
        return false;
//    }

    // TODO it would be nice is Solid provided this
    // so we wouldn't be waking up PackageKit for this Solid task.
    Daemon::Network network = Daemon::global()->networkState();
    // test whether network is connected
    if (network == Daemon::NetworkOffline || network == Daemon::NetworkUnknown) {
        qCDebug(APPER_DAEMON) << "System is not ready, network state" << network;
        return false;
    }

    // check how applications should behave (e.g. on battery power)
    if (!ignoreMobile && network == Daemon::NetworkMobile) {
        qCDebug(APPER_DAEMON) << "System is not ready, network state" << network;
        return false;
    }

    return true;
}

#include "moc_ApperdThread.cpp"
