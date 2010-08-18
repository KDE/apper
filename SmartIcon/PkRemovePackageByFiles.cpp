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

#include "PkRemovePackageByFiles.h"

#include <KpkReviewChanges.h>
#include <KpkStrings.h>

#include <KLocale>
#include <KMessageBox>
#include <KService>

#include <KDebug>

PkRemovePackageByFiles::PkRemovePackageByFiles(uint xid,
                                               const QStringList &files,
                                               const QString &interaction,
                                               const QDBusMessage &message,
                                               QWidget *parent)
 : KpkAbstractTask(xid, interaction, message, parent),
   m_files(files)
{
}

PkRemovePackageByFiles::~PkRemovePackageByFiles()
{
}

void PkRemovePackageByFiles::start()
{
    int ret = KMessageBox::Yes;
    if (showConfirmSearch()) {
        QString title;
        // this will come from DBus interface
        if (parentTitle.isNull()) {
            title = i18np("A program wants to remove a file",
                        "A program wants to remove files",
                        m_files.size());
        } else {
            title = i18np("%2 wants to remove a file",
                        "%2 wants to remove files",
                        m_files.size(),
                        parentTitle);
        }

        QString message = i18np("The following file is going to be removed:",
                                "The following files are going to be removed:",
                                m_files.size())
                                + QString("<ul><li>%1</li></ul>").arg(m_files.join("</li><li>")) +
                        i18np("Do you want to search for packages containing this file and remove it now?",
                                "Do you want to search for packages containing these files and remove them now?",
                            m_files.size());
        QString msg = "<h3>" + title + "</h3>" + message;
        KGuiItem searchBt = KStandardGuiItem::yes();
        searchBt.setText(i18nc("Search for a package and remove", "Search"));
        searchBt.setIcon(KIcon("edit-find"));
        int ret;
        ret = KMessageBox::questionYesNoWId(parentWId(),
                                            msg,
                                            title,
                                            searchBt);
    }

    if (ret == KMessageBox::Yes) {
        Transaction *t = Client::instance()->searchFiles(m_files,
                                                         Enum::FilterInstalled);
        if (t->error()) {
            QString msg(i18n("Failed to start search file transaction"));
            if (showWarning()) {
                KMessageBox::sorryWId(parentWId(),
                                      KpkStrings::daemonError(t->error()),
                                      msg);
            }
            sendErrorFinished(Failed, "Failed to search for package");
        } else {
            connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                    this, SLOT(resolveFinished(PackageKit::Enum::Exit)));
            connect(t, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                    this, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
            if (showProgress()) {
                kTransaction()->setTransaction(t);
                kTransaction()->show();
            }
        }
    } else {
        sendErrorFinished(Cancelled, "did not agree to search");
    }
}

void PkRemovePackageByFiles::searchFinished(PackageKit::Enum::Exit status)
{
    kDebug() << "Finished.";
    if (status == Enum::ExitSuccess) {
        if (m_foundPackages.size()) {
            kTransaction()->hide();
            KpkReviewChanges *frm = new KpkReviewChanges(m_foundPackages, this, parentWId());
            frm->setTitle(i18np("The following package will be removed",
                                "The following packages will be removed",
                                m_foundPackages.size()));
            if (frm->exec(operationModes()) == 0) {
                sendErrorFinished(Failed, i18n("Transaction did not finish with success"));
            } else {
                finishTaskOk();
            }
        } else {
            if (showWarning()) {
                KMessageBox::sorryWId(parentWId(),
                                      i18np("The file could not be found in any installed package",
                                            "The files could not be found in any installed package",
                                            m_files.size()),
                                      i18n("Could not find %1",
                                           m_files.join(", ")));
            }
            sendErrorFinished(NoPackagesFound, "no package found");
        }
    } else {
        sendErrorFinished(Failed, "failed to search for file");
    }
}

void PkRemovePackageByFiles::addPackage(QSharedPointer<PackageKit::Package> package)
{
    m_foundPackages.append(package);
}

#include "PkRemovePackageByFiles.moc"
