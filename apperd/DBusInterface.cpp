/***************************************************************************
 *   Copyright (C) 2008-2011 Daniel Nicoletti                              *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "DBusInterface.h"

#include "apperdadaptor.h"

#include <QtDBus/QDBusConnection>

#ifdef HAVE_DEBCONFKDE
#include <KDialog>
#include <KWindowSystem>
#include <Transaction>
using namespace PackageKit;
#endif

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(APPER_DAEMON)

DBusInterface::DBusInterface(QObject *parent) :
    QObject(parent)
{
    qCDebug(APPER_DAEMON) << "Creating Helper";
    (void) new ApperdAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.apperd"))) {
        qCDebug(APPER_DAEMON) << "another helper is already running";
        return;
    }

    if (!QDBusConnection::sessionBus().registerObject(QStringLiteral("/"), this)) {
        qCDebug(APPER_DAEMON) << "unable to register service interface to dbus";
        return;
    }
}

DBusInterface::~DBusInterface()
{
    qCDebug(APPER_DAEMON) << "-------------DBusInterface-------------" << QThread::currentThreadId();
}

void DBusInterface::RefreshCache()
{
    emit refreshCache();
}

void DBusInterface::SetupDebconfDialog(const QString &tid, const QString &socketPath, uint xidParent)
{
#ifdef HAVE_DEBCONFKDE
    qCDebug(APPER_DAEMON) << tid << socketPath << xidParent;
    DebconfGui *gui;
    if (m_debconfGuis.contains(socketPath)) {
        gui = m_debconfGuis[socketPath];
    } else {
        // Create the Transaction object to delete
        // the DebconfGui class when the transaction finishes
        auto transaction = new Transaction(QDBusObjectPath(tid));
        transaction->setProperty("socketPath", socketPath);
        connect(transaction, &Transaction::finished, this, &DBusInterface::transactionFinished);

        // Setup the Debconf dialog
        gui = new DebconfGui(socketPath);
        gui->setWindowModality(Qt::WindowModal);
        gui->setWindowFlags(Qt::Dialog);
        m_debconfGuis[socketPath] = gui;
        connect(gui, SIGNAL(activated()), this, SLOT(debconfActivate()));
        connect(gui, SIGNAL(deactivated()), gui, SLOT(hide()));
    }
    gui->setProperty("xidParent", xidParent);
#else
    Q_UNUSED(tid)
    Q_UNUSED(socketPath)
    Q_UNUSED(xidParent)
    qCDebug(APPER_DAEMON) << "Not compiled with Debconf support - ignoring";
#endif //HAVE_DEBCONFKDE
}

void DBusInterface::WatchTransaction(const QDBusObjectPath &tid)
{
    emit watchTransaction(tid);
}

void DBusInterface::debconfActivate()
{
#ifdef HAVE_DEBCONFKDE
    // Correct the parent
    qCDebug(APPER_DAEMON);
    DebconfGui *gui = qobject_cast<DebconfGui*>(sender());
    uint xidParent  = gui->property("xidParent").toUInt();
    KWindowSystem::setMainWindow(gui, xidParent);
    gui->show();
#endif
}

void DBusInterface::transactionFinished()
{
#ifdef HAVE_DEBCONFKDE
    QString socketPath = sender()->property("socketPath").toString();
    if (m_debconfGuis.contains(socketPath)) {
        // remove the gui from the list and also delete it
        m_debconfGuis.take(socketPath)->deleteLater();
    }
#endif // HAVE_DEBCONFKDE
}

#include "moc_DBusInterface.cpp"
