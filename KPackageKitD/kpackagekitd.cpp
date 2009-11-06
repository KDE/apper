/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
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
#include "../libkpackagekit/KpkEnum.h"

#include <KGenericFactory>
#include <KStandardDirs>
#include <KConfigGroup>
#include <KDirWatch>

#include <QDateTime>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>

#include <limits.h>
#include <solid/networking.h>
#include <solid/acadapter.h>
#include <solid/powermanagement.h>

#define FIVE_MIN 360000

K_PLUGIN_FACTORY(KPackageKitFactory, registerPlugin<KPackageKitD>();)
K_EXPORT_PLUGIN(KPackageKitFactory("kpackagekitd"))

KPackageKitD::KPackageKitD(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent)
{
    m_qtimer = new QTimer(this);
    connect(m_qtimer, SIGNAL(timeout()), this, SLOT(init()));

    // Create a new daemon
    m_client = Client::instance();
    connect(m_client, SIGNAL(transactionListChanged(const QList<PackageKit::Transaction*> &)),
            this, SLOT(transactionListChanged(const QList<PackageKit::Transaction*> &)));

    // Start after 5 minutes, 360000 msec
    // To keep the startup fast..
    m_qtimer->start(FIVE_MIN);
}

KPackageKitD::~KPackageKitD()
{
}

void KPackageKitD::init()
{
    m_qtimer->stop();
    m_qtimer->disconnect();
    connect(m_qtimer, SIGNAL(timeout()), this, SLOT(read()));

    Client::Actions act = m_client->getActions();

    // check to see when the next check update will happen
    // if more that 15 minutes, call show updates
    KConfig config("KPackageKit");
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    // default to one day, 86400 sec
    uint interval = checkUpdateGroup.readEntry("interval", KpkEnum::TimeIntervalDefault);

    // 1160 -> 15 minutes
    if (((m_client->getTimeSinceAction(Client::ActionRefreshCache) - interval > 1160) && interval != 0 )
        || !(act & Client::ActionRefreshCache)) {
        // WE ARE NOT GOING TO REFRESH THE CACHE if it is not the time BUT
        // WE can SHOW the user his system updates :D
        update();
    }

    if (!(act & Client::ActionRefreshCache)) {
        //if the backend does not suport refreshing cache let's don't do nothing
        return;
    }

    read();

    //check if any changes to the file occour
    //this also prevents from reading when a checkUpdate happens
    KDirWatch *confWatch = new KDirWatch(this);
    confWatch->addFile(KStandardDirs::locateLocal("config", "KPackageKit"));
    connect(confWatch, SIGNAL(  dirty(const QString &)), this, SLOT(read()));
    connect(confWatch, SIGNAL(created(const QString &)), this, SLOT(read()));
    connect(confWatch, SIGNAL(deleted(const QString &)), this, SLOT(read()));
    confWatch->startScan();
}

void KPackageKitD::read()
{
    KConfig config("KPackageKit");
    KConfigGroup checkUpdateGroup( &config, "CheckUpdate" );
    // default to one day, 86400 sec
    int interval = checkUpdateGroup.readEntry("interval", KpkEnum::TimeIntervalDefault);
    int actRefreshCache = m_client->getTimeSinceAction(Client::ActionRefreshCache);
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

void KPackageKitD::transactionListChanged(const QList<PackageKit::Transaction*> &tids)
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
