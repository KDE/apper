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

#include "PkRemovePackageByFiles.h"

#include "IntroDialog.h"
#include "FilesModel.h"

#include <KpkStrings.h>
#include <PkTransactionDialog.h>

#include <KLocale>
#include <KService>

#include <KDebug>

PkRemovePackageByFiles::PkRemovePackageByFiles(uint xid,
                                               const QStringList &files,
                                               const QString &interaction,
                                               const QDBusMessage &message,
                                               QWidget *parent)
 : SessionTask(xid, interaction, message, parent),
   m_files(files)
{
    setWindowTitle(i18n("Remove Packages that Provides Files"));

    m_introDialog = new IntroDialog(this);
    m_model = new FilesModel(files, QStringList(), this);
    connect(m_model, SIGNAL(rowsInserted(QModelIndex, int, int)),
            this, SLOT(modelChanged()));
    m_introDialog->setModel(m_model);
    setMainWidget(m_introDialog);

    modelChanged();
}

PkRemovePackageByFiles::~PkRemovePackageByFiles()
{
}

void PkRemovePackageByFiles::modelChanged()
{
    QString message;
    message = i18n("Press <i>Continue</i> if you want to remove");
    enableButtonOk(!m_model->files().isEmpty());
    m_introDialog->setDescription(message);

    QString title;
    // this will come from DBus interface
    if (!m_model->files().isEmpty() && parentTitle.isNull()) {
        if (m_model->onlyApplications()) {
            title = i18np("Do you want to remove the following application:",
                          "Do you want to remove the following applications:",
                          m_model->rowCount());
        } else {
            title = i18np("Do you want to remove a package:",
                          "Do you want to remove packages:",
                          m_model->rowCount());
        }
    } else if (!m_model->files().isEmpty()) {
        if (m_model->onlyApplications()) {
            title = i18np("The application <i>%2</i> is asking to remove an application:",
                          "The application <i>%2</i> is asking to remove applications:",
                          m_model->rowCount(),
                          parentTitle);
        } else {
            title = i18np("The application <i>%2</i> is asking to remove a package:",
                          "The application <i>%2</i> is asking to remove packages:",
                          m_model->rowCount(),
                          parentTitle);
        }
    } else {
        title = i18n("No application was found");
    }
    setTitle(title);
}

void PkRemovePackageByFiles::search()
{
    Transaction *t = new Transaction(this);
    PkTransaction *trans = setTransaction(t);
    connect(trans, SIGNAL(finished(PkTransaction::ExitStatus)),
            this, SLOT(searchFinished(PkTransaction::ExitStatus)), Qt::UniqueConnection);
    connect(t, SIGNAL(package(PackageKit::Package)),
            this, SLOT(addPackage(PackageKit::Package)));
    t->searchFiles(m_model->files(), Transaction::FilterInstalled);
    if (t->error()) {
        QString msg(i18n("Failed to start search file transaction"));
        if (showWarning()) {
            setError(msg,
                     KpkStrings::daemonError(t->error()));
        }
        sendErrorFinished(Failed, "Failed to search for package");
    }
}

void PkRemovePackageByFiles::notFound()
{
    if (showWarning()) {
        setInfo(i18n("Could not find %1", m_files.join(", ")),
                i18np("The file could not be found in any installed package",
                      "The files could not be found in any installed package",
                      m_files.size()));
    }
    sendErrorFinished(NoPackagesFound, "no package found");
}

#include "PkRemovePackageByFiles.moc"
