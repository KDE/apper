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

#include "KpkSmartUpdate.h"
#include "../libkpackagekit/KpkEnum.h"

#include <KGlobal>
#include <KLocale>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>

KpkSmartUpdate::KpkSmartUpdate( QObject *parent) :
QObject( parent ), m_running(false)
{
    Client::instance()->setLocale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
}

KpkSmartUpdate::~KpkSmartUpdate()
{
}

void KpkSmartUpdate::smartUpdate()
{
    if (!m_running) {
        m_running = true;
        KConfig config("KPackageKit");
        KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
        m_autoUpdateType = checkUpdateGroup.readEntry("autoUpdate", KpkEnum::AutoUpdateDefault);
        packages.clear();
        if (m_autoUpdateType == KpkEnum::None) {
            emit showUpdates();
            m_running = false;
        } else {
            kDebug() << "PkSmartUpdate";
            Transaction *t = Client::instance()->getUpdates();
            if (!t->error()) {
                connect(t, SIGNAL(package(PackageKit::Package *)),
                        this, SLOT(package(PackageKit::Package *)));
                connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                        this, SLOT(getUpdatesFinished(PackageKit::Transaction::ExitStatus, uint)));
            }
        }
    }
}

void KpkSmartUpdate::getUpdatesFinished(PackageKit::Transaction::ExitStatus status, uint /*runtime*/)
{
    kDebug() << "Status: " << status ;
    if (status == Transaction::Success) {
        if (packages.size() && m_autoUpdateType != KpkEnum::None) {
            if (m_autoUpdateType == KpkEnum::All) {
            kDebug() << "ALL" << packages.size();
                Transaction *t = Client::instance()->updateSystem();
                if (t->error()) {
                    emit showUpdates();
                    m_running = false;
                } else {
                    connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                            this, SLOT(updatesFinished(PackageKit::Transaction::ExitStatus, uint)));
                    emit autoUpdatesBeingInstalled(t);
                }
            } else {
                kDebug() << "Security" << packages.size();
                // Defaults to Security
                Transaction *t = Client::instance()->updatePackages(packages);
                if (t->error()) {
                    emit showUpdates();
                    m_running = false;
                } else {
                    connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                            this, SLOT(updatesFinished(PackageKit::Transaction::ExitStatus, uint)));
                    emit autoUpdatesBeingInstalled(t);
                }
            }
        } else {
            kDebug() << "Show Uptates Only";
            emit showUpdates();
            m_running = false;
        }
    } else {
        m_running = false;
    }
    packages.clear();
    kDebug() << "Running: " << m_running ;
}

void KpkSmartUpdate::updatesFinished(PackageKit::Transaction::ExitStatus status, uint /*runtime*/)
{
    kDebug() << "Status: " << status;
    if (status == Transaction::Success) {
        emit showUpdates();
    }
    m_running = false;
}

void KpkSmartUpdate::package(PackageKit::Package *p)
{
    if (p->state() != Package::Blocked) {
        if ( m_autoUpdateType == KpkEnum::Security && p->state() == Package::Security)
            packages << p;
        else
            packages << p;
    }
}

#include "KpkSmartUpdate.moc"
