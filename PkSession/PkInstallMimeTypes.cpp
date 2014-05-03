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

#include "PkInstallMimeTypes.h"
#include "IntroDialog.h"
#include "FilesModel.h"

#include <Daemon>

#include <PkStrings.h>

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

    IntroDialog *introDialog = new IntroDialog(this);
    m_model = new FilesModel(QStringList(), mime_types, this);
    introDialog->setModel(m_model);
    connect(introDialog, SIGNAL(continueChanged(bool)),
            this, SLOT(enableButtonOk(bool)));
    setMainWidget(introDialog);

    QString description;
    if (m_model->rowCount()) {
        description = i18np("Do you want to search for a program that can open this file type?",
                            "Do you want to search for a program that can open these file types?",
                            m_model->rowCount());
        enableButtonOk(true);
    } else {
        description = i18n("No valid file types were provided");
    }
    introDialog->setDescription(description);

    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        title = i18np("A program is requiring support to open this kind of files",
                      "A program is requiring support to open these kinds of files",
                      m_model->rowCount());
    } else {
        title = i18np("The application %2 is requiring support to open this kind of files",
                      "The application %2 is requiring support to open these kinds of files",
                      m_model->rowCount(),
                      parentTitle);
    }
    setTitle(title);
}

PkInstallMimeTypes::~PkInstallMimeTypes()
{
}

void PkInstallMimeTypes::search()
{
    QStringList mimeTypes = m_model->files();
    PkTransaction *transaction = new PkTransaction(this);
    Transaction *t;
    t = Daemon::whatProvides(mimeTypes,
                             Transaction::FilterNotInstalled | Transaction::FilterArch | Transaction::FilterNewest);
    transaction->setupTransaction(t);
    setTransaction(Transaction::RoleWhatProvides, transaction);
    connect(transaction, SIGNAL(finished(PkTransaction::ExitStatus)),
            this, SLOT(searchFinished(PkTransaction::ExitStatus)), Qt::UniqueConnection);
    connect(transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
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
