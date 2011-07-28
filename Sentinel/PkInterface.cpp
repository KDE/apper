/***************************************************************************
 *   Copyright (C) 2008-2010 Daniel Nicoletti <dantti85-pk@yahoo.com.br>   *
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

#include "PkInterface.h"
#include "packagekitadaptor.h"

#include <QtDBus/QDBusConnection>
#include <KpkStrings.h>
#include <KpkMacros.h>
#include <KIcon>
#include <KNotification>

#include "PkInstallPackageNames.h"
#include "PkInstallMimeTypes.h"
#include "PkInstallGStreamerResources.h"
#include "PkInstallFontconfigResources.h"
#include "PkInstallPlasmaResources.h"
#include "PkInstallPackageFiles.h"
#include "PkInstallProvideFiles.h"
#include "PkInstallCatalogs.h"
#include "PkRemovePackageByFiles.h"
#include "PkInstallPrinterDrivers.h"

#include "PkIsInstalled.h"
#include "PkSearchFile.h"

#include <KDebug>

using namespace PackageKit;

PkInterface::PkInterface(QObject *parent) :
    AbstractIsRunning(parent)
{
    kDebug() << "Creating Helper";
    (void) new ModifyAdaptor(this);
    (void) new QueryAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService("org.freedesktop.PackageKit")) {
        kDebug() << "unable to register service to dbus";
        return;
    }

    if (!QDBusConnection::sessionBus().registerObject("/org/freedesktop/PackageKit", this)) {
        kDebug() << "unable to register object to dbus";
        return;
    }
}

PkInterface::~PkInterface()
{
}

void PkInterface::InstallCatalogs(uint xid, const QStringList &files, const QString &interaction)
{
    increaseRunning();
    kDebug() << xid << files << interaction;
    setDelayedReply(true);
    PkInstallCatalogs *task;
    task = new PkInstallCatalogs(xid, files, interaction, message());
    connect(task, SIGNAL(finished()), this, SLOT(decreaseRunning()));
    task->run();
}

void PkInterface::InstallFontconfigResources(uint xid, const QStringList &resources, const QString &interaction)
{
    increaseRunning();
    kDebug() << xid << resources << interaction;
    setDelayedReply(true);
    PkInstallFontconfigResources *task;
    task = new PkInstallFontconfigResources(xid, resources, interaction, message());
    connect(task, SIGNAL(finished()), this, SLOT(decreaseRunning()));
    task->run();
}

void PkInterface::InstallGStreamerResources(uint xid, const QStringList &resources, const QString &interaction)
{
    increaseRunning();
    kDebug() << xid << resources << interaction;
    setDelayedReply(true);
    PkInstallGStreamerResources *task;
    task = new PkInstallGStreamerResources(xid, resources, interaction, message());
    connect(task, SIGNAL(finished()), this, SLOT(decreaseRunning()));
    task->show();
}

void PkInterface::InstallMimeTypes(uint xid, const QStringList &mime_types, const QString &interaction)
{
    increaseRunning();
    kDebug() << xid << mime_types << interaction;
    setDelayedReply(true);
    PkInstallMimeTypes *task = new PkInstallMimeTypes(xid, mime_types, interaction, message());
    connect(task, SIGNAL(finished()), this, SLOT(decreaseRunning()));
    task->run();
}

void PkInterface::InstallPackageFiles(uint xid, const QStringList &files, const QString &interaction)
{
    increaseRunning();
    kDebug() << xid << files << interaction;
    setDelayedReply(true);
    PkInstallPackageFiles *task = new PkInstallPackageFiles(xid, files, interaction, message());
    connect(task, SIGNAL(finished()), this, SLOT(decreaseRunning()));
    task->show();
}

void PkInterface::InstallPackageNames(uint xid, const QStringList &packages, const QString &interaction)
{
    increaseRunning();
    kDebug() << xid << packages << interaction;
    setDelayedReply(true);
    PkInstallPackageNames *task = new PkInstallPackageNames(xid, packages, interaction, message());
    connect(task, SIGNAL(finished()), this, SLOT(decreaseRunning()));
    task->show();
}

void PkInterface::InstallProvideFiles(uint xid, const QStringList &files, const QString &interaction)
{
    increaseRunning();
    kDebug() << xid << files << interaction;
    setDelayedReply(true);
    PkInstallProvideFiles *task = new PkInstallProvideFiles(xid, files, interaction, message());
    connect(task, SIGNAL(finished()), this, SLOT(decreaseRunning()));
    task->run();
}

void PkInterface::RemovePackageByFiles(uint xid, const QStringList &files, const QString &interaction)
{
    increaseRunning();
    kDebug() << xid << files << interaction;
    setDelayedReply(true);
    PkRemovePackageByFiles *task;
    task = new PkRemovePackageByFiles(xid, files, interaction, message());
    connect(task, SIGNAL(finished()), this, SLOT(decreaseRunning()));
    task->run();
}

void PkInterface::InstallPrinterDrivers(uint xid, const QStringList &resources, const QString &interaction)
{
    increaseRunning();
    kDebug() << xid << resources << interaction;
    setDelayedReply(true);
    PkInstallPrinterDrivers *task;
    task = new PkInstallPrinterDrivers(xid, resources, interaction, message());
    connect(task, SIGNAL(finished()), this, SLOT(decreaseRunning()));
    task->run();
}

void PkInterface::InstallPlasmaResources(uint xid, const QStringList &resources, const QString &interaction)
{
    increaseRunning();
    kDebug() << xid << resources << interaction;
    setDelayedReply(true);
    PkInstallPlasmaResources *task;
    task = new PkInstallPlasmaResources(xid, resources, interaction, message());
    connect(task, SIGNAL(finished()), this, SLOT(decreaseRunning()));
    task->run();
}

void PkInterface::InstallResources(uint xid, const QString &type, const QStringList &resources, const QString &interaction)
{
    if (type == "codec")
        InstallGStreamerResources(xid, resources, interaction);
    else if (type == "mimetype")
        InstallMimeTypes(xid, resources, interaction);
    else if (type == "font")
        InstallFontconfigResources(xid, resources, interaction);
    else if (type == "postscript-driver")
        InstallPrinterDrivers(xid, resources, interaction);
    else if (type == "plasma-service")
        InstallPlasmaResources(xid, resources, interaction);
    else
        sendErrorReply("org.freedesktop.PackageKit.Failed", "Unsupported resource type");
}

//Query
bool PkInterface::IsInstalled(const QString &package_name, const QString &interaction)
{
    increaseRunning();
    setDelayedReply(true);
    PkIsInstalled *task = new PkIsInstalled(package_name, interaction, message());
    connect(task, SIGNAL(finished()), this, SLOT(decreaseRunning()));
    task->run();
    // This is discarted
    return false;
}

bool PkInterface::SearchFile(const QString &file_name, const QString &interaction, QString &package_name)
{
    Q_UNUSED(package_name)
    increaseRunning();
    setDelayedReply(true);
    PkSearchFile *task = new PkSearchFile(file_name, interaction, message());
    connect(task, SIGNAL(finished()), this, SLOT(decreaseRunning()));
    task->run();
    // This is discarted
    return false;
}
