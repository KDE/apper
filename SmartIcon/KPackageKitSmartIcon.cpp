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

#define MINUTE 5000

using namespace kpackagekit;

KPackageKit_Smart_Icon::KPackageKit_Smart_Icon()
 : KUniqueApplication(),
   m_trayIcon(0),
   m_updateIcon(0),
   m_distroUpgrade(0)
{
    // this enables not quitting when closing a transaction ui
    setQuitOnLastWindowClosed(false);

    // create the close timer and connect it's signal
    m_closeT = new QTimer(this);
    connect(m_closeT, SIGNAL(timeout()),
            this, SLOT(close()));

    m_trayIcon = new KpkTransactionTrayIcon(this);
    connect(m_trayIcon, SIGNAL(close()), this, SLOT(prepareToClose()));
    // This MUST be called after connecting all the signals or slots!
    m_trayIcon->checkTransactionList();

    m_updateIcon = new KpkUpdateIcon(this);
    connect(m_updateIcon, SIGNAL(close()), this, SLOT(prepareToClose()));

    m_distroUpgrade = new KpkDistroUpgrade(this);
    connect(m_distroUpgrade, SIGNAL(close()), this, SLOT(prepareToClose()));

    prepareToClose();
}

void KPackageKit_Smart_Icon::prepareToClose()
{
    if (isRunning()) {
        kDebug() << "Stoping Timer";
        m_closeT->stop();
    } else {
        kDebug() << "Starting Timer: " << MINUTE;
        m_closeT->start(MINUTE);
    }
}

bool KPackageKit_Smart_Icon::isRunning()
{
    // check to see if no piece of code is running
    if (m_trayIcon && m_trayIcon->isRunning()) {
        return true;
    }
    if (m_updateIcon && m_updateIcon->isRunning()) {
        return true;
    }
    if (m_distroUpgrade && m_distroUpgrade->isRunning()) {
        return true;
    }
    return false;
}

void KPackageKit_Smart_Icon::close()
{
    // This will run when the timer times out, we will check
    // again just to be sure.
    if (!isRunning()) {
        kDebug() << "Closed by Timer";
        QTimer::singleShot(1, this, SLOT(quit()));
    }
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

#include "KPackageKitSmartIcon.moc"
