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

#include "PkSession.h"

#include "PkInterface.h"

#include <QStringBuilder>

#include <KLocalizedString>
#include <KCmdLineArgs>
#include <KDebug>
#include <KGlobal>

#include <Daemon>

#define MINUTE 60000

using namespace PackageKit;

PkSession::PkSession() :
    KUniqueApplication()
{
    m_pkInterface = new PkInterface(this);
    connect(m_pkInterface, SIGNAL(close()),
            this, SLOT(prepareToClose()));

    QString locale(KGlobal::locale()->language() % QLatin1Char('.') % KGlobal::locale()->encoding());
    Daemon::global()->setHints(QLatin1String("locale=") % locale);

    // this enables not quitting when closing a transaction ui
    setQuitOnLastWindowClosed(false);

    // create the close timer and connect it's signal
    m_closeT = new QTimer(this);
    connect(m_closeT, SIGNAL(timeout()),
            this, SLOT(close()));

    prepareToClose();
}

void PkSession::prepareToClose()
{
    if (isRunning()) {
        kDebug() << "Stoping Timer";
        m_closeT->stop();
    } else {
        kDebug() << "Starting Timer: " << MINUTE;
        m_closeT->start(MINUTE);
    }
}

bool PkSession::isRunning()
{
    if (m_pkInterface && m_pkInterface->isRunning()) {
        kDebug() << m_pkInterface;
        return true;
    }

    return false;
}

void PkSession::close()
{
    // This will run when the timer times out, we will check
    // again just to be sure.
    if (!isRunning()) {
        kDebug() << "Closed by Timer";
        quit();
    }
}

int PkSession::newInstance()
{
    return 0;
}

PkSession::~PkSession()
{
}
