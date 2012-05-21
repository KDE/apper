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

#include <Enum.h>

#include <KStandardDirs>
#include <KConfig>
#include <KConfigGroup>
#include <KDirWatch>

#include <Solid/PowerManagement>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusServiceWatcher>
#include <QtDBus/QDBusInterface>

#include <limits.h>

#include <KDebug>

#define FIVE_MIN 360000
#define ONE_MIN   72000

/*
 * What we need:
 * - Refresh the package cache periodicaly (implies listenning to PK's updates changed)
 * - Update or Display Package Updates
 * - Display Distro Upgrades
 * - Start Sentinel to keep an eye on running transactions
 */

ApperdThread::ApperdThread(QObject *parent) :
    QObject(parent),
    m_actRefreshCacheChecked(false),
    m_refreshCacheInterval(Enum::TimeIntervalDefault)
{
    // Make all our init code run on the thread since
    // the DBus calls were made blocking
    QTimer::singleShot(0, this, SLOT(init()));

    m_thread = new QThread(this);
    moveToThread(m_thread);
    m_thread->start();
}

ApperdThread::~ApperdThread()
{
    m_thread->quit();
    m_thread->wait();
    delete m_thread;
}

void ApperdThread::init()
{
    // This will prevent the user seeing updates again,
    // PackageKit emits UpdatesChanges when we should display
    // that information again
    QTimer::singleShot(FIVE_MIN, this, SLOT(updatesChanged()));

    // This timer keeps polling to see if it has
    // to refresh the cache
    m_qtimer = new QTimer(this);
    m_qtimer->setInterval(FIVE_MIN);
    connect(m_qtimer, SIGNAL(timeout()), this, SLOT(poll()));
    m_qtimer->start();

    // Watch for TransactionListChanged so we start sentinel
    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String(""),
                                         QLatin1String("org.freedesktop.PackageKit"),
                                         QLatin1String("TransactionListChanged"),
                                         this,
                                         SLOT(transactionListChanged(QStringList)));

    // Watch for UpdatesChanged so we display new updates
    QDBusConnection::systemBus().connect(QLatin1String(""),
                                         QLatin1String(""),
                                         QLatin1String("org.freedesktop.PackageKit"),
                                         QLatin1String("UpdatesChanged"),
                                         this,
                                         SLOT(updatesChanged()));

    //check if any changes to the file occour
    //this also prevents from reading when a checkUpdate happens
    KDirWatch *confWatch = new KDirWatch(this);
    confWatch->addFile(KStandardDirs::locateLocal("config", "apper"));
    connect(confWatch, SIGNAL(dirty(QString)), this, SLOT(configFileChanged()));
    connect(confWatch, SIGNAL(created(QString)), this, SLOT(configFileChanged()));
    connect(confWatch, SIGNAL(deleted(QString)), this, SLOT(configFileChanged()));
    confWatch->startScan();

    // Make sure we know is Sentinel is running
    QDBusServiceWatcher *watcher;
    watcher = new QDBusServiceWatcher(QLatin1String("org.kde.ApperSentinel"),
                                      QDBusConnection::sessionBus(),
                                      QDBusServiceWatcher::WatchForOwnerChange,
                                      this);
    connect(watcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            this, SLOT(serviceOwnerChanged(QString,QString,QString)));

    // Check if Sentinel is running
    m_sentinelIsRunning = nameHasOwner(QLatin1String("org.kde.ApperSentinel"),
                                       QDBusConnection::sessionBus());

    // if PackageKit is running check to see if there are running transactons already
    if (!m_sentinelIsRunning && nameHasOwner(QLatin1String("org.freedesktop.PackageKit"),
                                             QDBusConnection::systemBus())) {
        QDBusMessage message;
        message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.PackageKit"),
                                                 QLatin1String("/org/freedesktop/PackageKit"),
                                                 QLatin1String("org.freedesktop.PackageKit"),
                                                 QLatin1String("GetTransactionList"));
        QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(message);
        transactionListChanged(reply.value()); // In case of a running transaction fire up sentinel
    }

    // read the current settings
    configFileChanged();
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
    if (m_refreshCacheInterval != Enum::Never) {
        // Find out how many seconds passed since last refresh cache
        uint secsSinceLastRefresh;
        secsSinceLastRefresh = QDateTime::currentDateTime().toTime_t() - m_lastRefreshCache.toTime_t();

        // If lastRefreshCache is null it means that the cache was never refreshed
        if (m_lastRefreshCache.isNull() || secsSinceLastRefresh > m_refreshCacheInterval) {
            KConfig config("apper");
            KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
            bool ignoreBattery;
            bool ignoreMobile;
            ignoreBattery = checkUpdateGroup.readEntry("checkUpdatesOnBattery", false);
            ignoreMobile = checkUpdateGroup.readEntry("checkUpdatesOnMobile", false);
            if (isSystemReady(ignoreBattery, ignoreMobile)) {
                callApperSentinel(QLatin1String("RefreshCache"));
            }

            // Invalidate the last time the cache was refreshed
            m_lastRefreshCache = QDateTime();
        }
    }
}

