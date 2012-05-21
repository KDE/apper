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

#include <Macros.h>
#include <PkStrings.h>
#include <PkIcons.h>

#include <Daemon>

#include <KIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KLocale>

#include <KDebug>

using namespace PackageKit;

RefreshCacheTask::RefreshCacheTask(QObject *parent) :
    AbstractIsRunning(parent)
{
}

void RefreshCacheTask::refreshCache()
{
    kDebug() << "isRunning" << isRunning();
    if (!isRunning()) {
        SET_PROXY
        increaseRunning();
        Transaction *t = new Transaction(this);
        connect(t, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                this, SLOT(refreshCacheFinished(PackageKit::Transaction::Exit)));
        connect(t, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
                this, SLOT(errorCode(PackageKit::Transaction::Error,QString)));

        // Refresh Cache is false otherwise it will rebuild
        // the whole cache on Fedora
        t->refreshCache(false);
        if (t->error()) {
            m_notification = new KNotification("TransactionFailed", KNotification::Persistent, this);
            connect(m_notification, SIGNAL(closed()), this, SLOT(notificationClosed()));
            KIcon icon("dialog-cancel");
            // use of QSize does the right thing
            m_notification->setPixmap(icon.pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
            m_notification->setText(PkStrings::daemonError(t->error()));
            m_notification->sendEvent();
        }
    }
}

void RefreshCacheTask::refreshCacheFinished(PackageKit::Transaction::Exit status)
{
    if (status == Transaction::ExitSuccess || status == Transaction::ExitCancelled) {
        // decrease first only because we want to check for updates again
        decreaseRunning();
    }
}

void RefreshCacheTask::errorCode(Transaction::Error error, const QString &errorMessage)
{
    // Not decreasing and being Persistent
    // prevents multiple popups issued by
    // subsequent refresh cache tries
    m_notification = new KNotification("TransactionFailed", KNotification::Persistent, this);
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
    decreaseRunning();
}
