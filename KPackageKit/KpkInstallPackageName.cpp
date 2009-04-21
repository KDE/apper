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

KpkInstallPackageName::KpkInstallPackageName( QObject *parent )
 : QObject(parent)/*, m_running(0)*/
{
    Client::instance()->setLocale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
}

KpkInstallPackageName::~KpkInstallPackageName()
{
}

void KpkInstallPackageName::installPackageName(const QStringList &items)
{
    kDebug() << items.first();
    QString message = i18np("An additional package is required:",
                            "Additional packages are required:", items.size())
                            + QString("<ul><li>%1</li></ul>").arg(items.join("</li><li>")) +
                      i18np("Do you want to search for and install this package now?",
                            "Do you want to search for and install these packages now?",
                           items.size());
    QString parentTitle;
    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        title = i18np("A program wants to install a package",
                      "A program wants to install packages",
                      items.size());
    } else {
        title = i18np("%1 wants to install a package",
                      "%1 wants to install packages",
                      parentTitle,
                      items.size());
    }
    QString msg = "<h3>" + title + "</h3>" + message;
    KGuiItem searchBt = KStandardGuiItem::yes();
    searchBt.setText(i18nc("Search for a new mime type" ,"Install"));
    searchBt.setIcon(KIcon::KIcon("edit-find"));
    int ret;
    ret = KMessageBox::questionYesNo(0,
                                     msg,
                                     title,
                                     searchBt);
    if (ret == KMessageBox::Yes) {
        if (Transaction *t = Client::instance()->resolve(items,
                                                         Client::FilterNotInstalled)) {
            KpkTransaction *trans = new KpkTransaction(t, KpkTransaction::CloseOnFinish);
            connect(trans, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
                    this, SLOT(kTransactionFinished(KpkTransaction::ExitStatus)));
            connect(t, SIGNAL(package(PackageKit::Package *)),
                    this, SLOT(addPackage(PackageKit::Package *)));
            trans->show();
            // to skip the appClose()
            return;
        } else {
            KMessageBox::error(0,
                               i18n("Failed to search for provides"),
                               i18n("Failed to search for provides"));
        }
    }
    emit appClose();
}

void KpkInstallPackageName::kTransactionFinished(KpkTransaction::ExitStatus status)
{
    kDebug() << "Finished.";
//     KpkTransaction *transaction = (KpkTransaction *) sender();
    if (status == KpkTransaction::Success) {
        if (m_foundPackages.size()) {
            KpkReviewChanges *frm = new KpkReviewChanges(m_foundPackages, KpkReviewChanges::AutoStart);
            frm->setTitle(i18np("Application that can open this type of file",
                                "Applications that can open this type of file", m_foundPackages.size()));
            // to avoid appClose
            return;
        } else {
            KMessageBox::sorry(0,
                                i18n("No new applications can be found to handle this type of file"),
                                i18n("Failed to find software"));
        }
    }
    emit appClose();
}

void KpkInstallPackageName::addPackage(PackageKit::Package *package)
{
    m_foundPackages.append(package);
}

#include "KpkInstallPackageName.moc"
