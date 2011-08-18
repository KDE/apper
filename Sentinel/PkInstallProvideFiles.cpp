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

#include "PkInstallProvideFiles.h"
#include "IntroDialog.h"
#include "FilesModel.h"

#include <PkStrings.h>

#include <KLocale>

#include <KDebug>

PkInstallProvideFiles::PkInstallProvideFiles(uint xid,
                                             const QStringList &files,
                                             const QString &interaction,
                                             const QDBusMessage &message,
                                             QWidget *parent)
 : SessionTask(xid, interaction, message, parent),
   m_args(files)
{
    setWindowTitle(i18n("Install Packages that Provides Files"));

    IntroDialog *introDialog = new IntroDialog(this);
    FilesModel *model = new FilesModel(files, QStringList(), this);
    introDialog->setModel(model);
    connect(introDialog, SIGNAL(continueChanged(bool)),
            this, SLOT(enableButtonOk(bool)));
    setMainWidget(introDialog);

    if (m_args.isEmpty()) {
        introDialog->setDescription(i18n("No files were provided"));
    } else {
        QString description;
        description = i18np("Do you want to search for this now?",
                            "Do you want to search for these now?",
                            m_args.size());
        introDialog->setDescription(description);
        enableButtonOk(true);
    }

    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        title = i18np("A program wants to install a file",
                      "A program wants to install files",
                      m_args.size());
    } else {
        title = i18np("The application %2 is asking to install a file",
                      "The application %2 is asking to install files",
                      m_args.size(),
                      parentTitle);
    }
    setTitle(title);
}

PkInstallProvideFiles::~PkInstallProvideFiles()
{
}

void PkInstallProvideFiles::search()
{
    Transaction *t = new Transaction(this);
    PkTransaction *trans = setTransaction(Transaction::RoleSearchFile, t);
    connect(trans, SIGNAL(finished(PkTransaction::ExitStatus)),
            this, SLOT(searchFinished(PkTransaction::ExitStatus)), Qt::UniqueConnection);
    connect(t, SIGNAL(package(PackageKit::Package)),
            this, SLOT(addPackage(PackageKit::Package)));
    t->searchFiles(m_args.first(), Transaction::FilterArch | Transaction::FilterNewest);
    if (t->error()) {
        QString msg = i18n("Failed to start search file transaction");
        if (showWarning()) {
            setError(msg,
                     PkStrings::daemonError(t->error()));
        }
        sendErrorFinished(Failed, msg);
    }
}

void PkInstallProvideFiles::notFound()
{
    if (m_alreadyInstalled.size()) {
        if (showWarning()) {
            setInfo(i18n("Failed to install file"),
                    i18n("The %1 package already provides this file",
                         m_alreadyInstalled));
        }
        sendErrorFinished(Failed, "already provided");
    } else {
        if (showWarning()) {
            setInfo(i18n("Failed to find package"),
                    i18np("The file could not be found in any packages",
                          "The files could not be found in any packages",
                          m_args.size()));
        }
        sendErrorFinished(NoPackagesFound, "no files found");
    }
}

void PkInstallProvideFiles::addPackage(const PackageKit::Package &package)
{
    if (package.info() != Package::InfoInstalled) {
        SessionTask::addPackage(package);
    } else {
        m_alreadyInstalled = package.name();
    }
}

#include "PkInstallProvideFiles.moc"
