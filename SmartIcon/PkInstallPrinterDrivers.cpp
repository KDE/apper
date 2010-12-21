/**************************************************************************
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

#include "PkInstallPrinterDrivers.h"

#include <KpkReviewChanges.h>
#include <KpkStrings.h>
#include <KpkTransaction.h>

#include <KLocale>
#include <KMessageBox>

#include <KDebug>

#include <QTextStream>

PkInstallPrinterDrivers::PkInstallPrinterDrivers(uint xid,
                                                 const QStringList &resources,
                                                 const QString &interaction,
                                                 const QDBusMessage &message,
                                                 QWidget *parent) :
    KpkAbstractTask(xid, interaction, message, parent),
    m_resources(resources)
{
}

PkInstallPrinterDrivers::~PkInstallPrinterDrivers()
{
}

void PkInstallPrinterDrivers::start()
{
    QStringList search;
    foreach (const QString &deviceid, m_resources) {
        QString mfg, mdl;
        QStringList fields = deviceid.split(';');
        foreach (const QString &field, fields) {
            QString keyvalue = field.trimmed();
            if (keyvalue.startsWith(QLatin1String("MFG:"))) {
                mfg = keyvalue.mid(4);
            } else if (keyvalue.startsWith(QLatin1String("MDL:"))) {
                mdl = keyvalue.mid(4);
            }
        }

        if (!mfg.isEmpty() && !mdl.isEmpty()) {
            QString prov;
            QTextStream out(&prov);
            out << mfg.toLower().replace(' ', '_') << ';'
                << mdl.toLower().replace(' ', '_') << ';';
            search << prov;
        }
    }

    Transaction *t = new Transaction(QString());
    t->whatProvides(Enum::ProvidesPostscriptDriver,
                    search,
                    Enum::FilterNotInstalled | Enum::FilterArch |  Enum::FilterNewest);
    connect(t, SIGNAL(package(PackageKit::QSharedPointer<PackageKit::Package>)),
               this, SLOT(addPackage(PackageKit::QSharedPointer<PackageKit::Package>)));
    if (t->error()) {
        QString msg(i18n("Failed to search for provides"));
        KMessageBox::sorryWId(parentWId(),
                              KpkStrings::daemonError(t->error()),
                              msg);
        sendErrorFinished(InternalError, msg);
    } else {
        connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                this, SLOT(whatProvidesFinished(PackageKit::Enum::Exit, uint)));

        if (showProgress()) {
            kTransaction()->setTransaction(t);
            kTransaction()->show();
        }
    }
}

void PkInstallPrinterDrivers::whatProvidesFinished(PackageKit::Enum::Exit status, uint runtime)
{
    Q_UNUSED(runtime)
    kDebug() << "Finished.";
    if (status == Enum::ExitSuccess) {
        if (m_foundPackages.size()) {
            kTransaction()->hide();
            KpkReviewChanges *frm = new KpkReviewChanges(m_foundPackages, this, parentWId());
            if (frm->exec(operationModes()) == 0) {
                sendErrorFinished(Failed, i18n("Transaction did not finish with success"));
            } else {
                finishTaskOk();
            }
        } else {
            if (showWarning()) {
                KMessageBox::sorryWId(parentWId(),
                                      i18n("Could not find printer driver "
                                           "in any configured software source"),
                                      i18n("Failed to search for printer driver"));
            }
            sendErrorFinished(NoPackagesFound, "failed to find printer driver");
        }
    } else {
        sendErrorFinished(Failed, "what provides failed");
    }
}

void PkInstallPrinterDrivers::addPackage(QSharedPointer<PackageKit::Package> package)
{
    m_foundPackages.append(package);
}

#include "PkInstallPrinterDrivers.moc"
