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

#include "PkInstallMimeTypes.h"
#include "IntroDialog.h"
#include "FilesModel.h"

#include <KpkStrings.h>

#include <KLocale>

#include <KDebug>

PkInstallMimeTypes::PkInstallMimeTypes(uint xid,
                                      const QStringList &mime_types,
                                      const QString &interaction,
                                      const QDBusMessage &message,
                                      QWidget *parent)
 : SessionTask(xid, interaction, message, parent),
   m_mimeTypes(mime_types)
{
    setWindowTitle(i18n("Install Support for File Types"));

    m_introDialog = new IntroDialog(this);
    m_model = new FilesModel(QStringList(), mime_types, this);
    connect(m_model, SIGNAL(rowsInserted(QModelIndex, int, int)),
            this, SLOT(modelChanged()));
    m_introDialog->setModel(m_model);
    setMainWidget(m_introDialog);

    modelChanged();
}

PkInstallMimeTypes::~PkInstallMimeTypes()
{
}

void PkInstallMimeTypes::modelChanged()
{
    kDebug() << m_mimeTypes.first();
    QString message = i18n("Do you want to search for a program that can open this file type?",
                           m_mimeTypes.first());
    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        title = i18np("A program is requiring support to open this kind of files",
                      "A program is requiring support to open these kinds of files",
                      m_mimeTypes.size());
    } else {
        title = i18np("The application %2 is requiring support to open this kind of files",
                      "The application %2 is requiring support to open these kinds of files",
                      m_mimeTypes.size(),
                      parentTitle);
    }
    setTitle(title);
    m_introDialog->setDescription(message);
}

void PkInstallMimeTypes::search()
{
    Transaction *t = new Transaction(this);
    PkTransaction *trans = setTransaction(t);
    connect(trans, SIGNAL(finished(PkTransaction::ExitStatus)),
            this, SLOT(searchFinished(PkTransaction::ExitStatus)), Qt::UniqueConnection);
    connect(t, SIGNAL(package(PackageKit::Package)),
            this, SLOT(addPackage(PackageKit::Package)));
    t->whatProvides(Transaction::ProvidesMimetype,
                    m_mimeTypes,
                    Transaction::FilterNotInstalled | Transaction::FilterArch | Transaction::FilterNewest);
    if (t->error()) {
        if (showWarning()) {
            setError(i18n("Failed to search for provides"),
                     KpkStrings::daemonError(t->error()));
        }
        sendErrorFinished(Failed, "Failed to search for provides");
    }
}

void PkInstallMimeTypes::notFound()
{
    QString msg = i18n("Could not find software");
    if (showWarning()) {
        setInfo(msg, i18n("No new applications can be found "
                          "to handle this type of file"));
    }
    sendErrorFinished(NoPackagesFound, "nothing was found to handle mime type");
}

//setTitle(i18np("Application that can open this type of file",
//               "Applications that can open this type of file",
//               m_foundPackages.size()));

#include "PkInstallMimeTypes.moc"
