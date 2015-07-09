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

#include <QTimer>
#include <QDBusConnection>
#include <QDateTime>

class DBusInterface;
class DistroUpgrade;
class RefreshCacheTask;
class TransactionWatcher;
class Updater;
class AptRebootListener;

class ApperdThread : public QObject
{
    Q_OBJECT
public:
    explicit ApperdThread(QObject *parent = 0);
    ~ApperdThread();

    static bool nameHasOwner(const QString &name, const QDBusConnection &connection);

private Q_SLOTS:
    void init();
    void poll();
    void configFileChanged();
    void proxyChanged();
    void setProxy();

    void updatesChanged();
    void appShouldConserveResourcesChanged();

private:
    QDateTime getTimeSinceRefreshCache() const;
    bool isSystemReady(bool ignoreBattery, bool ignoreMobile) const;

    bool m_proxyChanged;
    QVariantHash m_configs;
    QHash<QString, QString> m_proxyConfig;
    QDateTime m_lastRefreshCache;
    QTimer *m_qtimer;

    DBusInterface *m_interface;
    DistroUpgrade *m_distroUpgrade;
    RefreshCacheTask *m_refreshCache;
    TransactionWatcher *m_transactionWatcher;
    Updater *m_updater;

    // Apt reboot listener
    AptRebootListener *m_AptRebootListener;
};

#endif // APPERDTHREAD_H
