/***************************************************************************
 *   Copyright (C) 2009-2018 by Daniel Nicoletti                           *
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

#include "AbstractIsRunning.h"

#include <Daemon>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(APPER_SESSION)

using namespace PackageKit;

AbstractIsRunning::AbstractIsRunning(QObject *parent) :
    QObject(parent)
{
}

AbstractIsRunning::~AbstractIsRunning()
{
}

void AbstractIsRunning::increaseRunning()
{
    m_running++;
    qCDebug(APPER_SESSION) << "Increase running" << m_running;
}

void AbstractIsRunning::decreaseRunning()
{
    m_running--;
    qCDebug(APPER_SESSION) << "Decrease running" << m_running;
    if (!isRunning()) {
        qCDebug(APPER_SESSION) << "All tasks completed";
        emit close();
    }
}

bool AbstractIsRunning::isRunning() const
{
    return m_running > 0;
}

#include "moc_AbstractIsRunning.cpp"
