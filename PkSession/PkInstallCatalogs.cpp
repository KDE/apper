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

#include "PkInstallCatalogs.h"
#include "IntroDialog.h"
#include "FilesModel.h"

#include <PkStrings.h>

#include <KLocalizedString>
#include <KMessageBox>

#include <QLoggingCategory>

#include <QFile>

#include <Daemon>

Q_DECLARE_LOGGING_CATEGORY(APPER_SESSION)

PkInstallCatalogs::PkInstallCatalogs(uint xid,
                                     const QStringList &files,
                                     const QString &interaction,
                                     const QDBusMessage &message,
                                     QWidget *parent)
 : SessionTask(xid, interaction, message, parent),
   m_interaction(interaction),
   m_message(message)
{
    setWindowTitle(i18n("Install Packages Catalogs"));

    // Find out how many packages PackageKit is able to resolve
    QFile file(QLatin1String("/etc/PackageKit/PackageKit.conf"));
    QRegExp rx(QLatin1String("\\s*MaximumItemsToResolve=(\\d+)"), Qt::CaseSensitive);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            if (rx.indexIn(in.readLine()) != -1) {
                m_maxResolve = rx.capturedTexts()[1].toInt();
                break;
            }
        }
    }

    const QStringList mimes{
        QLatin1String("application/x-catalog"), QLatin1String("text/plain")
    };
    m_introDialog = new IntroDialog(this);
    m_introDialog->acceptDrops(i18n("You can drop more catalogs in here"));
    m_model = new FilesModel(files, mimes, this);
    connect(m_model, &FilesModel::rowsInserted, this, &PkInstallCatalogs::modelChanged);
    m_introDialog->setModel(m_model);
    setMainWidget(m_introDialog);

    modelChanged();
}

PkInstallCatalogs::~PkInstallCatalogs()
{
}

void PkInstallCatalogs::modelChanged()
{
    const QStringList files = m_model->files();
    enableButtonOk(!files.isEmpty());
    QString description;
    if (files.isEmpty()) {
        description = i18n("No supported catalog was found");
    } else {
        description = i18np("Do you want to install this catalog?",
                            "Do you want to install these catalogs?",
                            files.size());
    }
    m_introDialog->setDescription(description);

    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        title = i18np("An application wants to install a catalog",
                      "An application wants to install catalogs",
                        files.size());
    } else {
        title = i18np("The application <i>%2</i> wants to install a catalog",
                      "The application <i>%2</i> wants to install catalogs",
                        files.size(),
                        parentTitle);
    }
    setTitle(title);
}

void PkInstallCatalogs::search()
{
    const QString distroId = Daemon::global()->distroID();
    const QStringList parts = distroId.split(QLatin1Char(';'));
    if (parts.size() != 3) {
        sendErrorFinished(Failed, QLatin1String("invalid distribution id, please fill a bug against you distribution backend"));
        return;
    }
    const QString distro = parts.at(0);
    const QString version = parts.at(1);
    const QString arch = parts.at(2);

    QStringList rxActions;
    Transaction::Roles roles = Daemon::global()->roles();
    if (roles & Transaction::RoleResolve) {
        rxActions << QLatin1String("InstallPackages");
    }

    if (roles & Transaction::RoleWhatProvides) {
        rxActions << QLatin1String("InstallProvides");
    }

    if (roles & Transaction::RoleSearchFile) {
        rxActions << QLatin1String("InstallFiles");
    }

    if (rxActions.isEmpty()) {
        if (showWarning()) {
            // TODO display a nicer message informing of already installed ones
            setInfo(i18n("Not supported"),
                    i18n("Your backend does not support any of the needed "
                         "methods to install a catalog"));
        }
        sendErrorFinished(Failed, QLatin1String("not supported by backend"));
        return;
    }

    // matches at the beginning of line installPackages or InstallProvides or installFiles and capture it
    // matches an optional set of parenthesis
    // matches *%1* and or *%2* and or *%3*
    // matches after '=' but ';' at the end
    QString pattern;
    pattern = QString(
                QLatin1String("^(%1)(?:\\((?:.*%2[^;]*(?:;(?:.*%3[^;]*(?:;(?:.*%4[^;]*)?)?)?)?)?\\))?=(.*[^;$])"))
            .arg(rxActions.join(QLatin1Char('|')), distro, version, arch);
    QRegExp rx(pattern, Qt::CaseInsensitive);

    QStringList filesFailedToOpen;
    const QStringList files = m_model->files();
    if (!files.isEmpty()) {
        for (const QString &file : files) {
            QFile catalog(file);
            if (catalog.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&catalog);
                while (!in.atEnd()) {
                    if (rx.indexIn(in.readLine()) != -1) {
                        if (rx.cap(1).compare(QLatin1String("InstallPackages"), Qt::CaseInsensitive) == 0) {
                            m_installPackages.append(rx.cap(2).split(QLatin1Char(';')));
                        } else if (rx.cap(1).compare(QLatin1String("InstallProvides"), Qt::CaseInsensitive) == 0) {
                            m_installProvides.append(rx.cap(2).split(QLatin1Char(';')));
                        } else if (rx.cap(1).compare(QLatin1String("InstallFiles"), Qt::CaseInsensitive) == 0) {
                            m_installFiles.append(rx.cap(2).split(QLatin1Char(';')));
                        }
                    }
                }
            } else {
                filesFailedToOpen << file;
            }
        }
    } else {
        setInfo(i18n("Catalog not found"),
                i18n("Could not find a catalog to install"));
        return;
    }

    if (showWarning() && filesFailedToOpen.size()) {
        // TODO display a nicer message informing of already installed ones
        KMessageBox::sorry(this,
                           i18np("Catalog %2 failed to open",
                                 "Catalogs %2 failed to open",
                                 filesFailedToOpen.size(),
                                 filesFailedToOpen.join(QLatin1Char(','))),
                           i18n("Failed to open"));
    }

    if (m_installPackages.isEmpty() &&
        m_installProvides.isEmpty() &&
        m_installFiles.isEmpty()) {
        setInfo(i18n("Catalog is Empty"),
                i18n("Could not find any package to install in this catalog"));
    } else {
        // Start resolving
        searchFinished(PkTransaction::Success);
    }
}

