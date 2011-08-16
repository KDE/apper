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

#ifndef APPER_SENTINEL_H
#define APPER_SENTINEL_H

#include <KUniqueApplication>
#include <QTimer>

class UpdateIcon;
class DistroUpgrade;
class TransactionWatcher;
class DBusInterface;
class PkInterface;

class ApperSentinel : public KUniqueApplication
{
    Q_OBJECT
public:
    ApperSentinel();
    virtual ~ApperSentinel();
    int newInstance();

private slots:
    void prepareToClose();
    void close();

private:
    bool isRunning();
    QTimer *m_closeT;

    TransactionWatcher *m_trayIcon;
    UpdateIcon *m_updateIcon;
    DistroUpgrade *m_distroUpgrade;

    DBusInterface *m_interface;
    PkInterface *m_pkInterface;
};

#endif
