/***************************************************************************
 *   Copyright (C) 2009-2010 by Daniel Nicoletti                           *
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

#ifndef KPK_DISTRO_UPGRADE_H
#define KPK_DISTRO_UPGRADE_H

#include "AbstractIsRunning.h"

#include <QProcess>

#include <Transaction>

using namespace PackageKit;

class KpkDistroUpgrade : public AbstractIsRunning
{
Q_OBJECT
public:
    KpkDistroUpgrade(QObject *parent = 0);
    ~KpkDistroUpgrade();

public slots:
    void checkDistroUpgrades();

private slots:
    void distroUpgrade(PackageKit::Transaction::DistroUpgrade type, const QString &name, const QString &description);
    void handleDistroUpgradeAction(uint action);
    void distroUpgradeError(QProcess::ProcessError error);
    void distroUpgradeFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_distroUpgradeProcess;
};

#endif
