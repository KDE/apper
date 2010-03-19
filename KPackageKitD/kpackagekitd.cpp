/***************************************************************************
 *   Copyright (C) 2008-2009 by Daniel Nicoletti                           *
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

#include "kpackagekitd.h"
#include <KpkEnum.h>

#include <KGenericFactory>
#include <KStandardDirs>
#include <KConfigGroup>
#include <KDirWatch>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusReply>

#include <limits.h>
#include <Solid/Networking>
#include <Solid/PowerManagement>

#define FIVE_MIN 360000

K_PLUGIN_FACTORY(KPackageKitFactory, registerPlugin<KPackageKitD>();)
K_EXPORT_PLUGIN(KPackageKitFactory("kpackagekitd"))

KPackageKitD::KPackageKitD(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent),
      m_actRefreshCacheChecked(false)
{
    m_qtimer = new QTimer(this);
    connect(m_qtimer, SIGNAL(timeout()), this, SLOT(init()));

    // Watch fot TransactionListChanged so we call smart icon
    QDBusConnection::systemBus().connect("",
                                         "",
                                         "org.freedesktop.PackageKit",
                                         "TransactionListChanged",
                                         this,
                                         SLOT(transactionListChanged(const QStringList &)));

    // Start after 5 minutes, 360000 msec
    // To keep the startup fast..
    m_qtimer->start(FIVE_MIN);

    //check if any changes to the file occour
    //this also prevents from reading when a checkUpdate happens
    KDirWatch *confWatch = new KDirWatch(this);
    confWatch->addFile(KStandardDirs::locateLocal("config", "KPackageKit"));
    connect(confWatch, SIGNAL(  dirty(const QString &)), this, SLOT(read()));
    connect(confWatch, SIGNAL(created(const QString &)), this, SLOT(read()));
    connect(confWatch, SIGNAL(deleted(const QString &)), this, SLOT(read()));
    confWatch->startScan();
}

KPackageKitD::~KPackageKitD()
{
}

void KPackageKitD::init()
{
    m_qtimer->stop();
    m_qtimer->disconnect();
    connect(m_qtimer, SIGNAL(timeout()), this, SLOT(read()));

    // check to see when the next check update will happen
    // if more that 15 minutes, call show updates
    KConfig config("KPackageKit");
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    // default to one day, 86400 sec
    uint interval = checkUpdateGroup.readEntry("interval", KpkEnum::TimeIntervalDefault);

    if (!canRefreshCache()) {
        //if the backend does not suport refreshing cache let's don't do nothing
        return;
    } else if ((getTimeSinceRefreshCache() - interval > 1160) && interval != 0 ) {
        // 1160 -> 15 minutes
        // WE ARE NOT GOING TO REFRESH THE CACHE if it is not the time BUT
        // WE can SHOW the user his system updates :D
        update();
    }

    read();
}

void KPackageKitD::read()
{
    KConfig config("KPackageKit");
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    // default to one day, 86400 sec
    int interval = checkUpdateGroup.readEntry("interval", KpkEnum::TimeIntervalDefault);
    int actRefreshCache = getTimeSinceRefreshCache();
    if (interval == KpkEnum::Never) {
        return;
    }
    if (actRefreshCache >= interval) {
        refreshAndUpdate();
    } else {
        //check first to see any overflow...
        if ((interval - actRefreshCache) > 4294966) {
            m_qtimer->start(UINT_MAX);
        } else {
            m_qtimer->start((interval - actRefreshCache) * 1000);
        }
    }
}

bool KPackageKitD::systemIsReady()
{
    // test whether network is connected
    if (Solid::Networking::status() != Solid::Networking::Connected  &&
        Solid::Networking::status() != Solid::Networking::Unknown) {
        return false;
    }

    // check how applications should behave (e.g. on battery power)
    if (Solid::PowerManagement::appShouldConserveResources()) {
        return false;
    }

    return true;
}

void KPackageKitD::transactionListChanged(const QStringList &tids)
{
    if (tids.size()) {
        QDBusMessage message;
        message = QDBusMessage::createMethodCall("org.freedesktop.DBus",
                                                 "/",
                                                 "org.freedesktop.DBus",
                                                 QLatin1String("StartServiceByName"));
        message << qVariantFromValue(QString("org.kde.KPackageKitSmartIcon"));
        message << qVariantFromValue((uint) 0);
        QDBusConnection::sessionBus().call(message);
    }
}

void KPackageKitD::refreshAndUpdate()
{
    // check whether system is ready for an update
    if (systemIsReady()) {
        QDBusMessage message;
        message = QDBusMessage::createMethodCall("org.kde.KPackageKitSmartIcon",
                                                 "/",
                                                 "org.kde.KPackageKitSmartIcon",
                                                QLatin1String("RefreshAndUpdate"));
        QDBusConnection::sessionBus().call(message);
    }
    m_qtimer->start(FIVE_MIN);
}

void KPackageKitD::update()
{
    // check whether system is ready for an update
    if (systemIsReady()) {
        QDBusMessage message;
        message = QDBusMessage::createMethodCall("org.kde.KPackageKitSmartIcon",
                                                 "/",
                                                 "org.kde.KPackageKitSmartIcon",
                                                QLatin1String("Update"));
        QDBusConnection::sessionBus().call(message);
    }
}

uint KPackageKitD::getTimeSinceRefreshCache() const
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.freedesktop.PackageKit",
                                             "/org/freedesktop/PackageKit",
                                             "org.freedesktop.PackageKit",
                                             QLatin1String("GetTimeSinceAction"));
    message << QLatin1String("refresh-cache");
    QDBusReply<uint> reply = QDBusConnection::systemBus().call(message);
    return reply.value();
}

bool KPackageKitD::canRefreshCache()
{
    if (m_actRefreshCacheChecked) {
        return m_canRefreshCache;
    }
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.freedesktop.PackageKit",
                                             "/org/freedesktop/PackageKit",
                                             "org.freedesktop.DBus.Properties",
                                             QLatin1String("Get"));
    message << QLatin1String("org.freedesktop.PackageKit");
    message << QLatin1String("Roles");
    QDBusReply<QDBusVariant> reply = QDBusConnection::systemBus().call(message);
    return m_canRefreshCache = reply.value().variant().toString().split(';').contains("refresh-cache");
}
