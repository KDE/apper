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

#include "PkInstallFontconfigResources.h"

#include <KpkReviewChanges.h>
#include <KpkStrings.h>

#include <KLocale>
#include <KMessageBox>

#include <KDebug>

PkInstallFontconfigResources::PkInstallFontconfigResources(uint xid,
                                                           const QStringList &resources,
                                                           const QString &interaction,
                                                           const QDBusMessage &message,
                                                           QWidget *parent)
 : KpkAbstractTask(xid, interaction, message, parent),
   m_resources(resources)
{
}

PkInstallFontconfigResources::~PkInstallFontconfigResources()
{
}

void PkInstallFontconfigResources::start()
{
    kDebug() << m_resources.first();
    int ret = KMessageBox::Yes;

    if (showConfirmSearch()) {
        foreach (const QString font, m_resources) {
            if (!font.startsWith(":lang=")) {
                sendErrorFinished(InternalError, QString("not recognised prefix: '%1'").arg(font));
                return;
            }
            int size = font.size();
            if (size < 7 || size > 20) {
                sendErrorFinished(InternalError, QString(" lang tag malformed: '%1'").arg(font));
                return;
            }
        }

        QString message = i18np("An additional font is required to view this document correctly. "
                                "Do you want to search for a suitable package now?",
                                "Additional fonts are required to view this document correctly. "
                                "Do you want to search for suitable packages now?",
                                m_resources.size());

        QString title;
        // this will come from DBus interface
        if (parentTitle.isNull()) {
            title = i18np("A program wants to install a font",
                        "A program wants to install fonts",
                        m_resources.size());
        } else {
            title = i18np("%2 wants to install a font",
                        "%2 wants to install fonts",
                        m_resources.size(),
                        parentTitle);
        }
        QString msg = "<h3>" + title + "</h3>" + message;
        KGuiItem searchBt = KStandardGuiItem::yes();
        searchBt.setText(i18nc("Search for packages" ,"Search"));
        searchBt.setIcon(KIcon("edit-find"));

        ret = KMessageBox::questionYesNo(0,
                                        msg,
                                        title,
                                        searchBt);
    }

    if (ret == KMessageBox::Yes) {
        Transaction *t = Client::instance()->whatProvides(Enum::ProvidesFont,
                                                          m_resources,
                                                          Enum::FilterNotInstalled |
                                                          Enum::FilterArch |
                                                          Enum::FilterNewest);
        if (t->error()) {
            QString msg(i18n("Failed to search for provides"));
            KMessageBox::sorry(0,
                               KpkStrings::daemonError(t->error()),
                               msg);
            sendErrorFinished(InternalError, msg);
        } else {
            connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                    this, SLOT(whatProvidesFinished(PackageKit::Transaction::ExitStatus, uint)));
            connect(t, SIGNAL(package(PackageKit::Package *)),
                    this, SLOT(addPackage(PackageKit::Package *)));
            if (showProgress()) {
                KpkTransaction *trans;
                trans = new KpkTransaction(t, KpkTransaction::CloseOnFinish);
                trans->show();
            }
        }
    } else {
        sendErrorFinished(Cancelled, i18n("did not agree to search"));
    }
}

void PkInstallFontconfigResources::whatProvidesFinished(PackageKit::Enum::Exit status, uint runtime)
{
    Q_UNUSED(runtime)
    kDebug() << "Finished.";
    if (status == Enum::ExitSuccess) {
        if (m_foundPackages.size()) {
            KpkReviewChanges *frm = new KpkReviewChanges(m_foundPackages);
            frm->setTitle(i18np("Application that can open this type of file",
                                "Applications that can open this type of file",
                                m_foundPackages.size()));

            if (frm->exec(operationModes()) == 0) {
                sendErrorFinished(Failed, "Transaction did not finish with success");
            } else {
                finishTaskOk();
            }
        } else {
            if (showWarning()) {
                KMessageBox::sorry(0,
                                   i18n("No new fonts can be found for this document"),
                                   i18n("Failed to find font"));
            }
            sendErrorFinished(NoPackagesFound, "failed to find font");
        }
    } else {
        if (showWarning()) {
                KMessageBox::sorry(0,
                                i18n("Failed to search for provides"),
                                i18n("Failed to find font"));
        }
        sendErrorFinished(NoPackagesFound, "failed to search for provides");
    }
}

void PkInstallFontconfigResources::addPackage(PackageKit::Package *package)
{
    m_foundPackages.append(package);
}

#include "PkInstallFontconfigResources.moc"
