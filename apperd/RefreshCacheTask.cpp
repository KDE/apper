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

#include <KIcon>
#include <KLocale>

#include <KDebug>

RefreshCacheTask::RefreshCacheTask(QObject *parent) :
    QObject(parent),
    m_transaction(0),
    m_notification(0),
    m_lastError(Transaction::ErrorUnknown)
{
}

void RefreshCacheTask::refreshCache()
{
    kDebug();
    if (!m_transaction) {
        m_transaction = new Transaction(this);
        connect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                this, SLOT(refreshCacheFinished(PackageKit::Transaction::Exit,uint)));
        connect(m_transaction, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
                this, SLOT(errorCode(PackageKit::Transaction::Error,QString)));

        // Refresh Cache is false otherwise it will rebuild
        // the whole cache on Fedora
        m_transaction->refreshCache(false);
        if (m_transaction->internalError() && !m_notification) {
            m_notification = new KNotification("TransactionFailed", KNotification::Persistent, this);
            m_notification->setComponentData(KComponentData("apperd"));
            connect(m_notification, SIGNAL(closed()), this, SLOT(notificationClosed()));
            KIcon icon("dialog-cancel");
            // use of QSize does the right thing
            m_notification->setPixmap(icon.pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
            m_notification->setText(PkStrings::daemonError(m_transaction->internalError()));
            m_notification->sendEvent();
        } else {
            m_transaction = 0;
        }
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

    m_notification = new KNotification("TransactionFailed", KNotification::Persistent, this);
    m_notification->setComponentData(KComponentData("apperd"));
    connect(m_notification, SIGNAL(closed()), this, SLOT(notificationClosed()));
    KIcon icon("dialog-cancel");
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
