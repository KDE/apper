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

#include "PkInstallProvideFiles.h"
#include "IntroDialog.h"
#include "FilesModel.h"

#include <KpkStrings.h>

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
    m_introDialog = new IntroDialog(this);
    m_model = new FilesModel(files, QStringList(), this);
    connect(m_model, SIGNAL(rowsInserted(QModelIndex, int, int)),
            this, SLOT(modelChanged()));
    m_introDialog->setModel(m_model);
    setMainWidget(m_introDialog);

    modelChanged();
}

PkInstallProvideFiles::~PkInstallProvideFiles()
{
}

void PkInstallProvideFiles::modelChanged()
{
    QString message = i18np("Do you want to search for this now?",
                            "Do you want to search for these now?",
                            m_args.size());
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
    m_introDialog->setDescription(message);
}

void PkInstallProvideFiles::search()
{
    Transaction *t = new Transaction(this);
    PkTransaction *trans = new PkTransaction(t, this);
    setMainWidget(trans);
    t->searchFiles(m_args.first(), Transaction::FilterArch | Transaction::FilterNewest);
    if (t->error()) {
        QString msg = i18n("Failed to start search file transaction");
        if (showWarning()) {
            setError(msg,
                     KpkStrings::daemonError(t->error()));
        }
        sendErrorFinished(Failed, msg);
    } else {
        connect(t, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
                this, SLOT(searchFinished(PackageKit::Transaction::Exit)));
        connect(t, SIGNAL(package(PackageKit::Package)),
                this, SLOT(addPackage(PackageKit::Package)));
    }
}

void PkInstallProvideFiles::searchFinished(PackageKit::Transaction::Exit status)
{
    if (status == Transaction::ExitSuccess) {
        if (m_alreadyInstalled.size()) {
            if (showWarning()) {
                setInfo(i18n("Failed to install file"),
                        i18n("The %1 package already provides this file",
                             m_alreadyInstalled));
            }
            sendErrorFinished(Failed, "already provided");
        } else if (m_foundPackages.size()) {
            ReviewChanges *frm = new ReviewChanges(m_foundPackages, this);
            setMainWidget(frm);
            setTitle(frm->title());
//            if (frm->exec(operationModes()) == 0) {
//                sendErrorFinished(Failed, "Transaction did not finish with success");
//            } else {
//                finishTaskOk();
//            }
        } else {
            if (showWarning()) {
                setInfo(i18n("Failed to find package"),
                        i18np("The file could not be found in any packages",
                              "The files could not be found in any packages",
                              m_args.size()));
            }
            sendErrorFinished(NoPackagesFound, "no files found");
        }
    } else {
        sendErrorFinished(Failed, "failed to resolve");
    }
}

void PkInstallProvideFiles::addPackage(const PackageKit::Package &package)
{
    if (package.info() != Package::InfoInstalled) {
        m_foundPackages.append(package);
    } else {
        m_alreadyInstalled = package.name();
    }
}

#include "PkInstallProvideFiles.moc"
