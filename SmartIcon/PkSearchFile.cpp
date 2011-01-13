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

#include "PkSearchFile.h"

#include <KpkStrings.h>

#include <KLocale>
#include <KMessageBox>

#include <KDebug>

PkSearchFile::PkSearchFile(const QString &file_name,
                           const QString &interaction,
                           const QDBusMessage &message,
                           QWidget *parent)
 : KpkAbstractTask(0, interaction, message, parent),
   m_fileName(file_name),
   m_message(message)
{
}

PkSearchFile::~PkSearchFile()
{
}

void PkSearchFile::start()
{
    // Check for a leading slash '/' return with an error if it's not there..
    if (!m_fileName.startsWith('/')) {
        sendErrorFinished(Failed, "Only full file name path is supported");
        return;
    }

    Transaction *t = Client::instance()->searchFiles(m_fileName,
                                                     Enum::FilterNewest);
    if (t->error()) {
        if (showWarning()) {
            KMessageBox::sorryWId(parentWId(),
                                  KpkStrings::daemonError(t->error()),
                                  i18n("Failed to start search file transaction"));
        }
        sendErrorFinished(Failed, "Failed to start search file transaction");
    } else {
        // TODO add timeout support
        // which cancel the transaction in x seconds if it's not running yet
        connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                this, SLOT(searchFinished(PackageKit::Enum::Exit)));
        connect(t, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                this, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
        if (showProgress()) {
            kTransaction()->setTransaction(t);
            kTransaction()->show();
        }
    }
}

void PkSearchFile::searchFinished(PackageKit::Enum::Exit status)
{
    kDebug();
    if (status == Enum::ExitSuccess) {
        if (m_foundPackages.size()) {
            QSharedPointer<PackageKit::Package>pkg = m_foundPackages.first();
            bool installed = pkg->info() == Enum::InfoInstalled;
            QDBusMessage reply = m_message.createReply();
            reply << installed;
            reply << pkg->name();
            kDebug() << reply;
            sendMessageFinished(reply);
        } else {
            QString msg(i18n("The file name could not be found in any software source"));
            KMessageBox::sorryWId(parentWId(),
                                  msg,
                                  i18n("Could not find %1", m_fileName));
            sendErrorFinished(NoPackagesFound, msg);
        }
    } else {
        sendErrorFinished(InternalError, "An unknown error happened");
    }
}

void PkSearchFile::addPackage(QSharedPointer<PackageKit::Package>package)
{
    m_foundPackages.append(package);
}

#include "PkSearchFile.moc"
