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
#include <KMimeType>

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
    QStringList mimeTypes = Client::instance()->mimeTypes();
    QString lastDirectory = m_urls.at(0).directory();
    QString lastDirectoryNotFiles = m_urls.at(0).directory();
    bool showFullPath = false;
    bool showFullPathNotFiles = false;
    for (int i = 0; i < m_urls.count(); i++) {
        bool supported = false;
        if (QFileInfo(m_urls.at(i).path()).isFile()) {
            kDebug() << "isFIle" << m_urls.at(i);
            KMimeType::Ptr mime = KMimeType::findByFileContent(m_urls.at(i).path());
            foreach (const QString &mimeType, mimeTypes) {
                if (mime->is(mimeType)) {
                    kDebug() << "Found Supported Mime" << mimeType;
                    supported = true;
                    m_files << m_urls.at(i).path();
                    // if the path of all the files is the same
                    // why bothering the user showing a full path?
                    if (m_urls.at(i).directory() != lastDirectory) {
                        showFullPath = true;
                    }
                    lastDirectory = m_urls.at(i).directory();
                    break;
                }
            }
        }

        if (!supported) {
            kDebug() << "~isFIle" << m_urls.at(i);
            notFiles << m_urls.at(i).path();
            if (m_urls.at(i).directory() != lastDirectoryNotFiles) {
                showFullPathNotFiles = true;
            }
            lastDirectoryNotFiles = m_urls.at(i).directory();
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
            KMessageBox::errorListWId(parentWId(),
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
            ret = KMessageBox::questionYesNoListWId(parentWId(),
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
                    // Send the error FIRST otherwise 't' might get deleted
                    sendErrorFinished(Failed, KpkStrings::daemonError(t->error()));
                    if (showWarning()) {
                        KMessageBox::sorryWId(parentWId(),
                                              KpkStrings::daemonError(t->error()),
                                              i18np("Failed to install file",
                                                    "Failed to install files",
                                                    m_files.count()));
                    }
                } else {
                    kTransaction()->setTransaction(t);
                    m_installFilesModel = new KpkSimulateModel(this);
                    connect(t, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                            m_installFilesModel, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
                    if (showProgress()) {
                        kTransaction()->show();
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
                KMessageBox::sorryWId(parentWId(), msg, msg);
            }
            sendErrorFinished(Cancelled, "Aborted");
        }
    } else {
        sendErrorFinished(Failed, "there were no files to install");
    }
}

void PkInstallPackageFiles::installFiles()
{
    SET_PROXY
    Transaction *t = Client::instance()->installFiles(m_files, true);
    if (t->error()) {
        if (showWarning()) {
            KMessageBox::sorryWId(parentWId(),
                                  KpkStrings::daemonError(t->error()),
                                  i18np("Failed to install file",
                                        "Failed to install files",
                                        m_files.count()));
        }
        sendErrorFinished(Failed, KpkStrings::daemonError(t->error()));
    } else {
        kTransaction()->setTransaction(t);
        kTransaction()->setFiles(m_files);
        kTransaction()->show();
    }
}

void PkInstallPackageFiles::transactionFinished(KpkTransaction::ExitStatus status)
{
    kDebug() << "Finished.";
    if (kTransaction()->role() == Enum::RoleSimulateInstallFiles) {
        if (status == KpkTransaction::Success) {
            if (m_installFilesModel->rowCount() > 0) {
                QPointer<KpkRequirements> frm = new KpkRequirements(m_installFilesModel);
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
            sendErrorFinished(Failed, kTransaction()->errorDetails());
        }
    } else {
        switch (status) {
        case KpkTransaction::Success :
            if (showFinished()) {
                KMessageBox::informationWId(parentWId(),
                                            i18np("File was installed successfully",
                                                  "Files were installed successfully",
                                                  kTransaction()->files().count()),
                                            i18np("File was installed successfully",
                                                  "Files were installed successfully",
                                                  kTransaction()->files().count()));
            }
            finishTaskOk();
            break;
        case KpkTransaction::Cancelled :
            sendErrorFinished(Cancelled, "Aborted");
            break;
        case KpkTransaction::Failed :
            if (showWarning()) {
                KMessageBox::errorWId(parentWId(),
                                      kTransaction()->errorDetails(),
                                      i18n("KPackageKit Error"));
            }
            sendErrorFinished(Failed, kTransaction()->errorDetails());
            break;
        }
    }
}

#include "PkInstallPackageFiles.moc"
