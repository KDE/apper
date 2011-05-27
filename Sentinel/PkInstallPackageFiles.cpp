/***************************************************************************
 *   Copyright (C) 2008-2010 by Daniel Nicoletti                           *
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

#include "PkInstallPackageFiles.h"

#include "IntroDialog.h"
#include "FilesModel.h"

#include <KpkSimulateModel.h>
#include <KpkRequirements.h>
#include <KpkStrings.h>
#include <KpkMacros.h>
#include <PkTransaction.h>

#include <KLocale>
#include <KMessageBox>
#include <KMimeType>

#include <KDebug>

#include <QWeakPointer>
#include <QFileInfo>
#include <QCoreApplication>

#include <Daemon>

PkInstallPackageFiles::PkInstallPackageFiles(uint xid,
                                             const QStringList &files,
                                             const QString &interaction,
                                             const QDBusMessage &message,
                                             QWidget *parent)
 : KpkAbstractTask(xid, interaction, message, parent)
{
    m_introDialog = new IntroDialog(this);
    m_model = new FilesModel(files, Daemon::mimeTypes(), this);
    connect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
            this, SLOT(modelChanged()));
    m_introDialog->setModel(m_model);
    setMainWidget(m_introDialog);

    modelChanged();
}

PkInstallPackageFiles::~PkInstallPackageFiles()
{
}

void PkInstallPackageFiles::modelChanged()
{
    QString message;
    message = i18np("Press <i>Continue</i> if you want to install this file:",
                    "Press <i>Continue</i> if you want to install these files:",
                    m_model->rowCount());
    enableButtonOk(m_model->rowCount() > 0);
    m_introDialog->setDescription(message);
}

void PkInstallPackageFiles::slotButtonClicked(int bt)
{
    if (bt == KDialog::Ok) {
        if (mainWidget() == m_introDialog) {
            PkTransaction *trans = new PkTransaction(0, this);
            setMainWidget(trans);
            trans->installFiles(m_model->files());
            enableButtonOk(false);
        }
    } else {
        sendErrorFinished(Cancelled, "Aborted");
    }
    KDialog::slotButtonClicked(bt);
}

void PkInstallPackageFiles::start()
{
    // check if there were "false" files
//     if (notFiles.count()) {
//         if (!showFullPathNotFiles) {
//             for (int i = 0; i < notFiles.count(); i++) {
//                 notFiles[i] = KUrl(notFiles.at(i)).fileName();
//             }
//         }
//         if (showWarning()) {
//             KMessageBox::errorListWId(parentWId(),
//                                       i18np("This item is not supported by your backend, "
//                                             "or it is not a file. ",
//                                             "These items are not supported by your "
//                                             "backend, or they are not files.",
//                                             notFiles.count()),
//                                       notFiles,
//                                       i18n("Impossible to install"));
//         }
//         sendErrorFinished(Failed, "Files not supported by your backend or they are not files");
//         return;
//     }

//     if (m_files.count()) {
//         QStringList displayFiles = m_files;
//         if (!showFullPath) {
//             for(int i = 0; i < displayFiles.count(); i++) {
//                 displayFiles[i] = KUrl(displayFiles.at(i)).fileName();
//             }
//         }
// 
//         KGuiItem installBt = KStandardGuiItem::yes();
//         installBt.setText(i18n("Install"));
// 
//         int ret = KMessageBox::Yes;
//         if (showConfirmSearch()) {
//             ret = KMessageBox::questionYesNoListWId(parentWId(),
//                                                     i18np("Do you want to install this file?",
//                                                           "Do you want to install these files?",
//                                                           displayFiles.count()),
//                                                     displayFiles,
//                                                     i18n("Install?"),
//                                                     installBt);
//         }
//         if (ret == KMessageBox::Yes) {
// 
//         } else {
//             QString msg = i18np("The file was not installed",
//                                 "The files were not installed",
//                                 displayFiles.count());
//             if (showWarning()) {
//                 KMessageBox::sorryWId(parentWId(), msg, msg);
//             }
//             sendErrorFinished(Cancelled, "Aborted");
//         }
//     } else {
//         sendErrorFinished(Failed, "there were no files to install");
//     }
}

void PkInstallPackageFiles::transactionFinished(PkTransaction::ExitStatus status)
{
    kDebug() << "Finished.";
    if (kTransaction()->transaction()->role() == Transaction::RoleSimulateInstallFiles) {
        if (status == PkTransaction::Success) {
//             if (m_installFilesModel->rowCount() > 0) {
//                 QWeakPointer<KpkRequirements> frm = new KpkRequirements(m_installFilesModel);
//                 if (frm.data()->exec() == QDialog::Accepted) {
// //                     installFiles();
//                 } else {
//                     sendErrorFinished(Cancelled, "Aborted");
//                 }
//                 frm.data()->deleteLater();
//             } else {
// //                 installFiles();
//             }
        } else {
            sendErrorFinished(Failed, kTransaction()->transaction()->errorDetails());
        }
    } else {
        switch (status) {
        case PkTransaction::Success :
            if (showFinished()) {
                KMessageBox::informationWId(parentWId(),
                                            i18np("File was installed successfully",
                                                  "Files were installed successfully",
                                                  kTransaction()->transaction()->files().count()),
                                            i18np("File was installed successfully",
                                                  "Files were installed successfully",
                                                  kTransaction()->transaction()->files().count()));
            }
            finishTaskOk();
            break;
        case PkTransaction::Cancelled :
            sendErrorFinished(Cancelled, "Aborted");
            break;
        case PkTransaction::Failed :
            if (showWarning()) {
                KMessageBox::errorWId(parentWId(),
                                      kTransaction()->transaction()->errorDetails(),
                                      i18n("KPackageKit Error"));
            }
            sendErrorFinished(Failed, kTransaction()->transaction()->errorDetails());
            break;
        }
    }
}

#include "PkInstallPackageFiles.moc"
