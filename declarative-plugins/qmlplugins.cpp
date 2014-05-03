/***************************************************************************
 *   Copyright (C) 2012-2013 by Daniel Nicoletti <dantti12@gmail.com>      *
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

#include "qmlplugins.h"

#include "daemonhelper.h"
#include "DBusUpdaterInterface.h"

#include <QtDeclarative/QDeclarativeItem>
#include <QtDeclarative/QDeclarativeContext>

#include <PackageModel.h>
#include <PkTransaction.h>
#include <PkTransactionProgressModel.h>
#include <ApplicationSortFilterModel.h>
#include <PkStrings.h>
#include <PkIcons.h>

#include <Daemon>
#include <QDeclarativeEngine>

static const KCatalogLoader loader(QLatin1String("apper"));

void QmlPlugins::registerTypes(const char* uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.apper"));

    qmlRegisterType<DaemonHelper>(uri, 0, 1, "DaemonHelper");
    qmlRegisterType<DBusUpdaterInterface>(uri, 0, 1, "DBusUpdaterInterface");
    qmlRegisterType<PackageModel>(uri, 0, 1, "PackageModel");
    qmlRegisterType<PkTransaction>(uri, 0, 1, "PkTransaction");
    qmlRegisterType<PkTransactionProgressModel>(uri, 0, 1, "PkTransactionProgressModel");
    qmlRegisterType<ApplicationSortFilterModel>(uri, 0, 1, "ApplicationSortFilterModel");
    qmlRegisterType<PackageKit::Transaction>(uri, 0, 1, "Transaction");
    qmlRegisterUncreatableType<PackageKit::Daemon>(uri, 0, 1, "Daemon", "Global");
    qRegisterMetaType<PkTransaction::ExitStatus>("PkTransaction::ExitStatus");
}

void QmlPlugins::initializeEngine(QDeclarativeEngine *engine, const char *uri)
{
    Q_UNUSED(uri)
    Q_ASSERT(uri == QLatin1String("org.kde.apper"));

    engine->rootContext()->setContextProperty("Daemon", Daemon::global());
    engine->rootContext()->setContextProperty("PkStrings", new PkStrings);
    engine->rootContext()->setContextProperty("PkIcons", new PkIcons);
    engine->rootContext()->setContextProperty("DaemonHelper", new DaemonHelper);
}

Q_EXPORT_PLUGIN2(apper, QmlPlugins)
