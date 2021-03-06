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

#include <QQuickItem>
#include <QQmlContext>

#include <PackageModel.h>
#include <PkTransaction.h>
#include <PkTransactionProgressModel.h>
#include <ApplicationSortFilterModel.h>
#include <PkStrings.h>
#include <PkIcons.h>
#include <KLocalizedString>

#include <Daemon>
#include <QQmlEngine>

void QmlPlugins::registerTypes(const char* uri)
{
//    Q_ASSERT(uri == QString(QLatin1String("org.kde.apper")));

    qmlRegisterType<DaemonHelper>(uri, 0, 1, "DaemonHelper");
    qmlRegisterType<DBusUpdaterInterface>(uri, 0, 1, "DBusUpdaterInterface");
    qmlRegisterType<PackageModel>(uri, 0, 1, "PackageModel");
    qmlRegisterType<PkTransaction>(uri, 0, 1, "PkTransaction");
    qmlRegisterType<PkTransactionProgressModel>(uri, 0, 1, "PkTransactionProgressModel");
    qmlRegisterType<ApplicationSortFilterModel>(uri, 0, 1, "ApplicationSortFilterModel");
    qmlRegisterType<PackageKit::Transaction>(uri, 0, 1, "Transaction");
    qmlRegisterUncreatableType<PackageKit::Daemon>(uri, 0, 1, "Daemon", QLatin1String("Global"));
    qRegisterMetaType<PkTransaction::ExitStatus>("PkTransaction::ExitStatus");
    qRegisterMetaType<PackageKit::Daemon::Network>("PackageKit::Daemon::Network");
    qRegisterMetaType<PackageKit::Daemon::Authorize>("PackageKit::Daemon::Authorize");
    qRegisterMetaType<PackageKit::Transaction::InternalError>("PackageKit::Transaction::InternalError");
    qRegisterMetaType<PackageKit::Transaction::Role>("PackageKit::Transaction::Role");
    qRegisterMetaType<PackageKit::Transaction::Error>("PackageKit::Transaction::Error");
    qRegisterMetaType<PackageKit::Transaction::Exit>("PackageKit::Transaction::Exit");
    qRegisterMetaType<PackageKit::Transaction::Filter>("PackageKit::Transaction::Filter");
//    qRegisterMetaType<PackageKit::Transaction::Message>("PackageKit::Transaction::Message");
    qRegisterMetaType<PackageKit::Transaction::Status>("PackageKit::Transaction::Status");
    qRegisterMetaType<PackageKit::Transaction::MediaType>("PackageKit::Transaction::MediaType");
    qRegisterMetaType<PackageKit::Transaction::DistroUpgrade>("PackageKit::Transaction::DistroUpgrade");
    qRegisterMetaType<PackageKit::Transaction::TransactionFlag>("PackageKit::Transaction::TransactionFlag");
    qRegisterMetaType<PackageKit::Transaction::TransactionFlags>("PackageKit::Transaction::TransactionFlags");
    qRegisterMetaType<PackageKit::Transaction::Restart>("PackageKit::Transaction::Restart");
    qRegisterMetaType<PackageKit::Transaction::UpdateState>("PackageKit::Transaction::UpdateState");
    qRegisterMetaType<PackageKit::Transaction::Group>("PackageKit::Transaction::Group");
    qRegisterMetaType<PackageKit::Transaction::Info>("PackageKit::Transaction::Info");
    qRegisterMetaType<PackageKit::Transaction::SigType>("PackageKit::Transaction::SigType");

}

void QmlPlugins::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(uri)
//    Q_ASSERT(uri == QLatin1String("org.kde.apper"));

    KLocalizedString::setApplicationDomain("apper");

    engine->rootContext()->setContextProperty(QLatin1String("Daemon"), Daemon::global());
    engine->rootContext()->setContextProperty(QLatin1String("PkStrings"), new PkStrings);
    engine->rootContext()->setContextProperty(QLatin1String("PkIcons"), new PkIcons);
    engine->rootContext()->setContextProperty(QLatin1String("DaemonHelper"), new DaemonHelper);
}