void PkInstallCatalogs::searchFinished(PkTransaction::ExitStatus status)
{
    if (status == PkTransaction::Success) {
        if (!m_installPackages.isEmpty()) {
            // Continue resolving Install Packages
            QStringList resolve;
            int count = 0;
            while (!m_installPackages.isEmpty() && count < m_maxResolve) {
                // Remove the items from the list so next call we have less
                resolve.append(m_installPackages.takeFirst());
                ++count;
            }
            qCDebug(APPER_SESSION) << "m_installPackages" << m_maxResolve << m_installPackages.size() << resolve.size();

            auto transaction = new PkTransaction(this);
            Transaction *t;
            t = Daemon::resolve(resolve,
                                Transaction::FilterNotInstalled | Transaction::FilterArch | Transaction::FilterNewest);
            transaction->setupTransaction(t);
            setTransaction(Transaction::RoleResolve, transaction);
            connect(transaction, &PkTransaction::finished, this, &PkInstallCatalogs::searchFinished, Qt::UniqueConnection);
            connect(transaction, &PkTransaction::package, this, &PkInstallCatalogs::addPackage);
        } else if (!m_installProvides.isEmpty()) {
            // Continue resolving Install Provides
            QStringList provides;
            int count = 0;
            while (!m_installProvides.isEmpty() && count < m_maxResolve) {
                // Remove the items from the list so next call we have less
                provides.append(m_installProvides.takeFirst());
                ++count;
            }
            qCDebug(APPER_SESSION) << "m_installProvides" <<  m_maxResolve << m_installProvides.size() << provides.size();

            auto transaction = new PkTransaction(this);
            Transaction *t;
            t = Daemon::whatProvides(provides,
                                     Transaction::FilterNotInstalled | Transaction::FilterArch | Transaction::FilterNewest);
            transaction->setupTransaction(t);
            setTransaction(Transaction::RoleWhatProvides, transaction);
            connect(transaction, &PkTransaction::finished, this, &PkInstallCatalogs::searchFinished, Qt::UniqueConnection);
            connect(transaction, &PkTransaction::package, this, &PkInstallCatalogs::addPackage);
        } else if (!m_installFiles.isEmpty()) {
            // Continue resolving Install Packages
            QStringList files;
            int count = 0;
            while (!m_installFiles.isEmpty() && count < m_maxResolve) {
                // Remove the items from the list so next call we have less
                files.append(m_installFiles.takeFirst());
                ++count;
            }
            qCDebug(APPER_SESSION) << "m_installFiles" << m_maxResolve << m_installFiles.size() << files.size();

            auto transaction = new PkTransaction(this);
            Transaction *t;
            t = Daemon::searchFiles(files,
                                    Transaction::FilterNotInstalled | Transaction::FilterArch | Transaction::FilterNewest);
            transaction->setupTransaction(t);
            setTransaction(Transaction::RoleSearchFile, transaction);
            connect(transaction, &PkTransaction::finished, this, &PkInstallCatalogs::searchFinished, Qt::UniqueConnection);
            connect(transaction, &PkTransaction::package, this, &PkInstallCatalogs::addPackage);
        } else {
            // we are done resolving
            SessionTask::searchFinished(status);
        }
    } else {
        // we got an error...
        SessionTask::searchFinished(status);
    }
}

#include "moc_PkInstallCatalogs.cpp"
