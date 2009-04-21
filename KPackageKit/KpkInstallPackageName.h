/***************************************************************************
 *   Copyright (C) 2009 by Daniel Nicoletti                                *
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

#ifndef KPK_INSTALL_PACKAGE_NAME_H
#define KPK_INSTALL_PACKAGE_NAME_H

#include <KpkTransaction.h>
#include <KpkAbstractIsRunning.h>

#include <QPackageKit>

using namespace PackageKit;

class KpkInstallPackageName : public KpkAbstractIsRunning
{
Q_OBJECT
public:
    KpkInstallPackageName(const QStringList &args, QObject *parent = 0);
    ~KpkInstallPackageName();

public slots:
    void start();

private slots:
    void kTransactionFinished(KpkTransaction::ExitStatus status);
    void addPackage(PackageKit::Package *package);

private:
    QList<Package*> m_foundPackages;
    QHash <KpkTransaction *,QStringList> m_transactionFiles;
    QStringList m_args;
};

#endif
