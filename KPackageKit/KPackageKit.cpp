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

#include <KGlobal>
#include <KStartupInfo>
#include <KCmdLineArgs>
#include <KStandardDirs>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <QStringList>

#include "KPackageKit.h"

namespace kpackagekit {

KPackageKit::KPackageKit()
 : KUniqueApplication()
{
    // this enables not quiting when closing a transaction ui
    setQuitOnLastWindowClosed(false);

    m_pkNotify = new KpkNotify(this);
    connect(m_pkNotify, SIGNAL( appClose() ), this, SLOT( appClose() ) );

    m_smartUpdate = new KpkSmartUpdate(this);
    connect(m_smartUpdate, SIGNAL( showUpdates() ), m_pkNotify, SLOT( showUpdates() ) );
    connect(m_smartUpdate, SIGNAL( autoUpdatesBeingInstalled(Transaction *) ), m_pkNotify, SLOT( installingAutoUpdates(Transaction *) ) );

    m_pkUi = new KpkUi(this);
    connect(m_pkNotify, SIGNAL( showUpdatesUi() ), m_pkUi, SLOT( showUpdatesUi() ) );
//     connect(m_pkNotify, SIGNAL( showUpdatesTrayIcon() ), m_pkUi, SLOT( showUpdatesTrayIcon() ) );
    connect(m_pkUi, SIGNAL( appClose() ), this, SLOT( appClose() ) );

    m_instFiles = new KpkInstallFiles(this);
    connect(m_instFiles, SIGNAL( appClose() ), this, SLOT( appClose() ) );
}

KPackageKit::~KPackageKit()
{
}

void KPackageKit::appClose()
{
    //check whether we can close
    if ( m_pkNotify->canClose() && m_smartUpdate->canClose() && m_pkUi->canClose() && m_instFiles->canClose() )
	quit();
}

int KPackageKit::newInstance()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if ( args->isSet("smart-update") ) {
	kDebug() << "Smartly handling updates";
	m_smartUpdate->smartUpdate();
    }
    else if ( args->count() ) {
	m_instFiles->installFiles(args);
    }
    else {
	qDebug() << "SHOW UI!";
	m_pkUi->showUi();
    }

    args->clear();
    return 0;
}


}

#include "KPackageKit.moc"
