/***************************************************************************
 *   Copyright (C) 2008-2011 Daniel Nicoletti <dantti12@gmail.com>         *
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

#ifndef DBUS_INTERFACE_H
#define DBUS_INTERFACE_H

#include <QtDBus/QDBusContext>
#include <QDBusObjectPath>

#include <config.h>

#ifdef HAVE_DEBCONFKDE
#include <DebconfGui.h>
using namespace DebconfKde;
#endif //HAVE_DEBCONFKDE

class DBusInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.apperd")
public:
    explicit DBusInterface(QObject *parent = 0);
    ~DBusInterface();

    void RefreshCache();
    void SetupDebconfDialog(const QString &tid, const QString &socketPath, uint xidParent);
    void WatchTransaction(const QDBusObjectPath &tid);

Q_SIGNALS:
    void refreshCache();
    void watchTransaction(const QDBusObjectPath &tid);

private Q_SLOTS:
    void debconfActivate();
    void transactionFinished();

#ifdef HAVE_DEBCONFKDE
private:
    QHash<QString, DebconfGui*> m_debconfGuis;
#endif
};


#endif
