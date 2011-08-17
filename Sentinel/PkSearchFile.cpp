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

    Transaction *t = new Transaction(this);
    PkTransaction *trans = setTransaction(Transaction::RoleSearchFile, t);
    connect(trans, SIGNAL(finished(PkTransaction::ExitStatus)),
            this, SLOT(searchFinished(PkTransaction::ExitStatus)), Qt::UniqueConnection);
    connect(t, SIGNAL(package(PackageKit::Package)),
            this, SLOT(addPackage(PackageKit::Package)));
    t->searchFiles(m_fileName, Transaction::FilterNewest);
    Transaction::InternalError error = t->error();
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
    const Package &pkg = foundPackagesList().first();
    bool installed = pkg.info() == Package::InfoInstalled;
    QDBusMessage reply = m_message.createReply();
    reply << installed;
    reply << pkg.name();
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
