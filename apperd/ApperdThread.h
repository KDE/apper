/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti <dantti12@gmail.com>           *
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

#ifndef APPERDTHREAD_H
#define APPERDTHREAD_H

#include <QThread>

#include <QTimer>
#include <QDBusConnection>
#include <QDateTime>

class DBusInterface;
class DistroUpgrade;
class RefreshCacheTask;
class TransactionWatcher;
class UpdateIcon;
class ApperdThread : public QObject
{
    Q_OBJECT
public:
    explicit ApperdThread(QObject *parent = 0);
    ~ApperdThread();

private slots:
    void init();
    void poll();
    void configFileChanged();
    void setProxy();

    void transactionListChanged(const QStringList &tids);
    void updatesChanged();

private:
    QDateTime getTimeSinceRefreshCache() const;
    bool nameHasOwner(const QString &name, const QDBusConnection &connection) const;
    bool isSystemReady(bool ignoreBattery, bool ignoreMobile) const;

    bool m_actRefreshCacheChecked;
    bool m_canRefreshCache;
    QDateTime m_lastRefreshCache;
    uint m_refreshCacheInterval;
    QTimer *m_qtimer;
    QThread *m_thread;

    DBusInterface *m_interface;
    DistroUpgrade *m_distroUpgrade;
    RefreshCacheTask *m_refreshCache;
    TransactionWatcher *m_trayIcon;
    UpdateIcon *m_updateIcon;
};

#endif // APPERDTHREAD_H
