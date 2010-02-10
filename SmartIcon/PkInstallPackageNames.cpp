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

#include "PkInstallPackageNames.h"

#include <KpkReviewChanges.h>
#include <KpkStrings.h>

#include <KLocale>
#include <KMessageBox>

#include <KDebug>

PkInstallPackageNames::PkInstallPackageNames(uint xid,
                                             const QStringList &packages,
                                             const QString &interaction,
                                             const QDBusMessage &message,
                                             QWidget *parent)
 : KpkAbstractTask(xid, interaction, message, parent),
   m_packages(packages),
   m_message(message)
{
}

PkInstallPackageNames::~PkInstallPackageNames()
{
}

void PkInstallPackageNames::start()
{
    kDebug() << m_packages.first();
    int ret = KMessageBox::Yes;
    if (showConfirmSearch()) {
        QString message = i18np("An additional package is required: <ul><li>%2</li></ul>"
                                "Do you want to search for and install this package now?",
                                "Additional packages are required: <ul><li>%2</li></ul>"
                                "Do you want to search for and install these packages now?",
                                m_packages.size(),
                                m_packages.join("</li><li>"));
        QString title;
        // this will come from DBus interface
        if (parentTitle.isNull()) {
            title = i18np("A program wants to install a package",
                          "A program wants to install packages",
                          m_packages.size());
        } else {
            title = i18np("%2 wants to install a package",
                          "%2 wants to install packages",
                          m_packages.size(),
                          parentTitle);
        }
        QString msg = "<h3>" + title + "</h3>" + message;
        KGuiItem searchBt = KStandardGuiItem::yes();
        searchBt.setText(i18nc("Search for a package and install it", "Install"));
        searchBt.setIcon(KIcon("edit-find"));
        ret = KMessageBox::questionYesNo(this,
                                         msg,
                                         title,
                                         searchBt);
    }

    if (ret == KMessageBox::Yes) {
        Transaction *t = Client::instance()->resolve(m_packages,
                                                     Enum::FilterArch |
                                                     Enum::FilterNewest);
        if (t->error()) {
            QString msg(i18n("Failed to start resolve transaction"));
            if (showWarning()) {
                KMessageBox::sorry(this,
                                   KpkStrings::daemonError(t->error()),
                                   msg);
            }
            sendErrorFinished(Failed, msg);
        } else {
            connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                    this, SLOT(resolveFinished(PackageKit::Transaction::ExitStatus, uint)));
            connect(t, SIGNAL(package(PackageKit::Package *)),
                    this, SLOT(addPackage(PackageKit::Package *)));
            if (showProgress()) {
                KpkTransaction *trans = new KpkTransaction(t, KpkTransaction::CloseOnFinish);
                trans->show();
                setParentWindow(trans);
            }
        }
    } else {
        sendErrorFinished(Cancelled, "did not agree to search");
    }
}

void PkInstallPackageNames::resolveFinished(PackageKit::Enum::Exit status,
                                            uint runtime)
{
    Q_UNUSED(runtime)
    kDebug() << "Finished.";
    if (status == Enum::ExitSuccess) {
        if (m_alreadyInstalled.size()) {
            if (showWarning()) {
                KMessageBox::sorry(this,
                                   i18np("The package %2 is already installed",
                                         "The packages %2 are already installed",
                                         m_alreadyInstalled.size(),
                                         m_alreadyInstalled.join(",")),
                                   i18n("Failed to install packages"));
            }
            sendErrorFinished(Failed, "package already found");
        } else if (m_foundPackages.size()) {
            KpkReviewChanges *frm = new KpkReviewChanges(m_foundPackages, this);
            frm->setTitle(i18np("The following package will be installed",
                                "The following packages will be installed",
                                m_foundPackages.size()));
            if (frm->exec(operationModes()) == 0) {
                sendErrorFinished(Failed, i18n("Transaction did not finish with success"));
            } else {
                finishTaskOk();
            }
        } else {
            if (showWarning()) {
                KMessageBox::sorry(this,
                                   i18np("The package could not be found in any software source",
                                         "The packages could not be found in any software source",
                                         m_packages.size()),
                                   i18n("Could not find %1", m_packages.join(", ")));
            }
            sendErrorFinished(NoPackagesFound, "no package found");
        }
    } else {
        sendErrorFinished(Failed, "failed to resolve package name");
    }
}

void PkInstallPackageNames::addPackage(PackageKit::Package *package)
{
    if (package->info() != Enum::InfoInstalled) {
        m_foundPackages.append(package);
    } else {
        m_alreadyInstalled << package->name();
    }
}

#include "PkInstallPackageNames.moc"
