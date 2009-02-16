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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "kpackagekitd.h"
#include "../libkpackagekit/KpkEnum.h"

#include <KGenericFactory>
#include <KStandardDirs>
#include <KConfigGroup>
#include <QDateTime>
#include <limits.h>
#include <solid/networking.h>
#include <solid/acadapter.h>
#include <solid/powermanagement.h>

#define FIVE_MIN 360000

K_PLUGIN_FACTORY(KPackageKitFactory, registerPlugin<KPackageKitD>(); )
K_EXPORT_PLUGIN(KPackageKitFactory("kpackagekitd"))

KPackageKitD::KPackageKitD(QObject *parent, const QList<QVariant> &)
    : KDEDModule(parent), m_refreshCacheT(0)
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
        || !act.contains(Client::ActionRefreshCache)) {
        // WE ARE NOT GOING TO REFRESH THE CACHE if it not time BUT
        // WE can SHOW the user his system update :D
        QProcess::execute("kpackagekit-smart-icon", QStringList() << "--update");
    }

    if (!act.contains(Client::ActionRefreshCache)) {
        //if the backend does not suport refreshing cache let's don't do nothing
        return;
    }

    read();

    //check if any changes to the file occour
    //this also prevents from reading when a checkUpdate happens
    m_confWatch = new KDirWatch(this);
    m_confWatch->addFile(KStandardDirs::locateLocal("config", "KPackageKit"));
    connect(m_confWatch, SIGNAL(dirty(const QString &)),   this, SLOT(read()));
    connect(m_confWatch, SIGNAL(created(const QString &)), this, SLOT(read()));
    connect(m_confWatch, SIGNAL(deleted(const QString &)), this, SLOT(read()));
    m_confWatch->startScan();
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
        checkUpdates();
    } else {
        //check first to see any overflow...
        if ((interval - actRefreshCache) > 4294966) {
            m_qtimer->start(UINT_MAX);
        } else {
            m_qtimer->start((interval - actRefreshCache) * 1000);
        }
    }
}

void KPackageKitD::finished(PackageKit::Transaction::ExitStatus status, uint)
{
    if (status == Transaction::Success) {
        QProcess::execute("kpackagekit-smart-icon", QStringList() << "--update");
    } else {
        // try again in 5 minutes
        m_qtimer->start(FIVE_MIN);
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

void KPackageKitD::checkUpdates()
{
    // check whether system is ready for an updates check
    if (!systemIsReady()) {
        m_qtimer->start(FIVE_MIN);
        return;
    }

    m_refreshCacheT = m_client->refreshCache(true);
    if (m_refreshCacheT == 0) {
        // try again in 5 minutes
        m_qtimer->start(FIVE_MIN);
    } else {
        connect(m_refreshCacheT, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                this, SLOT(finished(PackageKit::Transaction::ExitStatus, uint)));
    }
}

void KPackageKitD::transactionListChanged(const QList<PackageKit::Transaction*> &tids)
{
    if (tids.size()) {
        QProcess::execute("kpackagekit-smart-icon");
    }
}
