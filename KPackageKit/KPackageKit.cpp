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
#include <KCModuleInfo>

#include "KPackageKit.h"

namespace kpackagekit {

KPackageKit::KPackageKit()
 : KUniqueApplication()
{
    // this enables not quitting when closing a transaction ui
    setQuitOnLastWindowClosed(false);

    m_pkUi = new KCMultiDialog();
    m_pkUi->setCaption( QString() );
    m_pkUi->setWindowIcon( KIcon("applications-other") );
    connect( m_pkUi, SIGNAL( finished() ), this, SLOT ( appClose() ) );
    m_pkUi->addModule( KCModuleInfo::KCModuleInfo("kpk_addrm.desktop") );
    m_pkUi->addModule( KCModuleInfo::KCModuleInfo("kpk_update.desktop") );
    m_pkUi->addModule( KCModuleInfo::KCModuleInfo("kpk_settings.desktop") );
    //connect(m_pkNotify, SIGNAL( showUpdatesUi() ), m_pkUi, SLOT( showUpdatesUi() ) );

    m_instFiles = new KpkInstallFiles(this);
    connect(m_instFiles, SIGNAL( appClose() ), this, SLOT( appClose() ) );
    // register Meta Type so we can queue que connection
    qRegisterMetaType<KUrl::List>("KUrl::List &");
    connect(this, SIGNAL( installFiles(KUrl::List &) ), m_instFiles, SLOT( installFiles(KUrl::List &) ), Qt::QueuedConnection );
}

KPackageKit::~KPackageKit()
{
}

void KPackageKit::appClose()
{
    //check whether we can close
    if ( m_instFiles->canClose() )
	quit();
}

int KPackageKit::newInstance()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if ( args->count() ) {
        // grab the list of files
        KUrl::List urls;
        for ( int i = 0; i < args->count(); i++)
            urls << args->url(i);
        emit installFiles(urls);
    }
    else {
        qDebug() << "SHOW UI!";
        m_pkUi->show();
        m_pkUi->raise();
    }

    args->clear();
    return 0;
}


}

#include "KPackageKit.moc"
