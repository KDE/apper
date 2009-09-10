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

#include "KpkInstallMimeType.h"

#include <KpkReviewChanges.h>
#include <KpkStrings.h>

#include <KLocale>
#include <KMessageBox>

#include <KDebug>

KpkInstallMimeType::KpkInstallMimeType(const QStringList &args, QObject *parent)
 : KpkAbstractIsRunning(parent),
   m_args(args)
{
    Client::instance()->setLocale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
}

KpkInstallMimeType::~KpkInstallMimeType()
{
}

void KpkInstallMimeType::start()
{
    increaseRunning();
    kDebug() << m_args.first();
    QString message = i18n("An additional program is required to open this type of file:<br />%1<br/>"
                           "Do you want to search for a program to open this file type now?",
                           m_args.first());
    QString parentTitle;
    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        title = i18np("A program requires a new mime type",
                      "A program requires new mime types",
                      m_args.size());
    } else {
        title = i18np("%1 requires a new mime type",
                      "%1 requires new mime types",
                      parentTitle,
                      m_args.size());
    }
    QString msg = "<h3>" + title + "</h3>" + message;
    KGuiItem searchBt = KStandardGuiItem::yes();
    searchBt.setText(i18nc("Search for a new mime type" ,"Search"));
    searchBt.setIcon(KIcon::KIcon("edit-find"));
    int ret;
    ret = KMessageBox::questionYesNo(0,
                                     msg,
                                     title,
                                     searchBt);
    if (ret == KMessageBox::Yes) {
        Transaction *t = Client::instance()->whatProvides(Client::ProvidesMimetype,
                                                          m_args.first(),
                                                          Client::FilterNotInstalled);
        if (t->error()) {
            KMessageBox::sorry(0,
                               KpkStrings::daemonError(t->error()),
                               i18n("Failed to search for provides"));
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

void KpkInstallMimeType::kTransactionFinished(KpkTransaction::ExitStatus status)
{
    kDebug() << "Finished.";
    if (status == KpkTransaction::Success) {
        if (m_foundPackages.size()) {
            KpkReviewChanges *frm = new KpkReviewChanges(m_foundPackages);
            frm->setTitle(i18np("Application that can open this type of file",
                                "Applications that can open this type of file", m_foundPackages.size()));
            connect(frm, SIGNAL(finished()), this, SLOT(decreaseRunning()));
            frm->show();
            // return to avoid the decreaseRunning()
            return;
        } else {
            KMessageBox::sorry(0,
                                i18n("No new applications can be found to handle this type of file"),
                                i18n("Failed to find software"));
        }
    }
    decreaseRunning();
}

void KpkInstallMimeType::addPackage(PackageKit::Package *package)
{
    m_foundPackages.append(package);
}

#include "KpkInstallMimeType.moc"
