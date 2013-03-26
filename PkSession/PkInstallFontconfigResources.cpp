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

#include "PkInstallFontconfigResources.h"

#include "IntroDialog.h"

#include <PkStrings.h>

#include <QStandardItemModel>
#include <QXmlQuery>
#include <QFile>

#include <KLocale>
#include <KDebug>

PkInstallFontconfigResources::PkInstallFontconfigResources(uint xid,
                                                           const QStringList &resources,
                                                           const QString &interaction,
                                                           const QDBusMessage &message,
                                                           QWidget *parent)
 : SessionTask(xid, interaction, message, parent)
{
    setWindowTitle(i18n("Installs new Fonts"));

    IntroDialog *introDialog = new IntroDialog(this);
    QStandardItemModel *model = new QStandardItemModel(this);
    introDialog->setModel(model);
    connect(introDialog, SIGNAL(continueChanged(bool)),
            this, SLOT(enableButtonOk(bool)));
    setMainWidget(introDialog);

    // This will only validate fields
    QStringList errors;
    QStringList iso639;
    foreach (const QString &font, resources) {
        // TODO never return in here
        // TODO add name field from /usr/share/xml/iso-codes/iso_639.xml into model
        if (!font.startsWith(QLatin1String(":lang="))) {
            errors << QString("not recognised prefix: '%1'").arg(font);
            kWarning() << QString("not recognised prefix: '%1'").arg(font);
            continue;
        }
        int size = font.size();
        if (size < 7 || size > 20) {
            errors << QString("lang tag malformed: '%1'").arg(font);
            kWarning() << QString("lang tag malformed: '%1'").arg(font);
            continue;
        }

        m_resources << font;
        iso639 << font.mid(6);
    }

    if (m_resources.isEmpty()) {
        setError(i18n("Could interpret request"), i18n("Please verify if the request was valid"));
        sendErrorFinished(InternalError, errors.join("\n"));
        return;
    }
    enableButtonOk(true);

    // Search for the iso 639 names to present it nicely to the user
    QStringList niceNames;
    QFile file("/usr/share/xml/iso-codes/iso_639.xml");
    file.open(QFile::ReadOnly);
    QXmlQuery query;
    query.bindVariable("path", &file);
    foreach (const QString &font, iso639) {
        QString queryTxt;
        queryTxt = QString("declare variable $path external;"
                           "doc($path)/iso_639_entries/"
                           "iso_639_entry[@iso_639_2B_code=\"%1\" or "
                                         "@iso_639_2T_code=\"%1\" or "
                                         "@iso_639_1_code=\"%1\"]/string(@name)").arg(font);
        query.setQuery(queryTxt);
        QStringList result;
        query.evaluateTo(&result);
        niceNames.append(result);
    }

//    kDebug() << "result" << niceNames << iso639;
    foreach (const QString &name, niceNames) {
        QStandardItem *item = new QStandardItem(name);
        item->setIcon(KIcon("fonts-package").pixmap(32, 32));
        item->setFlags(Qt::ItemIsEnabled);
        model->appendRow(item);
    }

    QString description;
    description = i18np("An additional font is required to view this document correctly.\n"
                        "Do you want to search for a suitable package now?",
                        "Additional fonts are required to view this document correctly.\n"
                        "Do you want to search for suitable packages now?",
                        m_resources.size());
    introDialog->setDescription(description);

    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        title = i18np("A program wants to install a font",
                      "A program wants to install fonts",
                      m_resources.size());
    } else {
        title = i18np("The application %2 wants to install a font",
                      "The application %2 wants to install fonts",
                      m_resources.size(),
                      parentTitle);
    }
    setTitle(title);
}

PkInstallFontconfigResources::~PkInstallFontconfigResources()
{
}

void PkInstallFontconfigResources::search()
{
    PkTransaction *transaction = new PkTransaction(this);
    setTransaction(Transaction::RoleWhatProvides, transaction);
    connect(transaction, SIGNAL(finished(PkTransaction::ExitStatus)),
            this, SLOT(searchFinished(PkTransaction::ExitStatus)), Qt::UniqueConnection);
    connect(transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
    transaction->whatProvides(Transaction::ProvidesFont,
                              m_resources,
                              Transaction::FilterNotInstalled | Transaction::FilterArch | Transaction::FilterNewest);
    if (transaction->internalError()) {
        QString msg(i18n("Failed to search for provides"));
        if (showWarning()) {
            setError(msg, PkStrings::daemonError(transaction->internalError()));
        }
        sendErrorFinished(Failed, msg);
    }
}

void PkInstallFontconfigResources::notFound()
{
    QString msg = i18n("Failed to find font");
    if (showWarning()) {
        setInfo(msg, i18n("No new fonts could be found for this document"));
    }
    sendErrorFinished(NoPackagesFound, msg);
}

void PkInstallFontconfigResources::searchFailed()
{
    QString msg = i18n("Failed to find font");
    if (showWarning()) {
        setError(msg, i18n("Failed to search for provides"));
    }
    sendErrorFinished(Failed, "failed to search for provides");
}

#include "PkInstallFontconfigResources.moc"
