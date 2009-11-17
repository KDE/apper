/***************************************************************************
 *   Copyright (C) 2008 Daniel Nicoletti <dantti85-pk@yahoo.com.br>        *
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

#include "KpkInterface.h"
#include "kpackagekitsmarticonadaptor.h"

#include <QtDBus/QDBusConnection>
#include <KpkStrings.h>
#include <KpkIcons.h>
#include <KpkMacros.h>
#include <KIcon>
#include <KNotification>
#include <QPackageKit>

#include <KDebug>

using namespace PackageKit;

KpkInterface::KpkInterface(QObject *parent)
        : QObject(parent)
{
    qDebug() << "Creating Helper";
    (void) new KPackageKitSmartIconAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService("org.kde.KPackageKitSmartIcon")) {
        kDebug() << "another helper is already running";
        return;
    }

    if (!QDBusConnection::sessionBus().registerObject("/", this)) {
        kDebug() << "unable to register service interface to dbus";
        return;
    }
}

KpkInterface::~KpkInterface()
{
}

void KpkInterface::WatchTransaction(const QString &tid)
{
    emit watchTransaction(tid);
}

void KpkInterface::RefreshCache()
{
    SET_PROXY
    Transaction *t = Client::instance()->refreshCache(true);
    if (t->error()) {
        KNotification *notify = new KNotification("TransactionError", 0, KNotification::Persistent);
            notify->setText(KpkStrings::daemonError(t->error()));
            notify->setPixmap(KIcon("dialog-error").pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
            notify->sendEvent();
    } else {
        emit watchTransaction(t->tid());
    }
}

void KpkInterface::Update()
{
    emit refreshAndUpdate(false);
}

void KpkInterface::RefreshAndUpdate()
{
    emit refreshAndUpdate(true);
}
