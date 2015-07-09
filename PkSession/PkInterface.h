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

#ifndef PK_INTERFACE_H
#define PK_INTERFACE_H

#include <QtDBus/QDBusContext>

#include "AbstractIsRunning.h"

class SessionTask;
class PkInterface : public AbstractIsRunning, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.PackageKit")
public:
    explicit PkInterface(QObject *parent = 0);
    ~PkInterface();

public Q_SLOTS:
    void InstallCatalogs(uint xid, const QStringList &files, const QString &interaction);
    void InstallFontconfigResources(uint xid, const QStringList &resources, const QString &interaction);
    void InstallGStreamerResources(uint xid, const QStringList &resources, const QString &interaction);
    void InstallMimeTypes(uint xid, const QStringList &mime_types, const QString &interaction);
    void InstallPackageFiles(uint xid, const QStringList &files, const QString &interaction);
    void InstallPackageNames(uint xid, const QStringList &packages, const QString &interaction);
    void InstallProvideFiles(uint xid, const QStringList &files, const QString &interaction);
    void RemovePackageByFiles(uint xid, const QStringList &files, const QString &interaction);
    void InstallPrinterDrivers(uint xid, const QStringList &resources, const QString &interaction);
    void InstallResources(uint xid, const QString &type, const QStringList &resources, const QString &interaction);
//Query
    bool IsInstalled(const QString &package_name, const QString &interaction);
    bool SearchFile(const QString &file_name, const QString &interaction, QString &package_name);

private:
    void show(SessionTask *widget) const;
//    QVariantHash parseInteraction(const QString &interaction);
    void InstallPlasmaResources(uint xid, const QStringList &resources, const QString &interaction);
};


#endif
