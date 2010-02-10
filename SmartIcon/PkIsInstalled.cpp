/***************************************************************************
 *   Copyright (C) 2009-2010 by Daniel Nicoletti                           *
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

#include <KpkTransaction.h>
#include <KpkStrings.h>

#include <KMessageBox>
#include <KLocale>
#include <KDebug>

PkIsInstalled::PkIsInstalled(const QString &package_name,
                             const QString &interaction,
                             const QDBusMessage &message,
                             QWidget *parent)
 : KpkAbstractTask(0, interaction, message, parent),
   m_packageName(package_name),
   m_message(message)
{
}

PkIsInstalled::~PkIsInstalled()
{
}

void PkIsInstalled::start()
{
    Transaction *t = Client::instance()->resolve(m_packageName,
                                                 Enum::FilterInstalled);
    if (t->error()) {
        if (showWarning()) {
            KMessageBox::sorry(0,
                               KpkStrings::daemonError(t->error()),
                               i18n("Failed to start resolve transaction"));
        }
        sendErrorFinished(Failed, "Failed to start resolve transaction");
    } else {
        connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                this, SLOT(searchFinished(PackageKit::Enum::Exit, uint)));
        connect(t, SIGNAL(package(PackageKit::Package *)),
                this, SLOT(addPackage(PackageKit::Package *)));
        if (showProgress()) {
            KpkTransaction *trans = new KpkTransaction(t, KpkTransaction::CloseOnFinish);
            trans->show();
            setParentWindow(trans);
        }
    }
}

void PkIsInstalled::searchFinished(PackageKit::Enum::Exit status, uint)
{
    kDebug();
    if (status == Enum::ExitSuccess) {
        QDBusMessage reply = m_message.createReply();
        reply << (bool) m_foundPackages.size();
        sendMessageFinished(reply);
    } else if (status == Enum::ExitCancelled) {
        sendErrorFinished(Cancelled, i18n("User canceled the transaction"));
    } else {
        sendErrorFinished(InternalError, i18n("An unknown error happened"));
    }
}

void PkIsInstalled::addPackage(PackageKit::Package *package)
{
    m_foundPackages.append(package);
}

#include "PkIsInstalled.moc"
