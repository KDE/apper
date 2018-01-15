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

#include "PkInterface.h"
#include "packagekitadaptor.h"

#include <QtDBus/QDBusConnection>
//#include <KWindowSystem>

#include "SessionTask.h"
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

#include <Daemon>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(APPER_SESSION)

using namespace PackageKit;

PkInterface::PkInterface(QObject *parent) :
    AbstractIsRunning(parent)
{
    if (!Daemon::isRunning()) {
        QTimer timer;
        timer.setInterval(5000);
        QEventLoop loop;
        connect(Daemon::global(), &Daemon::isRunningChanged, &loop, &QEventLoop::quit);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        loop.exec();
        if (!Daemon::isRunning()) {
            qCWarning(APPER_SESSION) << "Packagekit didn't start";
            qApp->quit();
            return;
        }
    }

    qCDebug(APPER_SESSION) << "Creating Helper";
    (void) new ModifyAdaptor(this);
    (void) new QueryAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService("org.freedesktop.PackageKit")) {
        qCDebug(APPER_SESSION) << "unable to register service to dbus";
        return;
    }

    if (!QDBusConnection::sessionBus().registerObject("/org/freedesktop/PackageKit", this)) {
        qCDebug(APPER_SESSION) << "unable to register object to dbus";
        return;
    }
}

PkInterface::~PkInterface()
{
}

void PkInterface::InstallCatalogs(uint xid, const QStringList &files, const QString &interaction)
{
    qCDebug(APPER_SESSION) << xid << files << interaction;
    auto task = new PkInstallCatalogs(xid, files, interaction, message());
    show(task);
}

void PkInterface::InstallFontconfigResources(uint xid, const QStringList &resources, const QString &interaction)
{
    qCDebug(APPER_SESSION) << xid << resources << interaction;
    auto task = new PkInstallFontconfigResources(xid, resources, interaction, message());
    show(task);
}

void PkInterface::InstallGStreamerResources(uint xid, const QStringList &resources, const QString &interaction)
{
    qCDebug(APPER_SESSION) << xid << resources << interaction;
    auto task = new PkInstallGStreamerResources(xid, resources, interaction, message());
    show(task);
}

void PkInterface::InstallMimeTypes(uint xid, const QStringList &mime_types, const QString &interaction)
{
    qCDebug(APPER_SESSION) << xid << mime_types << interaction;
    auto task = new PkInstallMimeTypes(xid, mime_types, interaction, message());
    show(task);
}

void PkInterface::InstallPackageFiles(uint xid, const QStringList &files, const QString &interaction)
{
    qCDebug(APPER_SESSION) << xid << files << interaction;
    auto task = new PkInstallPackageFiles(xid, files, interaction, message());
    show(task);
}

void PkInterface::InstallPackageNames(uint xid, const QStringList &packages, const QString &interaction)
{
    qCDebug(APPER_SESSION) << xid << packages << interaction;
    auto task = new PkInstallPackageNames(xid, packages, interaction, message());
    show(task);
}

void PkInterface::InstallProvideFiles(uint xid, const QStringList &files, const QString &interaction)
{
    qCDebug(APPER_SESSION) << xid << files << interaction;
    auto task = new PkInstallProvideFiles(xid, files, interaction, message());
    show(task);
}

void PkInterface::RemovePackageByFiles(uint xid, const QStringList &files, const QString &interaction)
{
    qCDebug(APPER_SESSION) << xid << files << interaction;
    auto task = new PkRemovePackageByFiles(xid, files, interaction, message());
    show(task);
}

void PkInterface::InstallPrinterDrivers(uint xid, const QStringList &resources, const QString &interaction)
{
    qCDebug(APPER_SESSION) << xid << resources << interaction;
    auto task = new PkInstallPrinterDrivers(xid, resources, interaction, message());
    show(task);
}

void PkInterface::InstallPlasmaResources(uint xid, const QStringList &resources, const QString &interaction)
{
    qCDebug(APPER_SESSION) << xid << resources << interaction;
    auto task = new PkInstallPlasmaResources(xid, resources, interaction, message());
    show(task);
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
    auto task = new PkIsInstalled(package_name, interaction, message());
    show(task);
    // This is discarted
    return false;
}

bool PkInterface::SearchFile(const QString &file_name, const QString &interaction, QString &package_name)
{
    Q_UNUSED(package_name)
    auto task = new PkSearchFile(file_name, interaction, message());
    show(task);
    // This is discarted
    return false;
}

void PkInterface::show(SessionTask *widget)
{
    increaseRunning();
    setDelayedReply(true);

    connect(widget, &SessionTask::finished, this, &PkInterface::decreaseRunning);
    if (widget->parentWId()) {
        // Check before showing if the widget has
        // a parent, otherwise it should not be modal
        // to not lock the application
        widget->setWindowModality(Qt::WindowModal);
    }
    widget->show();
//    KWindowSystem::forceActiveWindow(widget->winId());
//    KWindowSystem::setMainWindow(widget, widget->parentWId());
}
