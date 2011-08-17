/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
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

#include "PkInstallPackageFiles.h"

#include "IntroDialog.h"
#include "FilesModel.h"

#include <KLocale>
#include <KDebug>

#include <Daemon>

PkInstallPackageFiles::PkInstallPackageFiles(uint xid,
                                             const QStringList &files,
                                             const QString &interaction,
                                             const QDBusMessage &message,
                                             QWidget *parent)
    : SessionTask(xid, interaction, message, parent)
{
    setWindowTitle(i18n("Install Packages Files"));

    if (Daemon::actions() & Transaction::RoleInstallFiles) {
        m_introDialog = new IntroDialog(this);
        m_introDialog->acceptDrops(i18n("You can drop more files in here"));

        m_model = new FilesModel(files, Daemon::mimeTypes(), this);
        m_introDialog->setModel(m_model);
        connect(m_model, SIGNAL(rowsInserted(QModelIndex, int, int)),
                this, SLOT(modelChanged()));
        setMainWidget(m_introDialog);

        modelChanged();
    } else {
        setError(i18n("Not Supported"),
                 i18n("Your current backend does not support installing files"));
    }
}

PkInstallPackageFiles::~PkInstallPackageFiles()
{
}

void PkInstallPackageFiles::modelChanged()
{
    QStringList files = m_model->files();
    enableButtonOk(!files.isEmpty());

    QString description;
    if (files.isEmpty()) {
        description = i18n("No supported files were provided");
    } else {
        description = i18np("Press <i>Continue</i> if you want to install this file",
                            "Press <i>Continue</i> if you want to install these files",
                            files.size());
    }
    m_introDialog->setDescription(description);

    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        title = i18np("An application wants to install a package",
                      "An application wants to install packages",
                      files.size());
    } else {
        title = i18np("The application <i>%2</i> wants to install a package",
                      "The application <i>%2</i> wants to install packages",
                      files.size(),
                      parentTitle);
    }
    setTitle(title);
}

void PkInstallPackageFiles::commit()
{
    PkTransaction *trans = setTransaction(Transaction::RoleInstallFiles);
    connect(trans, SIGNAL(finished(PkTransaction::ExitStatus)),
            this, SLOT(transactionFinished(PkTransaction::ExitStatus)), Qt::UniqueConnection);
    trans->installFiles(m_model->files());
}

void PkInstallPackageFiles::transactionFinished(PkTransaction::ExitStatus status)
{
     kDebug() << "Finished.";
     switch (status) {
     case PkTransaction::Success :
         if (showFinished()) {
             setFinish(i18n("Installation Complete"),
                       i18np("File was installed successfully",
                             "Files were installed successfully",
                             m_model->files().count()));
         }
         finishTaskOk();
         break;
     case PkTransaction::Cancelled :
         sendErrorFinished(Cancelled, "Aborted");
         break;
     case PkTransaction::Failed :
         if (showWarning()) {
             setError(i18n("Failed to install files"),
                      i18n("Could not install files"));
         }
         sendErrorFinished(Failed, i18n("Could not install files"));
         break;
     }
}

#include "PkInstallPackageFiles.moc"
