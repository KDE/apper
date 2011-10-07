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
    m_timeSinceRefreshCache(0)
{
    m_qtimer = new QTimer(this);
    connect(m_qtimer, SIGNAL(timeout()), this, SLOT(poll()));
    // Start after 5 minutes, 360000 msec
    // To keep the startup fast..
    m_qtimer->start(ONE_MIN);

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
    connect(confWatch, SIGNAL(  dirty(QString)), this, SLOT(configFileChanged()));
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
    m_sentinelIsRunning = nameHasOwner(QLatin1String("org.kde.ApperSentinel"), QDBusConnection::sessionBus());

    // if PackageKit is running check to see if there are running transactons already
    if (!m_sentinelIsRunning && nameHasOwner(QLatin1String("org.freedesktop.PackageKit"), QDBusConnection::systemBus())) {
        QDBusMessage message;
        message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.PackageKit"),
                                                 QLatin1String("/org/freedesktop/PackageKit"),
                                                 QLatin1String("org.freedesktop.PackageKit"),
                                                 QLatin1String("GetTransactionList"));
        QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(message);
        transactionListChanged(reply.value()); // In case of a running transaction fire up sentinel
        m_timeSinceRefreshCache = getTimeSinceRefreshCache();
    }

    configFileChanged();
}

ApperD::~ApperD()
{
}

void ApperD::poll()
{
    if (m_timeSinceRefreshCache == 0) {
        // This value wasn't set
        // convert this to QDateTime
        m_timeSinceRefreshCache = getTimeSinceRefreshCache();
    } else {
        // to not keep waking PK we do some sort of calculation
        m_timeSinceRefreshCache += m_qtimer->interval() / 1000;
    }

    // If check for updates is active
    if (m_refreshCacheInterval != Enum::Never) {
        if (m_timeSinceRefreshCache > m_refreshCacheInterval) {
            callApperSentinel(QLatin1String("RefreshAndUpdate"));
            // with this the next cache refresh will be from now to the interval
            m_timeSinceRefreshCache = 1;
        }
    }

    //
    if (!m_sentinelIsRunning) {
        callApperSentinel(QLatin1String("Update"));
    }
}

void ApperD::configFileChanged()
{
    KConfig config("apper");
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    m_refreshCacheInterval = checkUpdateGroup.readEntry("interval", Enum::TimeIntervalDefault);
}

void ApperD::transactionListChanged(const QStringList &tids)
{
    kDebug() << "tids.size()" << tids.size();
    if (!m_sentinelIsRunning && tids.size()) {
        QDBusMessage message;
        message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.DBus"),
                                                 QLatin1String("/"),
                                                 QLatin1String("org.freedesktop.DBus"),
                                                 QLatin1String("StartServiceByName"));
        message << QLatin1String("org.kde.ApperSentinel");
        message << static_cast<uint>(0);
        QDBusConnection::sessionBus().call(message);
    }
}

void ApperD::updatesChanged()
{
    // - update time since last refresh
    // - make sure sentinel is running to display this
    m_timeSinceRefreshCache = getTimeSinceRefreshCache();
    if (!m_sentinelIsRunning && m_showUpdates) {
        // start sentinel
        transactionListChanged(QStringList() << "run");
    }
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
    QDBusConnection::sessionBus().call(message);
}

uint ApperD::getTimeSinceRefreshCache() const
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.PackageKit"),
                                             QLatin1String("/org/freedesktop/PackageKit"),
                                             QLatin1String("org.freedesktop.PackageKit"),
                                             QLatin1String("GetTimeSinceAction"));
    message << QLatin1String("refresh-cache");
    QDBusReply<uint> reply = QDBusConnection::systemBus().call(message);
    return reply.value();
}

bool ApperD::nameHasOwner(const QString &name, const QDBusConnection &connection) const
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
