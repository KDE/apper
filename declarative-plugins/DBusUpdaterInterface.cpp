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

#include "DBusUpdaterInterface.h"

#include "apperupdatericonadaptor.h"

#include <QtDBus/QDBusConnection>

#include <KDebug>

DBusUpdaterInterface::DBusUpdaterInterface(QObject *parent) :
    QObject(parent),
    m_registered(false)
{
    (void) new ApperUpdaterIconAdaptor(this);
}

DBusUpdaterInterface::~DBusUpdaterInterface()
{
    if (m_registered) {
        unregisterService();
    }
}

bool DBusUpdaterInterface::isRegistered() const
{
    return m_registered;
}

void DBusUpdaterInterface::ReviewUpdates()
{
    emit reviewUpdates();
}

void DBusUpdaterInterface::registerService()
{
    kDebug();
    QDBusServiceWatcher *watcher = qobject_cast<QDBusServiceWatcher*>(sender());
    if (!m_registered && !QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.ApperUpdaterIcon"))) {
        kDebug() << "unable to register service to dbus";
        if (!watcher) {
            // in case registration fails due to another user or application running
            // keep an eye on it so we can register when available
            watcher = new QDBusServiceWatcher(QLatin1String("org.kde.ApperUpdaterIcon"),
                                              QDBusConnection::systemBus(),
                                              QDBusServiceWatcher::WatchForUnregistration,
                                              this);
            connect(watcher, SIGNAL(serviceUnregistered(QString)), this, SLOT(registerService()));
        }
        m_registered = false;
        emit registeredChanged();
    } else {
        if (!QDBusConnection::sessionBus().registerObject("/", this)) {
            kDebug() << "unable to register service interface to dbus";
            return;
        }

        m_registered = true;
        emit registeredChanged();
    }
}

void DBusUpdaterInterface::unregisterService()
{
    // We need to unregister the service since
    // plasma-desktop won't exit
    if (QDBusConnection::sessionBus().unregisterService(QLatin1String("org.kde.ApperUpdaterIcon"))) {
        m_registered = false;
        emit registeredChanged();
    } else {
        kDebug() << "unable to unregister service to dbus";
    }
}
