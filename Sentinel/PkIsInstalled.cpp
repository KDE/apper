/***************************************************************************
 *   Copyright (C) 2009-2011 by Daniel Nicoletti                           *
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
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "PkIsInstalled.h"

#include <PkStrings.h>

#include <KLocale>
#include <KDebug>

PkIsInstalled::PkIsInstalled(const QString &package_name,
                             const QString &interaction,
                             const QDBusMessage &message,
                             QWidget *parent)
 : SessionTask(0, interaction, message, parent),
   m_packageName(package_name),
   m_message(message)
{
    setWindowTitle(i18n("Querying if a Package is Installed"));

    Transaction *t = new Transaction(this);
    PkTransaction *trans = setTransaction(Transaction::RoleResolve, t);
    connect(trans, SIGNAL(finished(PkTransaction::ExitStatus)),
            this, SLOT(searchFinished(PkTransaction::ExitStatus)), Qt::UniqueConnection);
    connect(t, SIGNAL(package(PackageKit::Package)),
            this, SLOT(addPackage(PackageKit::Package)));
    t->resolve(m_packageName, Transaction::FilterInstalled);
    if (t->error()) {
        QString msg = i18n("Failed to start resolve transaction");
        if (showWarning()) {
            setError(msg,
                     PkStrings::daemonError(t->error()));
        }
        sendErrorFinished(Failed, msg);
    }
}

PkIsInstalled::~PkIsInstalled()
{
}

void PkIsInstalled::searchFinished(PkTransaction::ExitStatus status)
{
    kDebug();
    if (status == PkTransaction::Success) {
        QDBusMessage reply = m_message.createReply();
        reply << (bool) foundPackagesSize();
        sendMessageFinished(reply);
    } else if (status == PkTransaction::Cancelled) {
        sendErrorFinished(Cancelled, i18n("User canceled the transaction"));
    } else {
        sendErrorFinished(InternalError, i18n("An unknown error happened"));
    }
}

#include "PkIsInstalled.moc"
