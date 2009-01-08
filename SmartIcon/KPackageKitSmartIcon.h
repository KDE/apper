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

#ifndef KPACKAGEKITSMARTICON_H
#define KPACKAGEKITSMARTICON_H

#include <KUniqueApplication>

#include "KpkUpdateIcon.h"
#include "KpkTransactionTrayIcon.h"


namespace kpackagekit {

class KPackageKit_Smart_Icon : public KUniqueApplication
{
Q_OBJECT

public:
    KPackageKit_Smart_Icon();
    virtual ~KPackageKit_Smart_Icon();
    int newInstance();

private:
    KpkTransactionTrayIcon *m_trayIcon;
    KpkUpdateIcon* m_updateIcon;
};

}

#endif
