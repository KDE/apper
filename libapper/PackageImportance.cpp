/***************************************************************************
 *   Copyright (C) 2009-2011 by Daniel Nicoletti                           *
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

#include "PackageImportance.h"

#include <KDebug>

int PackageImportance::restartImportance(Transaction::Restart type)
{
    switch (type) {
    case Transaction::RestartUnknown:
    case Transaction::RestartNone:
        return 0;
    case Transaction::RestartApplication:
        return 1;
    case Transaction::RestartSession:
        return 2;
    case Transaction::RestartSecuritySession:
        return 3;
    case Transaction::RestartSystem:
        return 4;
    case Transaction::RestartSecuritySystem:
        return 5;
    }
    kWarning() << "restart type unrecognised: " << type;
    return 0;
}
