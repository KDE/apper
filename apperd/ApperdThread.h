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

    void transactionListChanged(const QStringList &tids);
    void updatesChanged();
    void serviceOwnerChanged(const QString &serviceName, const QString &oldOwner, const QString &newOwner);

private:
    void callApperSentinel(const QString &method,
                           const QList<QVariant> &arguments = QList<QVariant>());
    QDateTime getTimeSinceRefreshCache() const;
    QString networkState() const;
    bool nameHasOwner(const QString &name, const QDBusConnection &connection) const;
    bool isSystemReady(bool ignoreBattery, bool ignoreMobile) const;

    bool m_actRefreshCacheChecked;
    bool m_canRefreshCache;
    bool m_sentinelIsRunning;
    QDateTime m_lastRefreshCache;
    uint m_refreshCacheInterval;
    QTimer *m_qtimer;
    QThread *m_thread;
};

#endif // APPERDTHREAD_H
