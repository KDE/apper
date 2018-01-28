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

#include <QApplication>

#include <KLocalizedString>

#include <QLoggingCategory>

#include <Daemon>

#define MINUTE 60000

Q_DECLARE_LOGGING_CATEGORY(APPER_SESSION)

using namespace PackageKit;

PkSession::PkSession(QObject* parent) : QObject(parent)
  , m_pkInterface(new PkInterface(this))
{
    connect(m_pkInterface, &PkInterface::close, this, &PkSession::prepareToClose);

    Daemon::global()->setHints(QLatin1String("locale=") + QLocale::system().name() + QLatin1String(".UTF-8"));

    // this enables not quitting when closing a transaction ui
    qApp->setQuitOnLastWindowClosed(false);

    // create the close timer and connect it's signal
    m_closeT = new QTimer(this);
    connect(m_closeT, &QTimer::timeout, this, &PkSession::close);

    prepareToClose();
}

void PkSession::prepareToClose()
{
    if (isRunning()) {
        qCDebug(APPER_SESSION) << "Stoping Timer";
        m_closeT->stop();
    } else {
        qCDebug(APPER_SESSION) << "Starting Timer: " << MINUTE;
        m_closeT->start(MINUTE);
    }
}

bool PkSession::isRunning()
{
    if (m_pkInterface && m_pkInterface->isRunning()) {
        qCDebug(APPER_SESSION) << m_pkInterface;
        return true;
    }

    return false;
}

void PkSession::close()
{
    // This will run when the timer times out, we will check
    // again just to be sure.
    if (!isRunning()) {
        qCDebug(APPER_SESSION) << "Closed by Timer";
        qApp->quit();
    }
}

int PkSession::newInstance()
{
    return 0;
}

PkSession::~PkSession()
{
}

#include "moc_PkSession.cpp"
