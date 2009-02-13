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
#include <KNotification>
#include <QStringList>

#include "KPackageKitSmartIcon.h"


namespace kpackagekit {

KPackageKit_Smart_Icon::KPackageKit_Smart_Icon()
 : KUniqueApplication()
{
    // this enables not quitting when closing a transaction ui
    setQuitOnLastWindowClosed(false);

    m_trayIcon = new KpkTransactionTrayIcon(this);

    // This MUST be called after connecting all the signals or slots!
    m_trayIcon->checkTransactionList();
    m_updateIcon = new KpkUpdateIcon(this);
    m_distroUpgrade = new KpkDistroUpgrade(this);
}

int KPackageKit_Smart_Icon::newInstance()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->isSet("update")) {
        kDebug() << "Running update checker";
        m_updateIcon->checkUpdates();
        m_distroUpgrade->checkDistroUpgrades();
    }
    return 0;
}

KPackageKit_Smart_Icon::~KPackageKit_Smart_Icon()
{
}

}

#include "KPackageKitSmartIcon.moc"
