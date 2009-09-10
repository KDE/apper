/***************************************************************************
 *   Copyright (C) 2009 by Daniel Nicoletti                                *
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

#include "KpkRemovePackageByFile.h"

#include <KpkReviewChanges.h>
#include <KpkStrings.h>

#include <KLocale>
#include <KMessageBox>
#include <KService>

#include <KDebug>

KpkRemovePackageByFile::KpkRemovePackageByFile(const QStringList &args, QObject *parent)
 : KpkAbstractIsRunning(parent),
   m_args(args)
{
    kDebug() << "remove-package-by-file" << args;
    KService *srv = new KService("oi", args.first(), "j");
    kDebug() << "remove-package-by-exec" << srv->exec() << srv->path();
    Client::instance()->setLocale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
}

KpkRemovePackageByFile::~KpkRemovePackageByFile()
{
}

void KpkRemovePackageByFile::start()
{
    increaseRunning();
    kDebug() << m_args.first();
    QString parentTitle;
    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        title = i18np("A program wants to remove a file",
                      "A program wants to remove files",
                      m_args.size());
    } else {
        title = i18np("%1 wants to remove a file",
                      "%1 wants to remove files",
                      parentTitle,
                      m_args.size());
    }

    QString message = i18np("The following file is going to be removed:",
                            "The following files are going to be removed:",
                            m_args.size())
                            + QString("<ul><li>%1</li></ul>").arg(m_args.join("</li><li>")) +
                      i18np("Do you want to search for packages containing this file and remove it now?",
                            "Do you want to search for packages containing these files and remove them now?",
                           m_args.size());
    QString msg = "<h3>" + title + "</h3>" + message;
    KGuiItem searchBt = KStandardGuiItem::yes();
    searchBt.setText(i18nc("Search for a package and remove", "Search"));
    searchBt.setIcon(KIcon::KIcon("edit-find"));
    int ret;
    ret = KMessageBox::questionYesNo(0,
                                     msg,
                                     title,
                                     searchBt);
    if (ret == KMessageBox::Yes) {
        Transaction *t = Client::instance()->searchFile(m_args.first(),
                                                        Client::FilterInstalled);
        if (t->error()) {
            KMessageBox::sorry(0,
                               KpkStrings::daemonError(t->error()),
                               i18n("Failed to start search file transaction"));
        } else {
            KpkTransaction *trans = new KpkTransaction(t, KpkTransaction::CloseOnFinish);
            connect(trans, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
                    this, SLOT(kTransactionFinished(KpkTransaction::ExitStatus)));
            connect(t, SIGNAL(package(PackageKit::Package *)),
                    this, SLOT(addPackage(PackageKit::Package *)));
            trans->show();
            // return to avoid the decreaseRunning()
            return;
        }
    }
    decreaseRunning();
}

void KpkRemovePackageByFile::kTransactionFinished(KpkTransaction::ExitStatus status)
{
    kDebug() << "Finished.";
    if (status == KpkTransaction::Success) {
        if (m_foundPackages.size()) {
            KpkReviewChanges *frm = new KpkReviewChanges(m_foundPackages);
            frm->setTitle(i18np("The following package will be removed",
                                "The following packages will be removed",
                                m_foundPackages.size()));
            connect(frm, SIGNAL(finished()), this, SLOT(decreaseRunning()));
            frm->show();
            // return to avoid the decreaseRunning()
            return;
        } else {
            KMessageBox::sorry(0,
                                i18np("The file could not be found in any installed package",
                                      "The files could not be found in any installed package", m_args.size()),
                                i18n("Could not find %1", m_args.join(", ")));
        }
    }
    decreaseRunning();
}

void KpkRemovePackageByFile::addPackage(PackageKit::Package *package)
{
    m_foundPackages.append(package);
}

#include "KpkRemovePackageByFile.moc"
