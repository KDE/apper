/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti dantti12@gmail.com             *
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

#include "RefreshCacheTask.h"

#include <PkStrings.h>
#include <PkIcons.h>

#include <Daemon>

#include <KLocalizedString>

#include <QLoggingCategory>

RefreshCacheTask::RefreshCacheTask(QObject *parent) :
    QObject(parent),
    m_transaction(0),
    m_notification(0),
    m_lastError(Transaction::ErrorUnknown),
    m_cacheAge(3600)
{
}

void RefreshCacheTask::refreshCache()
{
//    kDebug();
    if (!m_transaction) {
        // Refresh Cache is false otherwise it will rebuild
        // the whole cache on Fedora
        Daemon::setHints (QLatin1String("cache-age=") + QString::number(m_cacheAge));
        m_transaction = Daemon::refreshCache(false);
        connect(m_transaction, &Transaction::finished, this, &RefreshCacheTask::refreshCacheFinished);
        connect(m_transaction, &Transaction::errorCode, this, &RefreshCacheTask::errorCode);
    }
}

void RefreshCacheTask::refreshCacheFinished(PackageKit::Transaction::Exit status, uint runtime)
{
    Q_UNUSED(runtime)

    m_transaction = 0;
    if (status == Transaction::ExitSuccess) {
        m_lastError = Transaction::ErrorUnknown;
        m_lastErrorString.clear();
    }
}

void RefreshCacheTask::errorCode(Transaction::Error error, const QString &errorMessage)
{
    if (m_notification || (m_lastError == error && m_lastErrorString == errorMessage)) {
        return;
    }

    m_notification = new KNotification(QLatin1String("TransactionFailed"), KNotification::Persistent, this);
    m_notification->setComponentName(QLatin1String("apperd"));
    connect(m_notification, &KNotification::closed, this, &RefreshCacheTask::notificationClosed);
    QIcon icon = QIcon::fromTheme(QLatin1String("dialog-cancel"));
    // use of QSize does the right thing
    m_notification->setPixmap(icon.pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
    m_notification->setTitle(PkStrings::error(error));
    m_notification->setText(errorMessage);
    m_notification->sendEvent();
}

void RefreshCacheTask::notificationClosed()
{
    m_notification->deleteLater();
    m_notification = 0;
}

#include "moc_RefreshCacheTask.cpp"
