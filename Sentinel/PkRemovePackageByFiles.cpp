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

#include "PkRemovePackageByFiles.h"

#include "IntroDialog.h"
#include "FilesModel.h"

#include <KpkStrings.h>
#include <PkTransactionDialog.h>

#include <KLocale>
#include <KMessageBox>
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
    m_introDialog = new IntroDialog(this);
    m_model = new FilesModel(files, QStringList(), this);
    connect(m_model, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
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
                          "Do you want to is asking to remove the following applications:",
                          m_model->rowCount());
        } else {
            title = i18np("Do you want to to remove a package:",
                          "Do you want to to remove packages:",
                          m_model->rowCount());
        }
    } else if (!m_model->files().isEmpty()) {
        if (m_model->onlyApplications()) {
            title = i18np("The application <i>%2</i> is asking to remove an application:",
                          "The application <i>%2</i> is asking to remove an application:",
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

void PkRemovePackageByFiles::slotButtonClicked(int bt)
{
    if (bt == KDialog::Ok) {
        if (mainWidget() == m_introDialog) {
            Transaction *t = new Transaction(QString());
            PkTransaction *trans = new PkTransaction(t, this);
            connect(trans, SIGNAL(finished(PkTransaction::ExitStatus)),
                    this, SLOT(transactionFinished(PkTransaction::ExitStatus)));
            connect(t, SIGNAL(package(const PackageKit::Package &)),
                    this, SLOT(addPackage(const PackageKit::Package &)));
            setMainWidget(trans);
            t->searchFiles(m_model->files(), Transaction::FilterInstalled);
            setTitle(trans->title());
            enableButtonOk(false);
            if (t->error()) {
                QString msg(i18n("Failed to start search file transaction"));
                if (showWarning()) {
//                    KMessageBox::sorryWId(this,
//                                          KpkStrings::daemonError(t->error()),
//                                          msg);
                }
                sendErrorFinished(Failed, "Failed to search for package");
            }
        }
    } else {
        KDialog::slotButtonClicked(bt);
        sendErrorFinished(Cancelled, "Aborted");
    }
}

void PkRemovePackageByFiles::start()
{
//    int ret = KMessageBox::Yes;
//    if (showConfirmSearch()) {
//        QString title;
//        // this will come from DBus interface
//        if (parentTitle.isNull()) {
//            title = i18np("A program wants to remove a file",
//                        "A program wants to remove files",
//                        m_files.size());
//        } else {
//            title = i18np("%2 wants to remove a file",
//                        "%2 wants to remove files",
//                        m_files.size(),
//                        parentTitle);
//        }

//        QString message = i18np("The following file is going to be removed:",
//                                "The following files are going to be removed:",
//                                m_files.size())
//                                + QString("<ul><li>%1</li></ul>").arg(m_files.join("</li><li>")) +
//                        i18np("Do you want to search for packages containing this file and remove it now?",
//                                "Do you want to search for packages containing these files and remove them now?",
//                            m_files.size());
//        QString msg = "<h3>" + title + "</h3>" + message;
//        KGuiItem searchBt = KStandardGuiItem::yes();
//        searchBt.setText(i18nc("Search for a package and remove", "Search"));
//        searchBt.setIcon(KIcon("edit-find"));
//        ret = KMessageBox::questionYesNoWId(parentWId(),
//                                            msg,
//                                            title,
//                                            searchBt);
//    }

//    if (ret == KMessageBox::Yes) {
//        Transaction *t = new Transaction(QString());
//        PkTransaction *trans = new PkTransaction(t, this);
//        connect(t, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
//                this, SLOT(searchFinished(PackageKit::Transaction::Exit)));
//        connect(t, SIGNAL(package(const PackageKit::Package &)),
//                this, SLOT(addPackage(const PackageKit::Package &)));
//        t->searchFiles(m_files, Transaction::FilterInstalled);
//        Transaction::InternalError error = t->error();
//        if (error) {
//            QString msg(i18n("Failed to start search file transaction"));
//            if (showWarning()) {
//                KMessageBox::sorryWId(parentWId(),
//                                      KpkStrings::daemonError(error),
//                                      msg);
//            }
//            sendErrorFinished(Failed, "Failed to search for package");
//        } else {
//            if (showProgress()) {
//                setMainWidget(trans);
//            }
//        }
//    } else {
//        sendErrorFinished(Cancelled, "did not agree to search");
//    }
}

void PkRemovePackageByFiles::transactionFinished(PkTransaction::ExitStatus status)
{
    kDebug() << "Finished." << (status == PkTransaction::Success) << m_foundPackages.size();
    if (status == PkTransaction::Success) {
        if (m_foundPackages.size()) {
            ReviewChanges *frm = new ReviewChanges(m_foundPackages, this, parentWId());
            setTitle(frm->title());
            setMainWidget(frm);
//            if (frm->exec(operationModes()) == 0) {
//                sendErrorFinished(Failed, i18n("Transaction did not finish with success"));
//            } else {
//                finishTaskOk();
//            }
        } else {
            if (showWarning()) {
                setInfo(i18n("Could not find %1", m_files.join(", ")),
                        i18np("The file could not be found in any installed package",
                              "The files could not be found in any installed package",
                              m_files.size()));
            }
            sendErrorFinished(NoPackagesFound, "no package found");
        }
    } else {
        sendErrorFinished(Failed, "failed to search for file");
    }
}

void PkRemovePackageByFiles::addPackage(const PackageKit::Package &package)
{
    m_foundPackages.append(package);
}

#include "PkRemovePackageByFiles.moc"
