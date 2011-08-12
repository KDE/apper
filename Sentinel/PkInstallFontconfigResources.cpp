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

#include "IntroDialog.h"

#include <KpkStrings.h>

#include <KLocale>

#include <KDebug>

PkInstallFontconfigResources::PkInstallFontconfigResources(uint xid,
                                                           const QStringList &resources,
                                                           const QString &interaction,
                                                           const QDBusMessage &message,
                                                           QWidget *parent)
 : SessionTask(xid, interaction, message, parent),
   m_resources(resources)
{
    m_introDialog = new IntroDialog(this);
    setMainWidget(m_introDialog);
    foreach (const QString &font, resources) {
        if (!font.startsWith(QLatin1String(":lang="))) {
            sendErrorFinished(InternalError, QString("not recognised prefix: '%1'").arg(font));
            return;
        }
        int size = font.size();
        if (size < 7 || size > 20) {
            sendErrorFinished(InternalError, QString(" lang tag malformed: '%1'").arg(font));
            return;
        }
    }

    QString description;
    description = i18np("An additional font is required to view this document correctly. "
                        "Do you want to search for a suitable package now?",
                        "Additional fonts are required to view this document correctly. "
                        "Do you want to search for suitable packages now?",
                        resources.size());

    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        title = i18np("A program wants to install a font",
                      "A program wants to install fonts",
                      resources.size());
    } else {
        title = i18np("The application %2 wants to install a font",
                      "The application %2 wants to install fonts",
                      resources.size(),
                      parentTitle);
    }

    m_introDialog->setDescription(description);
    setTitle(title);
}

PkInstallFontconfigResources::~PkInstallFontconfigResources()
{
}

void PkInstallFontconfigResources::search()
{
    Transaction *t = new Transaction(this);
    PkTransaction *trans = new PkTransaction(t, this);
    setMainWidget(trans);
    connect(t, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
            this, SLOT(whatProvidesFinished(PackageKit::Transaction::Exit)));
    connect(t, SIGNAL(package(PackageKit::Package)),
            this, SLOT(addPackage(PackageKit::Package)));
    t->whatProvides(Transaction::ProvidesFont,
                    m_resources,
                    Transaction::FilterNotInstalled | Transaction::FilterArch | Transaction::FilterNewest);
    if (t->error()) {
        QString msg(i18n("Failed to search for provides"));
        if (showWarning()) {
            setError(msg, KpkStrings::daemonError(t->error()));
        }
        sendErrorFinished(Failed, msg);
    }
}

void PkInstallFontconfigResources::whatProvidesFinished(PackageKit::Transaction::Exit status)
{
    kDebug() << "Finished.";
    if (status == Transaction::ExitSuccess) {
        if (m_foundPackages.size()) {
            ReviewChanges *frm = new ReviewChanges(m_foundPackages, this);
            setTitle(i18np("Application that can open this type of file",
                           "Applications that can open this type of file",
                           m_foundPackages.size()));
            setMainWidget(frm);
//            if (frm->exec(operationModes()) == 0) {
//                sendErrorFinished(Failed, "Transaction did not finish with success");
//            } else {
//                finishTaskOk();
//            }
        } else {
            QString msg = i18n("Failed to find font");
            if (showWarning()) {
                setInfo(msg, i18n("No new fonts could be found for this document"));
            }
            sendErrorFinished(NoPackagesFound, msg);
        }
    } else {
        QString msg = i18n("Failed to find font");
        if (showWarning()) {
            setError(msg, i18n("Failed to search for provides"));
        }
        sendErrorFinished(NoPackagesFound, "failed to search for provides");
    }
}

void PkInstallFontconfigResources::addPackage(const PackageKit::Package &package)
{
    m_foundPackages.append(package);
}

#include "PkInstallFontconfigResources.moc"
