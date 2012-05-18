/***************************************************************************
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

#include "apperd.h"
#include <Enum.h>

#include <KGenericFactory>
#include <KStandardDirs>
#include <KConfigGroup>
#include <KDirWatch>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusServiceWatcher>

#include <limits.h>

#define FIVE_MIN 360000
#define ONE_MIN   72000

K_PLUGIN_FACTORY(ApperFactory, registerPlugin<ApperD>();)
K_EXPORT_PLUGIN(ApperFactory("apperd"))


/*
 * What we need:
 * - Refresh the package cache periodicaly (implies listenning to PK's updates changed)
 * - Update or Display Package Updates
 * - Display Distro Upgrades
 * - Start Sentinel to keep an eye on running transactions
 */

ApperD::ApperD(QObject *parent, const QList<QVariant> &) :
    KDEDModule(parent),
    m_actRefreshCacheChecked(false),
    m_showedUpdates(false),
    m_refreshCacheInterval(Enum::TimeIntervalDefault)
{
    m_qtimer = new QTimer(this);
    connect(m_qtimer, SIGNAL(timeout()), this, SLOT(poll()));
    m_qtimer->setInterval(FIVE_MIN);
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
        QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(message, QDBus::BlockWithGui);
        transactionListChanged(reply.value()); // In case of a running transaction fire up sentinel
    }

    // read the current settings
    configFileChanged();
}

ApperD::~ApperD()
{
}

// This is called every 5 minutes
void ApperD::poll()
{
    if (m_lastRefreshCache.isNull()) {
        // This value wasn't set
        // convert this to QDateTime
        m_lastRefreshCache = getTimeSinceRefreshCache();
    }

    // If check for updates is active
    if (m_refreshCacheInterval != Enum::Never) {
        uint maxTime = QDateTime::currentDateTime().toTime_t() - m_refreshCacheInterval;
        // If lastRefreshCache is null it means that the cache was never refreshed
        if (m_lastRefreshCache.isNull() || m_lastRefreshCache.toTime_t() < maxTime) {
            callApperSentinel(QLatin1String("RefreshCache"));

            // Invalidate the last time the cache was refreshed
            m_lastRefreshCache = QDateTime();
        }
    }

    // Display updates to the user the first time the session starts
    if (!m_showedUpdates) {
        // This will prevent the user seeing updates again,
        // PackageKit emits UpdatesChanges when we should display
        // that information
        m_showedUpdates = true;

        // Show the updates to the users
        updatesChanged();
    }
}

void ApperD::configFileChanged()
{
    KConfig config("apper");
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    uint refreshCacheInterval;
    refreshCacheInterval = checkUpdateGroup.readEntry("interval", Enum::TimeIntervalDefault);
    // Check if the refresh cache interval was changed
    if (m_refreshCacheInterval != refreshCacheInterval) {
        m_refreshCacheInterval = refreshCacheInterval;
        kDebug() << "new refresh cache interval" << m_refreshCacheInterval;
    }
}

void ApperD::transactionListChanged(const QStringList &tids)
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
        QDBusConnection::sessionBus().send(message);
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
void ApperD::updatesChanged()
{
    // Make sure the user sees the updates
    callApperSentinel(QLatin1String("CheckForUpdates"));
}

void ApperD::serviceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner)
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

void ApperD::callApperSentinel(const QString &method)
{
    kDebug() << method;
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.kde.ApperSentinel"),
                                             QLatin1String("/"),
                                             QLatin1String("org.kde.ApperSentinel"),
                                             method);
    QDBusConnection::sessionBus().send(message);
}

QDateTime ApperD::getTimeSinceRefreshCache() const
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.PackageKit"),
                                             QLatin1String("/org/freedesktop/PackageKit"),
                                             QLatin1String("org.freedesktop.PackageKit"),
                                             QLatin1String("GetTimeSinceAction"));
    message << QLatin1String("refresh-cache");
    QDBusReply<uint> reply = QDBusConnection::systemBus().call(message, QDBus::BlockWithGui);

    // When the refresh cache value was not yet defined UINT_MAX is returned
    if (reply.value() == UINT_MAX) {
        return QDateTime();
    } else {
        return QDateTime::currentDateTime().addSecs(reply.value() * -1);
    }
}

bool ApperD::nameHasOwner(const QString &name, const QDBusConnection &connection) const
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.DBus"),
                                             QLatin1String("/"),
                                             QLatin1String("org.freedesktop.DBus"),
                                             QLatin1String("NameHasOwner"));
    message << qVariantFromValue(name);
    QDBusReply<bool> reply = connection.call(message, QDBus::BlockWithGui);
    return reply.value();
}
