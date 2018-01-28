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

#include <Daemon>

#include <PkStrings.h>

#include <KLocalizedString>
#include <KService>

#include <QLoggingCategory>

PkRemovePackageByFiles::PkRemovePackageByFiles(uint xid,
                                               const QStringList &files,
                                               const QString &interaction,
                                               const QDBusMessage &message,
                                               QWidget *parent) :
    SessionTask(xid, interaction, message, parent)
{
    setWindowTitle(i18n("Remove Packages that Provides Files"));

    m_introDialog = new IntroDialog(this);
    m_introDialog->acceptDrops(i18n("You can drop more files in here"));
    m_model = new FilesModel(files, QStringList(), this);
    connect(m_model, &FilesModel::rowsInserted, this, &PkRemovePackageByFiles::modelChanged);
    m_introDialog->setModel(m_model);
    setMainWidget(m_introDialog);

    modelChanged();
}

PkRemovePackageByFiles::~PkRemovePackageByFiles()
{
}

void PkRemovePackageByFiles::modelChanged()
{
    QStringList files = m_model->files();
    enableButtonOk(!files.isEmpty());

    QString description;
    if (files.isEmpty()) {
        description = i18n("No supported files were provided");
    } else {
        if (m_model->onlyApplications()) {
            description = i18np("Do you want to remove the following application?",
                                "Do you want to remove the following applications?",
                                files.size());
        } else {
            description = i18np("Do you want to search for a package providing this file?",
                                "Do you want to search for a package providing these files?",
                                files.size());
        }
    }
    m_introDialog->setDescription(description);

    QString title;
    // this will come from DBus interface
    if (!files.isEmpty() && parentTitle.isNull()) {
        if (m_model->onlyApplications()) {
            title = i18np("An application is asking to remove an application",
                          "An application is asking to remove applications",
                          files.size());
        } else {
            title = i18np("An application is asking to remove a file",
                          "An application is asking to remove files",
                          files.size());
        }
    } else if (!files.isEmpty()) {
        if (m_model->onlyApplications()) {
            title = i18np("The application <i>%2</i> is asking to remove an application",
                          "The application <i>%2</i> is asking to remove applications",
                          files.size(),
                          parentTitle);
        } else {
            title = i18np("The application <i>%2</i> is asking to remove a file",
                          "The application <i>%2</i> is asking to remove files",
                          files.size(),
                          parentTitle);
        }
    } else {
        title = i18n("No application was found");
    }
    setTitle(title);
}

void PkRemovePackageByFiles::search()
{
    m_files = m_model->files();
    searchFinished(PkTransaction::Success);
}

void PkRemovePackageByFiles::searchFinished(PkTransaction::ExitStatus status)
{
    if (status == PkTransaction::Success) {
        if (!m_files.isEmpty()) {
            QString file = m_files.takeFirst();

            auto transaction = new PkTransaction(this);
            transaction->setupTransaction(Daemon::searchFiles(file, Transaction::FilterInstalled));
            setTransaction(Transaction::RoleSearchFile, transaction);
            connect(transaction, &PkTransaction::finished, this, &PkRemovePackageByFiles::searchFinished, Qt::UniqueConnection);
            connect(transaction, &PkTransaction::package, this, &PkRemovePackageByFiles::addPackage);
        } else {
            // we are done resolving
            SessionTask::searchFinished(status);
        }
    } else {
        // we got an error...
        SessionTask::searchFinished(status);
    }
}

void PkRemovePackageByFiles::notFound()
{
    if (showWarning()) {
        QStringList files = m_model->files();
        setInfo(i18n("Could not find %1", files.join(QLatin1String(", "))),
                i18np("The file could not be found in any installed package",
                      "The files could not be found in any installed package",
                      files.size()));
    }
    sendErrorFinished(NoPackagesFound, QLatin1String("no package found"));
}

#include "moc_PkRemovePackageByFiles.cpp"
