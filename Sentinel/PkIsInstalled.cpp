/***************************************************************************
 *   Copyright (C) 2009-2011 by Daniel Nicoletti                           *
 *   dantti85-pk@yahoo.com.br                                              *
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

#include <KpkStrings.h>

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
    Transaction *t = new Transaction(this);
    connect(t, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
            this, SLOT(searchFinished(PackageKit::Transaction::Exit)));
    connect(t, SIGNAL(package(PackageKit::Package)),
            this, SLOT(addPackage(PackageKit::Package)));
    PkTransaction *trans = new PkTransaction(t, this);
    setMainWidget(trans);
    t->resolve(m_packageName, Transaction::FilterInstalled);
    if (t->error()) {
        QString msg = i18n("Failed to start resolve transaction");
        if (showWarning()) {
            setError(msg,
                     KpkStrings::daemonError(t->error()));
        }
        sendErrorFinished(Failed, msg);
    }
}

PkIsInstalled::~PkIsInstalled()
{
}

void PkIsInstalled::searchFinished(PackageKit::Transaction::Exit status)
{
    kDebug();
    if (status == Transaction::ExitSuccess) {
        QDBusMessage reply = m_message.createReply();
        reply << (bool) m_foundPackages.size();
        sendMessageFinished(reply);
    } else if (status == Transaction::ExitCancelled) {
        sendErrorFinished(Cancelled, i18n("User canceled the transaction"));
    } else {
        sendErrorFinished(InternalError, i18n("An unknown error happened"));
    }
}

void PkIsInstalled::addPackage(const PackageKit::Package &package)
{
    m_foundPackages.append(package);
}

#include "PkIsInstalled.moc"
