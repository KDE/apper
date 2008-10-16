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

#include "KpkUi.h"

#include <KDebug>

namespace kpackagekit {

KpkUi::KpkUi( QObject *parent ) : QObject( parent ),
 m_showingUi(false), m_showingOnlyUpdates(false)
{
}

KpkUi::~KpkUi()
{
}

void KpkUi::showUpdatesUi()
{
    if ( !m_showingUi ) {
	qDebug() << "GO UI!";
	m_kcmMD = new KCMultiDialog();
	m_kcmMD->setCaption( QString() );
	m_kcmMD->setWindowIcon( KIcon("applications-other") );
	connect( m_kcmMD, SIGNAL( finished() ), this, SLOT ( closeUi() ) );
	m_updatePWI = m_kcmMD->addModule( KCModuleInfo::KCModuleInfo("kpk_update.desktop") );
	m_kcmMD->show();
	m_kcmMD->raise();
	m_showingUi = true;
	m_showingOnlyUpdates = true;
    }
    else {
	m_kcmMD->setCurrentPage(m_updatePWI);
	m_kcmMD->activateWindow();
    }
}

void KpkUi::showUi()
{
    if ( !m_showingUi ) {
	qDebug() << "GO UI!";
	m_kcmMD = new KCMultiDialog();
	m_kcmMD->setCaption( QString() );
	m_kcmMD->setWindowIcon( KIcon("applications-other") );
	connect( m_kcmMD, SIGNAL( finished() ), this, SLOT ( closeUi() ) );
	m_kcmMD->addModule( KCModuleInfo::KCModuleInfo("kpk_addrm.desktop") );
	m_updatePWI = m_kcmMD->addModule( KCModuleInfo::KCModuleInfo("kpk_update.desktop") );
	m_kcmMD->addModule( KCModuleInfo::KCModuleInfo("kpk_settings.desktop") );
	m_kcmMD->show();
	m_kcmMD->activateWindow();
	m_kcmMD->raise();
	m_showingUi = true;
    }
    else {
	qDebug() << "RAISE UI!";
	if (m_showingOnlyUpdates) {
	    m_kcmMD->addModule( KCModuleInfo::KCModuleInfo("kpk_addrm.desktop") );
	    m_kcmMD->addModule( KCModuleInfo::KCModuleInfo("kpk_settings.desktop") );
	    m_showingOnlyUpdates = false;
	}
	m_kcmMD->activateWindow();
	m_kcmMD->raise();
    }
}

void KpkUi::closeUi()
{
    m_kcmMD->deleteLater();
    m_showingUi = false;
    m_showingOnlyUpdates = false;
    emit appClose();
}

void KpkUi::showUpdatesTrayIcon(){
    m_updatesTI = new KSystemTrayIcon("security-medium");
    m_updatesTI->show();
}


}

#include "KpkUi.moc"
