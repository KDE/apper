/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
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

#include "ApperSentinel.h"

#include "RefreshCacheTask.h"
#include "UpdateIcon.h"
#include "DistroUpgrade.h"
#include "TransactionWatcher.h"
#include "DBusInterface.h"
#include "PkInterface.h"

#include <KCmdLineArgs>
#include <KDebug>

#include <Daemon>

#define MINUTE 60000

ApperSentinel::ApperSentinel()
 : KUniqueApplication(),
   m_trayIcon(0),
   m_updateIcon(0),
   m_distroUpgrade(0)
{
    m_pkInterface = new PkInterface(this);
    connect(m_pkInterface, SIGNAL(close()),
            this, SLOT(prepareToClose()));

    QString locale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
    Daemon::setHints("locale=" + locale);

    // this enables not quitting when closing a transaction ui
    setQuitOnLastWindowClosed(false);

    // create the close timer and connect it's signal
    m_closeT = new QTimer(this);
    connect(m_closeT, SIGNAL(timeout()),
            this, SLOT(close()));

    m_trayIcon = new TransactionWatcher(this);
    connect(m_trayIcon, SIGNAL(close()),
            this, SLOT(prepareToClose()));

    m_refreshCache = new RefreshCacheTask(this);
    connect(m_refreshCache, SIGNAL(close()),
            this, SLOT(prepareToClose()));

    m_updateIcon = new UpdateIcon(this);
    connect(m_updateIcon, SIGNAL(close()),
            this, SLOT(prepareToClose()));

    m_distroUpgrade = new DistroUpgrade(this);
    connect(m_distroUpgrade, SIGNAL(close()),
            this, SLOT(prepareToClose()));

    m_interface = new DBusInterface(this);
    // connect the update signal from DBus to our update and distro classes
    connect(m_interface, SIGNAL(checkForUpdates(bool)),
            m_updateIcon, SLOT(checkForUpdates(bool)));
    connect(m_interface, SIGNAL(checkForUpdates(bool)),
            m_distroUpgrade, SLOT(checkDistroUpgrades()));

    connect(m_interface, SIGNAL(refreshCache()),
            m_refreshCache, SLOT(refreshCache()));

    // connect the watch transaction coming from the updater icon to our watcher
//     connect(m_updateIcon, SIGNAL(watchTransaction(const QString &, bool)),
//             m_transWatcher, SLOT(watchTransaction(const QString &, bool)));

    prepareToClose();
}

void ApperSentinel::prepareToClose()
{
    if (isRunning()) {
        kDebug() << "Stoping Timer";
        m_closeT->stop();
    } else {
        kDebug() << "Starting Timer: " << MINUTE;
        m_closeT->start(MINUTE);
    }
}

bool ApperSentinel::isRunning()
{
    // check to see if no piece of code is running
    if (m_trayIcon && m_trayIcon->isRunning()) {
        kDebug() << m_trayIcon;
        return true;
    }
    if (m_refreshCache && m_refreshCache->isRunning()) {
        kDebug() << m_refreshCache;
        return true;
    }
    if (m_updateIcon && m_updateIcon->isRunning()) {
        kDebug() << m_updateIcon;
        return true;
    }
    if (m_distroUpgrade && m_distroUpgrade->isRunning()) {
        kDebug() << m_distroUpgrade;
        return true;
    }

    if (m_pkInterface && m_pkInterface->isRunning()) {
        kDebug() << m_pkInterface;
        return true;
    }

    return false;
}

void ApperSentinel::close()
{
    // This will run when the timer times out, we will check
    // again just to be sure.
    if (!isRunning()) {
        kDebug() << "Closed by Timer";
        quit();
    }
}

int ApperSentinel::newInstance()
{
    return 0;
}

ApperSentinel::~ApperSentinel()
{
}
