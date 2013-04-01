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

#ifndef DISTRO_UPGRADE_H
#define DISTRO_UPGRADE_H

#include <QProcess>

#include <Transaction>

using namespace PackageKit;

class DistroUpgrade : public QObject
{
    Q_OBJECT
public:
    explicit DistroUpgrade(QObject *parent = 0);
    ~DistroUpgrade();

    void setConfig(const QVariantHash &configs);

public slots:
    void checkDistroUpgrades();

private slots:
    void distroUpgrade(PackageKit::Transaction::DistroUpgrade type, const QString &name, const QString &description);
    void checkDistroFinished(PackageKit::Transaction::Exit status, uint enlapsed);
    void handleDistroUpgradeAction(uint action);
    void distroUpgradeError(QProcess::ProcessError error);
    void distroUpgradeFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_distroUpgradeProcess;
    Transaction *m_transaction;
    QVariantHash m_configs;
    QStringList m_shownDistroUpgrades;
};

#endif