void ApperdThread::configFileChanged()
{
    KConfig config("apper");
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    uint refreshCacheInterval;
    refreshCacheInterval = checkUpdateGroup.readEntry("interval", Enum::TimeIntervalDefault);
    // Check if the refresh cache interval was changed
    if (m_refreshCacheInterval != refreshCacheInterval) {
        m_refreshCacheInterval = refreshCacheInterval;
        kDebug() << "New refresh cache interval" << m_refreshCacheInterval;
    }
}

void ApperdThread::transactionListChanged(const QStringList &tids)
{
    kDebug() << "tids.size()" << tids.size();
    if (!m_sentinelIsRunning && !tids.isEmpty()) {
        QDBusMessage message;
        message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.DBus"),
                                                 QLatin1String("/"),
                                                 QLatin1String("org.freedesktop.DBus"),
                                                 QLatin1String("StartServiceByName"));
        message << QLatin1String("org.kde.ApperSentinel");
        message << static_cast<uint>(0);
        QDBusConnection::sessionBus().call(message);
    }

    if (tids.isEmpty()) {
        // update the last time the cache was refreshed
        QDateTime lastCacheRefresh;
        lastCacheRefresh = getTimeSinceRefreshCache();
        if (lastCacheRefresh != m_lastRefreshCache) {
            m_lastRefreshCache = lastCacheRefresh;
        }
    }
}

// This is called when the list of updates changes
void ApperdThread::updatesChanged()
{
    KConfig config("apper");
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    bool ignoreBattery;
    bool ignoreMobile;
    ignoreBattery = checkUpdateGroup.readEntry("installUpdatesOnBattery", false);
    ignoreMobile = checkUpdateGroup.readEntry("installUpdatesOnMobile", false);

    // Append the system_ready argument
    QList<QVariant> arguments;
    arguments << isSystemReady(ignoreBattery, ignoreMobile);

    // Make sure the user sees the updates
    callApperSentinel(QLatin1String("CheckForUpdates"), arguments);
}

void ApperdThread::serviceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(serviceName)
    kDebug() << serviceName << oldOwner << newOwner;
    if (newOwner.isEmpty()) {
        // Sentinel has closed
        m_sentinelIsRunning = false;
    } else {
        m_sentinelIsRunning = true;
    }
}

void ApperdThread::callApperSentinel(const QString &method, const QList<QVariant> &arguments)
{
    kDebug() << method;
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.kde.ApperSentinel"),
                                             QLatin1String("/"),
                                             QLatin1String("org.kde.ApperSentinel"),
                                             method);
    message.setArguments(arguments);
    QDBusConnection::sessionBus().call(message);
}

QDateTime ApperdThread::getTimeSinceRefreshCache() const
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.PackageKit"),
                                             QLatin1String("/org/freedesktop/PackageKit"),
                                             QLatin1String("org.freedesktop.PackageKit"),
                                             QLatin1String("GetTimeSinceAction"));
    message << QLatin1String("refresh-cache");
    QDBusReply<uint> reply = QDBusConnection::systemBus().call(message);

    // When the refresh cache value was not yet defined UINT_MAX is returned
    kDebug() << reply.value();
    if (reply.value() == UINT_MAX) {
        return QDateTime();
    } else {
        // Calculate the last time the cache was refreshed by
        // subtracting the seconds from the current time
        return QDateTime::currentDateTime().addSecs(reply.value() * -1);
    }
}

QString ApperdThread::networkState() const
{
    QString ret;
    QDBusInterface packagekitInterface(QLatin1String("org.freedesktop.PackageKit"),
                                       QLatin1String("/org/freedesktop/PackageKit"),
                                       QLatin1String("org.freedesktop.PackageKit"),
                                       QDBusConnection::systemBus());
    if (!packagekitInterface.isValid()) {
        return ret;
    }

    ret = packagekitInterface.property("NetworkState").toString();
    return ret;
}

bool ApperdThread::nameHasOwner(const QString &name, const QDBusConnection &connection) const
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
    if (!ignoreBattery && Solid::PowerManagement::appShouldConserveResources()) {
        kDebug() << "System is not ready, application should conserve resources";
        // FIXME this always return true even on AC
//        return false;
    }

    // TODO it would be nice is Solid provided this
    // so we wouldn't be waking up PackageKit for this Solid task.
    QString netState = networkState();
    // test whether network is connected
    if (netState == QLatin1String("offline") || netState == QLatin1String("unknown")) {
        kDebug() << "System is not ready, network state" << netState;
        return false;
    }

    // check how applications should behave (e.g. on battery power)
    if (!ignoreMobile && netState == QLatin1String("mobile")) {
        kDebug() << "System is not ready, network state" << netState;
        return false;
    }

    return true;
}
