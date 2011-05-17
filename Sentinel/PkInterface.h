/***************************************************************************
 *   Copyright (C) 2008 Daniel Nicoletti <dantti85-pk@yahoo.com.br>        *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef PK_INTERFACE_H
#define PK_INTERFACE_H

#include <QtDBus/QDBusContext>

#include <KpkAbstractIsRunning.h>

class PkInterface : public KpkAbstractIsRunning, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.PackageKit")
public:
    PkInterface(QObject *parent = 0);
    ~PkInterface();

public slots:
    void InstallCatalogs(uint xid, const QStringList &files, const QString &interaction);
    void InstallFontconfigResources(uint xid, const QStringList &resources, const QString &interaction);
    void InstallGStreamerResources(uint xid, const QStringList &resources, const QString &interaction);
    void InstallMimeTypes(uint xid, const QStringList &mime_types, const QString &interaction);
    void InstallPackageFiles(uint xid, const QStringList &files, const QString &interaction);
    void InstallPackageNames(uint xid, const QStringList &packages, const QString &interaction);
    void InstallProvideFiles(uint xid, const QStringList &files, const QString &interaction);
    void RemovePackageByFiles(uint xid, const QStringList &files, const QString &interaction);
    void InstallPrinterDrivers(uint xid, const QStringList &resources, const QString &interaction);
//Query
    bool IsInstalled(const QString &package_name, const QString &interaction);
    bool SearchFile(const QString &file_name, const QString &interaction, QString &package_name);

private:
    QHash<QString, QVariant> parseInteraction(const QString &interaction);
};


#endif
