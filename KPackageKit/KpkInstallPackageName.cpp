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

#include "KpkInstallPackageName.h"
#include <KpkReviewChanges.h>

#include <KLocale>
#include <KMessageBox>

#include <KDebug>

KpkInstallPackageName::KpkInstallPackageName(const QStringList &args, QObject *parent)
 : KpkAbstractIsRunning(parent),
   m_args(args)
{
    kDebug() << "install-package-name!" << args;
    Client::instance()->setLocale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
}

KpkInstallPackageName::~KpkInstallPackageName()
{
}

void KpkInstallPackageName::start()
{
    increaseRunning();
    kDebug() << m_args.first();
    QString message = i18np("An additional package is required:",
                            "Additional packages are required:", m_args.size())
                            + QString("<ul><li>%1</li></ul>").arg(m_args.join("</li><li>")) +
                      i18np("Do you want to search for and install this package now?",
                            "Do you want to search for and install these packages now?",
                           m_args.size());
    QString parentTitle;
    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        title = i18np("A program wants to install a package",
                      "A program wants to install packages",
                      m_args.size());
    } else {
        title = i18np("%1 wants to install a package",
                      "%1 wants to install packages",
                      parentTitle,
                      m_args.size());
    }
    QString msg = "<h3>" + title + "</h3>" + message;
    KGuiItem searchBt = KStandardGuiItem::yes();
    searchBt.setText(i18nc("Search for a package and install it" ,"Install"));
    searchBt.setIcon(KIcon::KIcon("edit-find"));
    int ret;
    ret = KMessageBox::questionYesNo(0,
                                     msg,
                                     title,
                                     searchBt);
    if (ret == KMessageBox::Yes) {
        if (Transaction *t = Client::instance()->resolve(m_args,
                                                         Client::FilterNotInstalled)) {
            KpkTransaction *trans = new KpkTransaction(t, KpkTransaction::CloseOnFinish);
            connect(trans, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
                    this, SLOT(kTransactionFinished(KpkTransaction::ExitStatus)));
            connect(t, SIGNAL(package(PackageKit::Package *)),
                    this, SLOT(addPackage(PackageKit::Package *)));
            trans->show();
            // return to avoid the decreaseRunning()
            return;
        } else {
            KMessageBox::error(0,
                               i18n("Failed to start resolve transaction"),
                               i18n("Failed to start resolve transaction"));
        }
    }
    decreaseRunning();
}

void KpkInstallPackageName::kTransactionFinished(KpkTransaction::ExitStatus status)
{
    kDebug() << "Finished.";
    if (status == KpkTransaction::Success) {
        if (m_foundPackages.size()) {
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
                                i18np("The package could not be found in any software source",
                                      "The packages could not be found in any software source", m_args.size()),
                                i18n("Could not find %1", m_args.join(", ")));
        }
    }
    decreaseRunning();
}

void KpkInstallPackageName::addPackage(PackageKit::Package *package)
{
    m_foundPackages.append(package);
}

#include "KpkInstallPackageName.moc"
