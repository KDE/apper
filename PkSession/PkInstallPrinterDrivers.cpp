/**************************************************************************
*   Copyright (C) 2009-2011 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
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

#include <Daemon>

#include <PkStrings.h>

#include <KLocalizedString>

#include <KDebug>

#include <QTextStream>

PkInstallPrinterDrivers::PkInstallPrinterDrivers(uint xid,
                                                 const QStringList &resources,
                                                 const QString &interaction,
                                                 const QDBusMessage &message,
                                                 QWidget *parent) :
    SessionTask(xid, interaction, message, parent),
    m_resources(resources)
{
    setWindowTitle(i18n("Install Printer Drivers"));
    // TODO confirm operation
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

    PkTransaction *transaction = new PkTransaction(this);
    Transaction *t;
    t = Daemon::whatProvides(search,
                             Transaction::FilterNotInstalled | Transaction::FilterArch |  Transaction::FilterNewest);
    transaction->setupTransaction(t);
    setTransaction(Transaction::RoleWhatProvides, transaction);
    connect(transaction, SIGNAL(finished(PkTransaction::ExitStatus)),
            this, SLOT(searchFinished(PkTransaction::ExitStatus)), Qt::UniqueConnection);
    connect(transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
}

PkInstallPrinterDrivers::~PkInstallPrinterDrivers()
{
}

void PkInstallPrinterDrivers::notFound()
{
    if (showWarning()) {
        setInfo(i18n("Failed to search for printer driver"),
                i18n("Could not find printer driver "
                     "in any configured software source"));
    }
    sendErrorFinished(NoPackagesFound, "failed to find printer driver");
}

#include "PkInstallPrinterDrivers.moc"
