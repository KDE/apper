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
    KConfig config("apper");
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    bool ignoreBattery;
    bool ignoreMobile;
    ignoreBattery = checkUpdateGroup.readEntry("checkUpdatesOnBattery", false);
    ignoreMobile = checkUpdateGroup.readEntry("checkUpdatesOnMobile", false);
    if (!systemIsReady(ignoreBattery, ignoreMobile)) {
        kDebug() << "Not checking for updates, as we might be on battery or mobile connection";
        return;
    }

    kDebug() << "isRunning" << isRunning();
    if (!isRunning()) {
        SET_PROXY
        increaseRunning();
        Transaction *t = new Transaction(this);
        // ignore if there is an error
        // Be silent! don't bother the user if the cache couldn't be refreshed
        // TODO bother the user again... he needs to know the thruth
        connect(t, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                this, SLOT(decreaseRunning()));
        // Refresh Cache is false otherwise it will rebuild
        // the whole cache on Fedora
        t->refreshCache(false);
        if (t->error()) {
            kDebug() << "refresh error" << t->error();
            KNotification *notify = new KNotification("TransactionError", 0);
            notify->setText(PkStrings::daemonError(t->error()));
            notify->setPixmap(KIcon("dialog-error").pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
            notify->sendEvent();
            decreaseRunning();
        }
    }
}

void RefreshCacheTask::autoUpdatesFinished(PackageKit::Transaction::Exit status)
{
    if (status == Transaction::ExitSuccess || status == Transaction::ExitCancelled) {
        // decrease first only because we want to check for updates again
        decreaseRunning();
    } else {
        // Not decreasing and being Persistent
        // prevents multiple popups issued by
        // subsequent refresh cache tries
        m_notification = new KNotification("TransactionFailed", KNotification::Persistent, this);
        connect(m_notification, SIGNAL(closed()), this, SLOT(notificationClosed()));
        KIcon icon("dialog-cancel");
        // use of QSize does the right thing
        m_notification->setPixmap(icon.pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
        m_notification->setText(i18n("The automated refresh caches failed."));
        m_notification->sendEvent();
    }
}

void RefreshCacheTask::notificationClosed()
{
    m_notification->deleteLater();
    m_notification = 0;
    decreaseRunning();
}
