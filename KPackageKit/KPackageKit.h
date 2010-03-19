/***************************************************************************
 *   Copyright (C) 2008-2009 by Daniel Nicoletti                           *
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

#ifndef KPACKAGEKIT_H
#define KPACKAGEKIT_H

#include <KUniqueApplication>

class KpkMainUi;
class KPackageKit : public KUniqueApplication
{
Q_OBJECT

public:
    KPackageKit();
    virtual ~KPackageKit();

    virtual int newInstance();

private slots:
    void appClose();
    void kcmFinished();
    void decreaseAndKillRunning();
    void showUi();
    void showUpdates();
    void showSettings();

private:
    KpkMainUi *m_pkUi;
    void invoke(const QString &method_name, const QStringList &args);

    int m_running;
    bool m_init;
};


#endif
