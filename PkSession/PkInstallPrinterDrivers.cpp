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

#include <QLoggingCategory>

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
    for (const QString &deviceid : resources) {
        QString mfg, mdl;
        const QStringList fields = deviceid.split(QLatin1Char(';'));
        for (const QString &field : fields) {
            const QString keyvalue = field.trimmed();
            if (keyvalue.startsWith(QLatin1String("MFG:"))) {
                mfg = keyvalue.mid(4);
            } else if (keyvalue.startsWith(QLatin1String("MDL:"))) {
                mdl = keyvalue.mid(4);
            }
        }

        if (!mfg.isEmpty() && !mdl.isEmpty()) {
            QString prov;
            QTextStream out(&prov);
            out << mfg.toLower().replace(QLatin1Char(' '), QLatin1Char('_')) << QLatin1Char(';')
                << mdl.toLower().replace(QLatin1Char(' '), QLatin1Char('_')) << QLatin1Char(';');
            search << prov;
        }
    }

    auto transaction = new PkTransaction(this);
    Transaction *t;
    t = Daemon::whatProvides(search,
                             Transaction::FilterNotInstalled | Transaction::FilterArch |  Transaction::FilterNewest);
    transaction->setupTransaction(t);
    setTransaction(Transaction::RoleWhatProvides, transaction);
    connect(transaction, &PkTransaction::finished, this, &PkInstallPrinterDrivers::searchFinished, Qt::UniqueConnection);
    connect(transaction, &PkTransaction::package, this, &PkInstallPrinterDrivers::addPackage);
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
    sendErrorFinished(NoPackagesFound, QLatin1String("failed to find printer driver"));
}

#include "moc_PkInstallPrinterDrivers.cpp"
