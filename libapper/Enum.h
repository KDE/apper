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

#ifndef ENUM_H
#define ENUM_H

#include <config.h>

#ifdef HAVE_AUTOREMOVE
#define AUTOREMOVE true
#else
#define AUTOREMOVE false
#endif // HAVE_AUTOREMOVE

#define CFG_CHECK_UP_BATTERY   "checkUpdatesOnBattery"
#define CFG_CHECK_UP_MOBILE    "checkUpdatesOnMobile"
#define CFG_INSTALL_UP_BATTERY "installUpdatesOnBattery"
#define CFG_INSTALL_UP_MOBILE  "installUpdatesOnMobile"
#define CFG_AUTO_UP            "autoUpdate"
#define CFG_INTERVAL           "interval"
#define CFG_DISTRO_UPGRADE     "distroUpgrade"

#define DEFAULT_CHECK_UP_BATTERY   false
#define DEFAULT_CHECK_UP_MOBILE    false
#define DEFAULT_INSTALL_UP_BATTERY false
#define DEFAULT_INSTALL_UP_MOBILE  false

namespace Enum {

typedef enum {
    DistroNever,
    DistroDevelopment,
    DistroStable
} DistroUpdate;
const int DistroUpgradeDefault = DistroStable;

typedef enum {
    None,
    Security,
    All,
    DownloadOnly
} AutoUpdate;
const int AutoUpdateDefault = None;

typedef enum {
    Never   =       0,
    Hourly  =    3600,
    Daily   =   86400,
    Weekly  =  604800,
    Monthly = 2592000 // 30 days
} TimeInterval;
const int TimeIntervalDefault = Daily;

}

#endif
