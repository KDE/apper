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

#include "KpkInstallProvideFile.h"

#include <KpkReviewChanges.h>
#include <KpkStrings.h>

#include <KLocale>
#include <KMessageBox>

#include <KDebug>

KpkInstallProvideFile::KpkInstallProvideFile(const QStringList &args, QObject *parent)
 : KpkAbstractIsRunning(parent),
   m_args(args)
{
    kDebug() << "install-provide-file" << args;
    Client::instance()->setLocale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
}

KpkInstallProvideFile::~KpkInstallProvideFile()
{
}

void KpkInstallProvideFile::start()
{
    increaseRunning();
    kDebug() << m_args.first();
    QString message = i18np("The following file is required:",
                            "The following files are required:", m_args.size())
                            + QString("<ul><li>%1</li></ul>").arg(m_args.join("</li><li>")) +
                      i18np("Do you want to search for this now?",
                            "Do you want to search for these now?",
                            m_args.size());
    QString parentTitle;
    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        title = i18np("A program wants to install a file",
                      "A program wants to install files",
                      m_args.size());
    } else {
        title = i18np("%1 wants to install a file",
                      "%1 wants to install files",
                      parentTitle,
                      m_args.size());
    }
    QString msg = "<h3>" + title + "</h3>" + message;
    KGuiItem searchBt = KStandardGuiItem::yes();
    searchBt.setText(i18nc("Search for a package that provides these files and install it" ,"Install"));
    searchBt.setIcon(KIcon("edit-find"));
    int ret;
    ret = KMessageBox::questionYesNo(0,
                                     msg,
                                     title,
                                     searchBt);
    if (ret == KMessageBox::Yes) {
        Transaction *t = Client::instance()->searchFile(m_args.first());
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

void KpkInstallProvideFile::kTransactionFinished(KpkTransaction::ExitStatus status)
{
    kDebug() << "Finished.";
    if (status == KpkTransaction::Success) {
        if (!m_alreadyInstalled.isEmpty()) {
            KMessageBox::sorry(0,
                               i18n("The %1 package already provides this file",
                                    m_alreadyInstalled),
                               i18n("Failed to install package"));
        } else if (m_foundPackages.size()) {
            KpkReviewChanges *frm = new KpkReviewChanges(m_foundPackages);
            frm->setTitle(i18np("The following package will be installed",
                                "The following packages will be installed",
                                m_foundPackages.size()));
            connect(frm, SIGNAL(finished()), this, SLOT(decreaseRunning()));
            QTimer::singleShot(0, frm, SLOT(doAction()));
            frm->show();
            // return to avoid the decreaseRunning()
            return;
        } else {
            KMessageBox::sorry(0,
                                i18np("The file could not be found in any packages",
                                      "The files could not be found in any packages", m_args.size()),
                                i18n("Failed to find package"));
        }
    }
    decreaseRunning();
}

void KpkInstallProvideFile::addPackage(PackageKit::Package *package)
{
    if (package->state() != Package::StateInstalled) {
        m_foundPackages.append(package);
    } else {
        m_alreadyInstalled = package->name();
    }
}

#include "KpkInstallProvideFile.moc"
