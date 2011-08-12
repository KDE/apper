/***************************************************************************
 *   Copyright (C) 2009-2011 by Daniel Nicoletti                           *
 *   dantti85-pk@yahoo.com.br                                              *
 *   Copyright (C) 2011 Kevin Kofler <kevin.kofler@chello.at>              *
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

#include "PkInstallPlasmaResources.h"

#include "IntroDialog.h"

#include <KpkStrings.h>

#include <QStandardItemModel>
#include <KLocale>

#include <KDebug>

PkInstallPlasmaResources::PkInstallPlasmaResources(uint xid,
                                                   const QStringList &resources,
                                                   const QString &interaction,
                                                   const QDBusMessage &message,
                                                   QWidget *parent)
 : SessionTask(xid, interaction, message, parent)
{
    m_introDialog = new IntroDialog(this);
    setMainWidget(m_introDialog);
    QStandardItemModel *model = new QStandardItemModel(this);

    // Resources are strings like "dataengine-weather"
    foreach (const QString &service, resources) {
        QString prettyService = service;
        if (service.startsWith("dataengine-")) {
            prettyService = i18n("%1 data engine", service.mid(11));
        } else if (service.startsWith("scriptengine-")) {
            prettyService = i18n("%1 script engine", service.mid(13));
        }

        QStandardItem *item = new QStandardItem(prettyService);
        item->setIcon(KIcon("x-kde-nsplugin-generated").pixmap(32, 32));
        model->appendRow(item);

        m_resources << service;
    }

    QString description = i18np("The following service is required. "
                                "Do you want to search for this now?",
                                "The following services are required. "
                                "Do you want to search for these now?",
                                m_resources.size());

    QString title = i18np("Plasma requires an additional service for this operation",
                          "Plasma requires additional services for this operation",
                          m_resources.size());

    m_introDialog->setDescription(description);
    setTitle(title);
}

PkInstallPlasmaResources::~PkInstallPlasmaResources()
{
}

void PkInstallPlasmaResources::search()
{
    Transaction *t = new Transaction(this);
    connect(t, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
            this, SLOT(whatProvidesFinished(PackageKit::Transaction::Exit)));
    connect(t, SIGNAL(package(PackageKit::Package)),
            this, SLOT(addPackage(PackageKit::Package)));
    PkTransaction *trans = new PkTransaction(t, this);
    setMainWidget(trans);
    t->whatProvides(Transaction::ProvidesPlasmaService,
                    m_resources,
                    Transaction::FilterNotInstalled | Transaction::FilterArch | Transaction::FilterNewest);
    if (t->error()) {
        QString msg(i18n("Failed to search for provides"));
        if (showWarning()) {
            setError(msg,
                     KpkStrings::daemonError(t->error()));
        }
        sendErrorFinished(Failed, msg);
    }
}

void PkInstallPlasmaResources::whatProvidesFinished(PackageKit::Transaction::Exit status)
{
    kDebug() << "Finished.";
    if (status == Transaction::ExitSuccess) {
        if (m_foundPackages.size()) {
            ReviewChanges *frm = new ReviewChanges(m_foundPackages, this);
            setMainWidget(frm);
            setTitle(frm->title());
//             if (frm->exec(operationModes()) == 0) {
//                 sendErrorFinished(Failed, "Transaction did not finish with success");
//             } else {
//                 finishTaskOk();
//             }
        } else {
            QString msg = i18n("Failed to search for Plasma service");
            if (showWarning()) {
                setInfo(msg,
                        i18n("Could not find service "
                             "in any configured software source"));
            }
            sendErrorFinished(NoPackagesFound, msg);
        }
    } else {
        sendErrorFinished(Failed, "what provides transaction failed");
    }
}

void PkInstallPlasmaResources::addPackage(const PackageKit::Package &package)
{
    m_foundPackages.append(package);
}

#include "PkInstallPlasmaResources.moc"
