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

#ifndef PK_INSTALL_PRINTER_DRIVERS_H
#define PK_INSTALL_PRINTER_DRIVERS_H

#include "KpkAbstractTask.h"

#include <QPackageKit>

using namespace PackageKit;

class PkInstallPrinterDrivers : public KpkAbstractTask
{
Q_OBJECT
public:
    PkInstallPrinterDrivers(uint xid,
                           const QStringList &resources,
                           const QString &interaction,
                           const QDBusMessage &message,
                           QWidget *parent = 0);
    ~PkInstallPrinterDrivers();

public slots:
    void start();

private slots:
    void whatProvidesFinished(PackageKit::Enum::Exit status, uint runtime);
    void addPackage(QSharedPointer<PackageKit::Package>package);

private:
    QList<QSharedPointer<PackageKit::Package> > m_foundPackages;
    QStringList     m_resources;
};

#endif
