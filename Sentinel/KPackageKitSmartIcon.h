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
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifndef KPACKAGEKIT_SMART_ICON_H
#define KPACKAGEKIT_SMART_ICON_H

#include <KUniqueApplication>
#include <QTimer>

class KpkUpdateIcon;
class KpkDistroUpgrade;
class KpkTransactionTrayIcon;
class KpkInterface;
class PkInterface;

namespace kpackagekit {

class KPackageKit_Smart_Icon : public KUniqueApplication
{
Q_OBJECT

public:
    KPackageKit_Smart_Icon();
    virtual ~KPackageKit_Smart_Icon();
    int newInstance();

private slots:
    void prepareToClose();
    void close();

private:
    bool isRunning();
    QTimer *m_closeT;

    KpkTransactionTrayIcon *m_trayIcon;
    KpkUpdateIcon *m_updateIcon;
    KpkDistroUpgrade *m_distroUpgrade;

    KpkInterface *m_interface;
    PkInterface *m_pkInterface;
};

}

#endif
