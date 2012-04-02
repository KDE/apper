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

#include <KLocale>
#include <KMessageBox>

#include <KDebug>

#include <QFile>

#include <Daemon>

PkInstallCatalogs::PkInstallCatalogs(uint xid,
                                     const QStringList &files,
                                     const QString &interaction,
                                     const QDBusMessage &message,
                                     QWidget *parent)
 : SessionTask(xid, interaction, message, parent),
   m_interaction(interaction),
   m_message(message),
   m_maxResolve(100)
{
    setWindowTitle(i18n("Install Packages Catalogs"));

    // Find out how many packages PackageKit is able to resolve
    QFile file("/etc/PackageKit/PackageKit.conf");
    QRegExp rx("\\s*MaximumItemsToResolve=(\\d+)", Qt::CaseSensitive);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            if (rx.indexIn(in.readLine()) != -1) {
                m_maxResolve = rx.capturedTexts()[1].toInt();
                break;
            }
        }
    }

    QStringList mimes;
    mimes << "application/x-catalog";
    mimes << "text/plain";
    m_introDialog = new IntroDialog(this);
    m_introDialog->acceptDrops(i18n("You can drop more catalogs in here"));
    m_model = new FilesModel(files, mimes, this);
    connect(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(modelChanged()));
    m_introDialog->setModel(m_model);
    setMainWidget(m_introDialog);

    modelChanged();
}

PkInstallCatalogs::~PkInstallCatalogs()
{
}

void PkInstallCatalogs::modelChanged()
{
    QStringList files = m_model->files();
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
    QString distroId = Daemon::distroId();
    QStringList parts = distroId.split(';');
    if (parts.size() != 3) {
        sendErrorFinished(Failed, "invalid distribution id, please fill a bug against you distribution backend");
        return;
    }
    QString distro = parts.at(0);
    QString version = parts.at(1);
    QString arch = parts.at(2);

    QStringList rxActions;
    Transaction::Roles actions = Daemon::actions();
    if (actions & Transaction::RoleResolve) {
        rxActions << "InstallPackages";
    }

    if (actions & Transaction::RoleWhatProvides) {
        rxActions << "InstallProvides";
    }

    if (actions & Transaction::RoleSearchFile) {
        rxActions << "InstallFiles";
    }

    if (rxActions.isEmpty()) {
        if (showWarning()) {
            // TODO display a nicer message informing of already installed ones
            setInfo(i18n("Not supported"),
                    i18n("Your backend does not support any of the needed "
                         "methods to install a catalog"));
        }
        sendErrorFinished(Failed, "not supported by backend");
        return;
    }

    // matches at the beginning of line installPackages or InstallProvides or installFiles and capture it
    // matches an optional set of parenthesis
    // matches *%1* and or *%2* and or *%3*
    // matches after '=' but ';' at the end
    QString pattern;
    pattern = QString(
                "^(%1)(?:\\((?:.*%2[^;]*(?:;(?:.*%3[^;]*(?:;(?:.*%4[^;]*)?)?)?)?)?\\))?=(.*[^;$])").arg(rxActions.join("|")).arg(distro).arg(version).arg(arch);
    QRegExp rx(pattern, Qt::CaseInsensitive);

    QStringList filesFailedToOpen;
    QStringList files = m_model->files();
    if (!files.isEmpty()) {
        foreach (const QString &file, files) {
            QFile catalog(file);
            if (catalog.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&catalog);
                while (!in.atEnd()) {
                    if (rx.indexIn(in.readLine()) != -1) {
                        if (rx.cap(1).compare("InstallPackages", Qt::CaseInsensitive) == 0) {
                            m_installPackages.append(rx.cap(2).split(';'));
                        } else if (rx.cap(1).compare("InstallProvides", Qt::CaseInsensitive) == 0) {
                            m_installProvides.append(rx.cap(2).split(';'));
                        } else if (rx.cap(1).compare("InstallFiles", Qt::CaseInsensitive) == 0) {
                            m_installFiles.append(rx.cap(2).split(';'));
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
                                 filesFailedToOpen.join(",")),
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
            kDebug() << "m_installPackages" << m_maxResolve << m_installPackages.size() << resolve.size();

            Transaction *t = new Transaction(this);
            PkTransaction *trans = setTransaction(Transaction::RoleResolve, t);
            connect(trans, SIGNAL(finished(PkTransaction::ExitStatus)),
                    this, SLOT(searchFinished(PkTransaction::ExitStatus)), Qt::UniqueConnection);
            connect(t, SIGNAL(package(PackageKit::Package)),
                    this, SLOT(addPackage(PackageKit::Package)));
            t->resolve(resolve, Transaction::FilterArch | Transaction::FilterNewest);
            checkTransaction(t);
        } else if (!m_installProvides.isEmpty()) {
            // Continue resolving Install Provides
            QStringList provides;
            int count = 0;
            while (!m_installProvides.isEmpty() && count < m_maxResolve) {
                // Remove the items from the list so next call we have less
                provides.append(m_installProvides.takeFirst());
                ++count;
            }
            kDebug() << "m_installProvides" <<  m_maxResolve << m_installProvides.size() << provides.size();

            Transaction *t = new Transaction(this);
            PkTransaction *trans = setTransaction(Transaction::RoleWhatProvides, t);
            connect(trans, SIGNAL(finished(PkTransaction::ExitStatus)),
                    this, SLOT(searchFinished(PkTransaction::ExitStatus)), Qt::UniqueConnection);
            connect(t, SIGNAL(package(PackageKit::Package)),
                    this, SLOT(addPackage(PackageKit::Package)));
            t->whatProvides(Transaction::ProvidesAny,
                            provides,
                            Transaction::FilterArch | Transaction::FilterNewest);
            checkTransaction(t);
        } else if (!m_installFiles.isEmpty()) {
            // Continue resolving Install Packages
            QStringList files;
            int count = 0;
            while (!m_installFiles.isEmpty() && count < m_maxResolve) {
                // Remove the items from the list so next call we have less
                files.append(m_installFiles.takeFirst());
                ++count;
            }
            kDebug() << "m_installFiles" << m_maxResolve << m_installFiles.size() << files.size();

            Transaction *t = new Transaction(this);
            PkTransaction *trans = setTransaction(Transaction::RoleSearchFile, t);
            connect(trans, SIGNAL(finished(PkTransaction::ExitStatus)),
                    this, SLOT(searchFinished(PkTransaction::ExitStatus)), Qt::UniqueConnection);
            connect(t, SIGNAL(package(PackageKit::Package)),
                    this, SLOT(addPackage(PackageKit::Package)));
            t->searchFiles(files, Transaction::FilterArch | Transaction::FilterNewest);
            checkTransaction(t);
        } else {
            // we are done resolving
            SessionTask::searchFinished(status);
        }
    } else {
        // we got an error...
        SessionTask::searchFinished(status);
    }
}

void PkInstallCatalogs::checkTransaction(Transaction *transaction)
{
    if (transaction->error()) {
        QString msg(i18n("Failed to start resolve transaction"));
        setError(msg, PkStrings::daemonError(transaction->error()));
        sendErrorFinished(Failed, msg);
    }
}

void PkInstallCatalogs::addPackage(const PackageKit::Package &package)
{
    // When there are updates the package is available...
    if (package.info() != Package::InfoInstalled) {
        SessionTask::addPackage(package);
    } else {
        m_alreadyInstalled << package.name();
    }
}

#include "PkInstallCatalogs.moc"
