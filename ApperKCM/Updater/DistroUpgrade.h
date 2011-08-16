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

#include <KTitleWidget>
#include <KUrlLabel>
#include <KProgressDialog>

class DistroUpgrade : public KTitleWidget
{
    Q_OBJECT
public:
    DistroUpgrade(QWidget *parent = 0);
    ~DistroUpgrade();

    void setName(const QString &name);

private slots:
    void startDistroUpgrade();

    void distroUpgradeError(QProcess::ProcessError);
    void distroUpgradeFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    KUrlLabel *m_distroUpgradeUL;
    QProcess *m_distroUpgradeProcess;
    KProgressDialog *m_distroUpgradeDialog;
};

#endif
