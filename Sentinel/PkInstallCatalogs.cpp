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

#include "PkInstallCatalogs.h"
#include "IntroDialog.h"
#include "FilesModel.h"

#include <KpkStrings.h>

#include <KLocale>
#include <KMessageBox>

#include <KDebug>

#include <QEventLoop>
#include <QFile>

#include <Daemon>

PkInstallCatalogs::PkInstallCatalogs(uint xid,
                                     const QStringList &files,
                                     const QString &interaction,
                                     const QDBusMessage &message,
                                     QWidget *parent)
 : SessionTask(xid, interaction, message, parent),
   m_files(files),
   m_interaction(interaction),
   m_message(message),
   m_maxResolve(100)
{
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

    m_introDialog = new IntroDialog(this);
    m_model = new FilesModel(files, QStringList(), this);
    connect(m_model, SIGNAL(rowsInserted(QModelIndex, int, int)),
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

    QString message = i18np("Do you want to install this catalog?",
                            "Do you want to install these catalogs?",
                            m_files.size());
    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        title = i18np("An application wants to install a catalog",
                      "An application wants to install catalogs",
                        m_files.size());
    } else {
        title = i18np("The application <i>%2</i> wants to install a catalog",
                      "The application <i>%2</i> wants to install catalogs",
                        m_files.size(),
                        parentTitle);
    }

    m_introDialog->setDescription(message);
    setTitle(title);

//    if (ret == KMessageBox::Yes) {
//
//    } else {
//        sendErrorFinished(Cancelled, "did not agree to install");
//    }
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
    bool failed = false;

    if (!m_files.isEmpty()) {
        m_trans = new PkTransaction(0, this);
        setMainWidget(m_trans);

        foreach (const QString &file, m_files) {
            QFile catalog(file);
            if (catalog.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QTextStream in(&catalog);
                while (!in.atEnd()) {
                    if (rx.indexIn(in.readLine()) != -1) {
                        if (rx.cap(1).compare("InstallPackages", Qt::CaseInsensitive) == 0) {
                            failed = !installPackages(rx.cap(2).split(';'));
                        } else if (rx.cap(1).compare("InstallProvides", Qt::CaseInsensitive) == 0) {
                            failed = !installProvides(rx.cap(2).split(';'));
                        } else if (rx.cap(1).compare("InstallFiles", Qt::CaseInsensitive) == 0) {
                            failed = !installFiles(rx.cap(2).split(';'));
                        }

                        if (failed) {
                            return;
                        }
                    }
                }
            } else {
                filesFailedToOpen << file;
            }
        }
    } else if (showWarning() && filesFailedToOpen.size()) {
        // TODO display a nicer message informing of already installed ones
        KMessageBox::sorry(this,
                           i18np("Catalog %2 failed to open",
                                 "Catalogs %2 failed to open",
                                 filesFailedToOpen.size(),
                                 filesFailedToOpen.join(",")),
                           i18n("Failed to open"));
    }

    if (m_foundPackages.size()) {
        ReviewChanges *frm = new ReviewChanges(m_foundPackages, this);
        setTitle(frm->title());
        setMainWidget(frm);/*
        if (frm->exec(operationModes()) == 0) {
            sendErrorFinished(Failed, i18n("Transaction did not finish with success"));
        } else {
            finishTaskOk();
        }*/
    } else {
        if (showWarning()) {
            // TODO display a nicer message informing of already installed ones
            setInfo(i18n("Catalog search complete"),
                    i18n("No package was found to be installed"));
        }
        sendErrorFinished(NoPackagesFound, "no package found");
    }
}

bool PkInstallCatalogs::installPackages(const QStringList &packages)
{
    int count = 0;
    while (count < packages.size()) {
//         kDebug() << packages.mid(count, m_maxResolve);
        Transaction *t = new Transaction(this);
        t->resolve(packages.mid(count, m_maxResolve),
                   Transaction::FilterArch | Transaction::FilterNewest);
        count += m_maxResolve;
        if (!runTransaction(t)) {
            return false;
        }
    }
    return true;
}

bool PkInstallCatalogs::installProvides(const QStringList &provides)
{
    kDebug() << provides;
    Transaction *t = new Transaction(this);
    t->whatProvides(Transaction::ProvidesAny,
                    provides,
                    Transaction::FilterArch | Transaction::FilterNewest);
    return runTransaction(t);
}

bool PkInstallCatalogs::installFiles(const QStringList &files)
{
    kDebug() << files;
    Transaction *t = new Transaction(this);
    t->searchFiles(files,
                   Transaction::FilterArch | Transaction::FilterNewest);
    return runTransaction(t);
}

bool PkInstallCatalogs::runTransaction(Transaction *t)
{
    if (t->error()) {
        QString msg(i18n("Failed to start setup transaction"));
        if (showWarning()) {
            setError(msg, KpkStrings::daemonError(t->error()));
        }
        sendErrorFinished(Failed, msg);
        return false;
    } else {
        QEventLoop loop;
        connect(t, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
                &loop, SLOT(quit()));
        connect(t, SIGNAL(package(PackageKit::Package)),
                this, SLOT(addPackage(PackageKit::Package)));
        if (showProgress()) {
            m_trans->setTransaction(t);
        }
        loop.exec();
        return true;
    }
}

void PkInstallCatalogs::addPackage(const PackageKit::Package &package)
{
    if (package.info() != Package::InfoInstalled) {
        m_foundPackages.append(package);
    } else {
        m_alreadyInstalled << package.name();
    }
}

#include "PkInstallCatalogs.moc"
