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

#include "KpkSmartUpdate.h"

#include <KConfig>
#include <KConfigGroup>

#include <KDebug>

#define NONE 0
#define SECURITY 1
#define ALL 2

KpkSmartUpdate::KpkSmartUpdate( QObject *parent ) :
 QObject( parent ), m_running(false)
{
}

KpkSmartUpdate::~KpkSmartUpdate()
{
}

void KpkSmartUpdate::smartUpdate()
{
    if (!m_running) {
	m_running = true;
	KConfig config("KPackageKit");
	KConfigGroup checkUpdateGroup( &config, "CheckUpdate" );
	m_autoUpdateType = checkUpdateGroup.readEntry( "autoUpdate", SECURITY );
	packages.clear();
	if ( m_autoUpdateType == NONE ) {
	    emit showUpdates();
	    m_running = false;
	}
	else {
	    kDebug() << "PkSmartUpdate";
	    Transaction *t = Client::instance()->getUpdates();
	    connect( t, SIGNAL( package(PackageKit::Package *) ),
		this, SLOT( package(PackageKit::Package *) ) );
	    connect( t, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint) ),
		this, SLOT( getUpdatesFinished(PackageKit::Transaction::ExitStatus, uint) ) );
	}
    }
}

void KpkSmartUpdate::getUpdatesFinished(PackageKit::Transaction::ExitStatus status, uint /*runtime*/)
{
    kDebug() << "Status: " << status ;
    if (status == Transaction::Success) {
	if ( packages.size() && m_autoUpdateType != NONE) {
	    if ( m_autoUpdateType == ALL ) {
	    kDebug() << "ALL" << packages.size();
		if (Transaction *t = Client::instance()->updateSystem() ) {
		    connect( t, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint) ),
			this, SLOT( updatesFinished(PackageKit::Transaction::ExitStatus, uint) ) );
		    emit autoUpdatesBeingInstalled(t);
		}
		else {
		    emit showUpdates();
		    m_running = false;
		}
	    }
	    else {
	    kDebug() << "Security" << packages.size();
		// Defaults to Security
		if ( Transaction *t = Client::instance()->updatePackages(packages) ) {
		    connect( t, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint) ),
			this, SLOT( updatesFinished(PackageKit::Transaction::ExitStatus, uint) ) );
		    emit autoUpdatesBeingInstalled(t);
		}
		else {
		    emit showUpdates();
		    m_running = false;
		}
	    }
	}
	else {
	    kDebug() << "Show Uptates Only";
	    emit showUpdates();
	    m_running = false;
	}
    }
    else
	m_running = false;
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
    if ( p->state() != Package::Blocked ) {
	if ( m_autoUpdateType == SECURITY && p->state() == Package::Security )
	    packages << p;
	else
	    packages << p;
    }
}

#include "KpkSmartUpdate.moc"
