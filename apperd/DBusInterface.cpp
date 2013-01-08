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

#include <KDebug>

DBusInterface::DBusInterface(QObject *parent) :
    QObject(parent)
{
    kDebug() << "Creating Helper";
    (void) new ApperdAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService("org.kde.apperd")) {
        kDebug() << "another helper is already running";
        return;
    }

    if (!QDBusConnection::sessionBus().registerObject("/", this)) {
        kDebug() << "unable to register service interface to dbus";
        return;
    }
}

DBusInterface::~DBusInterface()
{
    kDebug() << "-------------DBusInterface-------------" << QThread::currentThreadId();
}

void DBusInterface::RefreshCache()
{
    emit refreshCache();
}

void DBusInterface::SetupDebconfDialog(const QString &tid, const QString &socketPath, uint xidParent)
{
#ifdef HAVE_DEBCONFKDE
    kDebug() << tid << socketPath << xidParent;
    DebconfGui *gui;
    if (m_debconfGuis.contains(socketPath)) {
        gui = m_debconfGuis[socketPath];
    } else {
        // Create the Transaction object to delete
        // the DebconfGui class when the transaction finishes
        Transaction *transaction = new Transaction(QDBusObjectPath(tid));
        if (transaction->error()) {
            transaction->deleteLater();
            return;
        }
        transaction->setProperty("socketPath", socketPath);
        connect(transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                this, SLOT(transactionFinished()));

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
    kDebug() << "Not compiled with Debconf support - ignoring";
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
    kDebug();
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
