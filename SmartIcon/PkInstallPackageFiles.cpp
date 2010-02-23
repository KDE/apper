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

#include <KpkSimulateModel.h>
#include <KpkRequirements.h>
#include <KpkStrings.h>
#include <KpkMacros.h>

#include <KLocale>
#include <KMessageBox>

#include <KDebug>

PkInstallPackageFiles::PkInstallPackageFiles(uint xid,
                                             const QStringList &files,
                                             const QString &interaction,
                                             const QDBusMessage &message,
                                             QWidget *parent)
 : KpkAbstractTask(xid, interaction, message, parent),
   m_urls(files)
{
}

PkInstallPackageFiles::~PkInstallPackageFiles()
{
}

void PkInstallPackageFiles::start()
{
    QStringList notFiles;
    QString lastDirectory = m_urls.at(0).directory();
    QString lastDirectoryNotFiles = m_urls.at(0).directory();
    bool showFullPath = false;
    bool showFullPathNotFiles = false;
    for (int i = 0; i < m_urls.count(); i++) {
        if (QFileInfo(m_urls.at(i).path()).isFile()) {
            qDebug() << "isFIle";
            m_files << m_urls.at(i).path();
            // if the path of all the files is the same
            // why bothering the user showing a full path?
            if (m_urls.at(i).directory() != lastDirectory) {
                showFullPath = true;
            }
            lastDirectory = m_urls.at(i).directory();
        } else {
            qDebug() << "~isFIle";
            notFiles << m_urls.at(i).path();
            if (m_urls.at(i).directory() != lastDirectoryNotFiles) {
                showFullPathNotFiles = true;
            }
            lastDirectoryNotFiles =m_urls.at(i).directory();
        }
    }

    // check if there were "false" files
    if (notFiles.count()) {
        if (!showFullPathNotFiles) {
            for(int i = 0; i < notFiles.count(); i++) {
                notFiles[i] = KUrl(notFiles.at(i)).fileName();
            }
        }
        if (showWarning()) {
            KMessageBox::errorList(0,
                                    i18np("This item is not supported by your backend, "
                                          "or it is not a file. ",
                                          "These items are not supported by your "
                                          "backend, or they are not files.",
                                          notFiles.count()),
                                   notFiles,
                                   i18n("Impossible to install"));
        }
        sendErrorFinished(Failed, "Files not supported by your backend or they are not files");
        return;
    }

    if (m_files.count()) {
        QStringList displayFiles = m_files;
        if (!showFullPath) {
            for(int i = 0; i < displayFiles.count(); i++) {
                displayFiles[i] = KUrl(displayFiles.at(i)).fileName();
            }
        }

        KGuiItem installBt = KStandardGuiItem::yes();
        installBt.setText(i18n("Install"));

        int ret = KMessageBox::Yes;
        if (showConfirmSearch()) {
            ret = KMessageBox::questionYesNoList(0,
                                                 i18np("Do you want to install this file?",
                                                       "Do you want to install these files?",
                                                       displayFiles.count()),
                                                 displayFiles,
                                                 i18n("Install?"),
                                                 installBt);
        }
        if (ret == KMessageBox::Yes) {
            if (Client::instance()->actions() & Enum::RoleSimulateInstallFiles &&
                showConfirmDeps()) {
                // TODO
                Transaction *t;
                t = Client::instance()->simulateInstallFiles(m_files);
                if (t->error()) {
                    if (showWarning()) {
                        KMessageBox::sorry(0,
                                           KpkStrings::daemonError(t->error()),
                                           i18np("Failed to install file",
                                                 "Failed to install files",
                                                 m_files.count()));
                    }
                    sendErrorFinished(Failed, KpkStrings::daemonError(t->error()));
                } else {
                    m_installFilesModel = new KpkSimulateModel(this);
                    connect(t, SIGNAL(package(PackageKit::QSharedPointer<PackageKit::Package>)),
                            m_installFilesModel, SLOT(addPackage(PackageKit::QSharedPointer<PackageKit::Package>)));
                    connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                            this, SLOT(simulateFinished(PackageKit::Transaction::ExitStatus, uint)));
                    if (showProgress()) {
                        KpkTransaction *trans;
                        trans = new KpkTransaction(t,
                                                   KpkTransaction::CloseOnFinish |
                                                   KpkTransaction::Modal);
                        trans->show();
                    }
                }
            } else {
                installFiles();
            }
        } else {
            QString msg = i18np("The file was not installed",
                                "The files were not installed",
                                displayFiles.count());
            if (showWarning()) {
                KMessageBox::sorry(0,
                                   msg,
                                   msg);
            }
            sendErrorFinished(Cancelled, "Aborted");
        }
    } else {
        sendErrorFinished(Failed, "there were no files to install");
    }
}

void PkInstallPackageFiles::simulateFinished(PackageKit::Enum::Exit status, uint runtime)
{
    Q_UNUSED(runtime)
    if (status == Enum::ExitSuccess) {
        if (m_installFilesModel->rowCount() > 0) {
            KpkRequirements *frm = new KpkRequirements(m_installFilesModel);
            if (frm->exec() == QDialog::Accepted) {
                installFiles();
            } else {
                sendErrorFinished(Cancelled, "Aborted");
            }
            delete frm;
        } else {
            installFiles();
        }
    } else {
        sendErrorFinished(Failed, "failed to simulate a package file install");
    }
}

void PkInstallPackageFiles::installFiles()
{
    SET_PROXY
    Transaction *t = Client::instance()->installFiles(m_files, true);
    if (t->error()) {
        if (showWarning()) {
            KMessageBox::sorry(0,
                               KpkStrings::daemonError(t->error()),
                               i18np("Failed to install file",
                                     "Failed to install files",
                                     m_files.count()));
        }
        sendErrorFinished(Failed, KpkStrings::daemonError(t->error()));
    } else {
        KpkTransaction *trans = new KpkTransaction(t);
        // TODO we need to keep an eye here
//         connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
//                 this, SLOT(installFinished(PackageKit::Transaction::ExitStatus, uint)));
        connect(trans, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
                this, SLOT(installFilesFinished(KpkTransaction::ExitStatus)));
        trans->show();
        m_transactionFiles[trans] = m_files;
    }
}

void PkInstallPackageFiles::installFilesFinished(KpkTransaction::ExitStatus status)
{
    kDebug() << "Finished.";
    KpkTransaction *transaction = (KpkTransaction *) sender();
    switch (status) {
    case KpkTransaction::Success :
        if (showFinished()) {
            KMessageBox::information(0,
                                     i18np("File was installed successfully",
                                           "Files were installed successfully",
                                           m_transactionFiles[transaction].count()),
                                     i18np("File was installed successfully",
                                           "Files were installed successfully",
                                           m_transactionFiles[transaction].count()));
        }
        finishTaskOk();
        // return to avoid the finished() since in the above it's already emmited
        return;
    case KpkTransaction::Cancelled :
        m_transactionFiles.remove(transaction);
        sendErrorFinished(Cancelled, "Aborted");
        break;
    case KpkTransaction::Failed :
        m_transactionFiles.remove(transaction);
        if (showWarning()) {
            KMessageBox::error(0,
                               i18n("An error occurred."),
                               i18n("KPackageKit Error"));
        }
        sendErrorFinished(Failed, "An error occurred");
        break;
    case KpkTransaction::ReQueue :
        kDebug() << "ReQueue";
        SET_PROXY
        transaction->setTransaction(Client::instance()->installFiles(m_transactionFiles[transaction], false));
        // return to avoid the finished()
        return;
    }
}

#include "PkInstallPackageFiles.moc"
