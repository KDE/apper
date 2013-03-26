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

#include "PkSearchFile.h"

#include <PkStrings.h>
#include <PackageModel.h>

#include <KLocale>

#include <KDebug>

PkSearchFile::PkSearchFile(const QString &file_name,
                           const QString &interaction,
                           const QDBusMessage &message,
                           QWidget *parent)
 : SessionTask(0, interaction, message, parent),
   m_fileName(file_name),
   m_message(message)
{
    setWindowTitle(i18n("Search Packages that Provides Files"));

    // Check for a leading slash '/' return with an error if it's not there..
    if (!m_fileName.startsWith('/')) {
        sendErrorFinished(Failed, "Only full file name path is supported");
        return;
    }

    PkTransaction *transaction = new PkTransaction(this);
    setTransaction(Transaction::RoleSearchFile, transaction);
    connect(transaction, SIGNAL(finished(PkTransaction::ExitStatus)),
            this, SLOT(searchFinished(PkTransaction::ExitStatus)), Qt::UniqueConnection);
    connect(transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
    transaction->searchFiles(m_fileName, Transaction::FilterNewest);
    Transaction::InternalError error = transaction->internalError();
    if (error) {
        QString msg = i18n("Failed to start search file transaction");
        if (showWarning()) {
            setError(msg,
                     PkStrings::daemonError(error));
        }
        sendErrorFinished(Failed, msg);
    }
}

PkSearchFile::~PkSearchFile()
{
}

void PkSearchFile::searchSuccess()
{
    QModelIndex index = model()->index(0, 0);
    bool installed = false;
    QString packageID;
    if (index.isValid()) {
        Transaction::Info info;
        info = index.data(PackageModel::InfoRole).value<Transaction::Info>();
        installed = info == Transaction::InfoInstalled;
        packageID = index.data(PackageModel::IdRole).toString();
    }
    QDBusMessage reply = m_message.createReply();
    reply << installed;
    reply << Transaction::packageName(packageID);
    kDebug() << reply;
    sendMessageFinished(reply);
}

void PkSearchFile::notFound()
{
    QString msg(i18n("The file name could not be found in any software source"));
    setError(i18n("Could not find %1", m_fileName), msg);
    sendErrorFinished(NoPackagesFound, msg);
}

#include "PkSearchFile.moc"
