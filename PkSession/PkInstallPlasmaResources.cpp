/***************************************************************************
 *   Copyright (C) 2009-2011 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
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

#include <Daemon>
#include <PkStrings.h>

#include <QStandardItemModel>
#include <KLocalizedString>

#include <QLoggingCategory>

PkInstallPlasmaResources::PkInstallPlasmaResources(uint xid,
                                                   const QStringList &resources,
                                                   const QString &interaction,
                                                   const QDBusMessage &message,
                                                   QWidget *parent)
    : SessionTask(xid, interaction, message, parent)
{
    setWindowTitle(i18n("Install Plasma Resources"));

    auto introDialog = new IntroDialog(this);
    auto model = new QStandardItemModel(this);
    introDialog->setModel(model);
    connect(introDialog, &IntroDialog::continueChanged, this, &PkInstallPlasmaResources::enableButtonOk);
    setMainWidget(introDialog);

    // Resources are strings like "dataengine-weather"
    for (const QString &service : resources) {
        QString prettyService = service;
        if (service.startsWith(QLatin1String("dataengine-"))) {
            prettyService = i18n("%1 data engine", service.mid(11));
        } else if (service.startsWith(QLatin1String("scriptengine-"))) {
            prettyService = i18n("%1 script engine", service.mid(13));
        }

        auto item = new QStandardItem(prettyService);
        item->setIcon(QIcon::fromTheme(QLatin1String("application-x-plasma")).pixmap(32, 32));
        item->setFlags(Qt::ItemIsEnabled);
        model->appendRow(item);

        m_resources << service;
    }

    if (m_resources.isEmpty()) {
        introDialog->setDescription(i18n("No supported resources were provided"));
    } else {
        QString description = i18np("The following service is required. "
                                    "Do you want to search for this now?",
                                    "The following services are required. "
                                    "Do you want to search for these now?",
                                    m_resources.size());
        introDialog->setDescription(description);
        enableButtonOk(true);
    }

    QString title = i18np("Plasma requires an additional service for this operation",
                          "Plasma requires additional services for this operation",
                          m_resources.size());

    setTitle(title);
}

PkInstallPlasmaResources::~PkInstallPlasmaResources()
{
}

void PkInstallPlasmaResources::search()
{
    auto transaction = new PkTransaction(this);
    Transaction *t;
    t = Daemon::whatProvides(m_resources,
                             Transaction::FilterNotInstalled | Transaction::FilterArch | Transaction::FilterNewest);
    transaction->setupTransaction(t);
    setTransaction(Transaction::RoleWhatProvides, transaction);
    connect(transaction, &PkTransaction::finished, this, &PkInstallPlasmaResources::searchFinished, Qt::UniqueConnection);
    connect(transaction, &PkTransaction::package, this, &PkInstallPlasmaResources::addPackage);
}

void PkInstallPlasmaResources::notFound()
{
    QString msg = i18n("Failed to search for Plasma service");
    if (showWarning()) {
        setInfo(msg,
                i18n("Could not find service "
                     "in any configured software source"));
    }
    sendErrorFinished(NoPackagesFound, msg);
}

#include "moc_PkInstallPlasmaResources.cpp"
