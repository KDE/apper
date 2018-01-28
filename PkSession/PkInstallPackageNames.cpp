/***************************************************************************
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

#include "PkInstallPackageNames.h"

#include <PkStrings.h>

#include <KLocalizedString>
#include <QStandardItemModel>

#include <QLoggingCategory>

#include <Daemon>

#include "IntroDialog.h"

PkInstallPackageNames::PkInstallPackageNames(uint xid,
                                             const QStringList &packages,
                                             const QString &interaction,
                                             const QDBusMessage &message,
                                             QWidget *parent) :
    SessionTask(xid, interaction, message, parent),
    m_packages(packages),
    m_message(message)
{
    setWindowTitle(i18n("Install Packages by Name"));

    auto introDialog = new IntroDialog(this);
    auto model = new QStandardItemModel(this);
    introDialog->setModel(model);
    connect(introDialog, &IntroDialog::continueChanged, this, &PkInstallPackageNames::enableButtonOk);
    setMainWidget(introDialog);

    for (const QString &package : packages) {
        QStandardItem *item = new QStandardItem(package);
        item->setIcon(QIcon::fromTheme(QLatin1String("package-x-generic")).pixmap(32, 32));
        item->setFlags(Qt::ItemIsEnabled);
        model->appendRow(item);
    }

    if (m_packages.isEmpty()) {
        introDialog->setDescription(i18n("No package names were provided"));
    } else {
        QString description;
        description = i18np("Do you want to search for and install this package now?",
                            "Do you want to search for and install these packages now?",
                            m_packages.size());
        introDialog->setDescription(description);
        enableButtonOk(true);
    }

    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        title = i18np("An application wants to install a package",
                      "An application wants to install packages",
                        m_packages.size());
    } else {
        title = i18np("The application <i>%2</i> wants to install a package",
                      "The application <i>%2</i> wants to install packages",
                        m_packages.size(),
                        parentTitle);
    }
    setTitle(title);
}

PkInstallPackageNames::~PkInstallPackageNames()
{
}

void PkInstallPackageNames::search()
{
    auto transaction = new PkTransaction(this);
    transaction->setupTransaction(Daemon::resolve(m_packages, Transaction::FilterArch | Transaction::FilterNewest));
    setTransaction(Transaction::RoleResolve, transaction);
    connect(transaction, &PkTransaction::finished, this, &PkInstallPackageNames::searchFinished, Qt::UniqueConnection);
    connect(transaction, &PkTransaction::package, this, &PkInstallPackageNames::addPackage);
}

void PkInstallPackageNames::notFound()
{
    if (m_alreadyInstalled.size()) {
        if (showWarning()) {
            setInfo(i18n("Failed to install packages"),
                    i18np("The package %2 is already installed",
                          "The packages %2 are already installed",
                          m_alreadyInstalled.size(),
                          m_alreadyInstalled.join(QLatin1Char(','))));
        }
        sendErrorFinished(Failed, QLatin1String("package already found"));
    } else {
        if (showWarning()) {
            setInfo(i18n("Could not find %1", m_packages.join(QLatin1String(", "))),
                    i18np("The package could not be found in any software source",
                          "The packages could not be found in any software source",
                          m_packages.size()));
        }
        sendErrorFinished(NoPackagesFound, QLatin1String("no package found"));
    }
}

void PkInstallPackageNames::searchFailed()
{
    sendErrorFinished(Failed, QLatin1String("failed to resolve package name"));
}

void PkInstallPackageNames::addPackage(Transaction::Info info, const QString &packageID, const QString &summary)
{
    if (info != Transaction::InfoInstalled) {
        SessionTask::addPackage(info, packageID, summary);
    } else {
        m_alreadyInstalled << Transaction::packageName(packageID);
    }
}

#include "moc_PkInstallPackageNames.cpp"
