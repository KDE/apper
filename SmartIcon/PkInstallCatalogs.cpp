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

#include <KpkReviewChanges.h>
#include <KpkStrings.h>

#include <KLocale>
#include <KMessageBox>

#include <KDebug>

PkInstallCatalogs::PkInstallCatalogs(uint xid,
                                     const QStringList &files,
                                     const QString &interaction,
                                     const QDBusMessage &message,
                                     QWidget *parent)
 : KpkAbstractTask(xid, interaction, message, parent),
   m_files(files),
   m_interaction(interaction),
   m_message(message),
   m_maxResolve(100)
{
    kDebug() << xid;
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
}

PkInstallCatalogs::~PkInstallCatalogs()
{
}

void PkInstallCatalogs::start()
{
    int ret = KMessageBox::Yes;
    if (showConfirmSearch()) {
        QString message = i18np("<h3>Do you want to install this catalog?</h3>"
                                "<ul><li>%2</li></ul>",
                                "<h3>Do you want to install these catalogs?</h3>"
                                "<ul><li>%2</li></ul>",
                                m_files.size(),
                                m_files.join("</li><li>"));
        QString title;
        title = i18np("Install catalog",
                      "Install catalogs",
                      m_files.size());
        KGuiItem searchBt = KStandardGuiItem::yes();
        searchBt.setText(i18nc("Parse the catalog search and install it", "Install"));
        searchBt.setIcon(KIcon("edit-find"));
        ret = KMessageBox::questionYesNoWId(parentWId(),
                                            message,
                                            title,
                                            searchBt);
    }

    if (ret == KMessageBox::Yes) {
        QString distroId = Client::instance()->distroId();
        QStringList parts = distroId.split(';');
        if (parts.size() != 3) {
            sendErrorFinished(Failed, "invalid distribution id, please fill a bug against you distribution backend");
            return;
        }
        QString distro = parts.at(0);
        QString version = parts.at(1);
        QString arch = parts.at(2);

        QStringList rxActions;
        Enum::Roles actions = Client::instance()->actions();
        if (actions & Enum::RoleResolve) {
            rxActions << "InstallPackages";
        }

        if (actions & Enum::RoleWhatProvides) {
            rxActions << "InstallProvides";
        }

        if (actions & Enum::RoleSearchFile) {
            rxActions << "InstallFiles";
        }

        if (rxActions.isEmpty()) {
            if (showWarning()) {
                // TODO display a nicer message informing of already installed ones
                KMessageBox::sorryWId(parentWId(),
                                      i18n("Your backend does not support any of the needed methods to install a catalog"),
                                      i18n("Not supported"));
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

        if (showWarning() && filesFailedToOpen.size()) {
            // TODO display a nicer message informing of already installed ones
            KMessageBox::sorryWId(parentWId(),
                                  i18np("Catalog %2 failed to open",
                                        "Catalogs %2 failed to open",
                                        filesFailedToOpen.size(),
                                        filesFailedToOpen.join(",")),
                                  i18n("Failed to open"));
        }

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
                // TODO display a nicer message informing of already installed ones
                KMessageBox::sorryWId(parentWId(),
                                      i18n("No package was found to be installed"),
                                      i18n("No package was found to be installed"));
            }
            sendErrorFinished(NoPackagesFound, "no package found");
        }
    } else {
        sendErrorFinished(Cancelled, "did not agree to install");
    }
}

bool PkInstallCatalogs::installPackages(const QStringList &packages)
{
    int count = 0;
    while (count < packages.size()) {
//         kDebug() << packages.mid(count, m_maxResolve);
        Transaction *t = Client::instance()->resolve(packages.mid(count, m_maxResolve),
                                                     Enum::FilterArch |
                                                     Enum::FilterNewest);
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
    Transaction *t = Client::instance()->whatProvides(Enum::ProvidesAny,
                                                      provides,
                                                      Enum::FilterArch |
                                                      Enum::FilterNewest);
    return runTransaction(t);
}

bool PkInstallCatalogs::installFiles(const QStringList &files)
{
    kDebug() << files;
    Transaction *t = Client::instance()->searchFiles(files,
                                                     Enum::FilterArch |
                                                     Enum::FilterNewest);
    return runTransaction(t);
}

bool PkInstallCatalogs::runTransaction(Transaction *t)
{
    if (t->error()) {
        QString msg(i18n("Failed to start setup transaction"));
        if (showWarning()) {
            KMessageBox::sorryWId(parentWId(),
                                  KpkStrings::daemonError(t->error()),
                                  msg);
        }
        sendErrorFinished(Failed, msg);
        return false;
    } else {
        QEventLoop loop;
        connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                &loop, SLOT(quit()));
        connect(t, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                this, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
        if (showProgress()) {
            kTransaction()->setTransaction(t);
            kTransaction()->show();
        }
        loop.exec();
        return true;
    }
}

void PkInstallCatalogs::addPackage(QSharedPointer<PackageKit::Package> package)
{
    if (package->info() != Enum::InfoInstalled) {
        m_foundPackages.append(package);
    } else {
        m_alreadyInstalled << package->name();
    }
}

#include "PkInstallCatalogs.moc"
