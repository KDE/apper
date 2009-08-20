/***************************************************************************
 *   Copyright (C) 200 by Daniel Nicoletti                                *
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
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "KpkImportance.h"

#include <KDebug>

int KpkImportance::restartImportance(Client::RestartType type)
{
    switch (type) {
    case Client::UnknownRestartType :
    case Client::RestartNone :
        return 0;
    case Client::RestartApplication :
        return 1;
    case Client::RestartSession :
        return 2;
    case Client::RestartSecuritySession :
        return 3;
    case Client::RestartSystem :
        return 4;
    case Client::RestartSecuritySystem :
        return 5;
    }
    kWarning() << "restart type unrecognised: " << type;
    return 0;
}
