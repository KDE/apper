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
#include <Daemon>
#include <PkStrings.h>

#include <QStandardItemModel>
#include <QXmlQuery>
#include <QFile>

#include <KLocalizedString>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(APPER_SESSION)

PkInstallFontconfigResources::PkInstallFontconfigResources(uint xid,
                                                           const QStringList &resources,
                                                           const QString &interaction,
                                                           const QDBusMessage &message,
                                                           QWidget *parent)
 : SessionTask(xid, interaction, message, parent)
{
    setWindowTitle(i18n("Installs new Fonts"));

    auto introDialog = new IntroDialog(this);
    auto model = new QStandardItemModel(this);
    introDialog->setModel(model);
    connect(introDialog, &IntroDialog::continueChanged, this, &PkInstallFontconfigResources::enableButtonOk);
    setMainWidget(introDialog);

    // This will only validate fields
    QStringList errors;
    QStringList iso639;
    for (const QString &font : resources) {
        // TODO never return in here
        // TODO add name field from /usr/share/xml/iso-codes/iso_639.xml into model
        if (!font.startsWith(QLatin1String(":lang="))) {
            errors << QString(QLatin1String("not recognised prefix: '%1'")).arg(font);
            qCWarning(APPER_SESSION) << QString(QLatin1String("not recognised prefix: '%1'")).arg(font);
            continue;
        }
        int size = font.size();
        if (size < 7 || size > 20) {
            errors << QString(QLatin1String("lang tag malformed: '%1'")).arg(font);
            qCWarning(APPER_SESSION) << QString(QLatin1String("lang tag malformed: '%1'")).arg(font);
            continue;
        }

        m_resources << font;
        iso639 << font.mid(6);
    }

    if (m_resources.isEmpty()) {
        setError(i18n("Could interpret request"), i18n("Please verify if the request was valid"));
        sendErrorFinished(InternalError, errors.join(QLatin1Char('\n')));
        return;
    }
    enableButtonOk(true);

    // Search for the iso 639 names to present it nicely to the user
    QStringList niceNames;
    QFile file(QLatin1String("/usr/share/xml/iso-codes/iso_639.xml"));
    file.open(QFile::ReadOnly);
    QXmlQuery query;
    query.bindVariable(QLatin1String("path"), &file);
    for (const QString &font : iso639) {
        QString queryTxt;
        queryTxt = QString(QLatin1String("declare variable $path external;"
                                         "doc($path)/iso_639_entries/"
                                         "iso_639_entry[@iso_639_2B_code=\"%1\" or "
                                         "@iso_639_2T_code=\"%1\" or "
                                         "@iso_639_1_code=\"%1\"]/string(@name)")).arg(font);
        query.setQuery(queryTxt);
        QStringList result;
        query.evaluateTo(&result);
        niceNames.append(result);
    }

//    kDebug() << "result" << niceNames << iso639;
    for (const QString &name : niceNames) {
        auto item = new QStandardItem(name);
        item->setIcon(QIcon::fromTheme(QLatin1String("fonts-package")).pixmap(32, 32));
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
    auto transaction = new PkTransaction(this);
    Transaction *t;
    t = Daemon::whatProvides(m_resources,
                             Transaction::FilterNotInstalled | Transaction::FilterArch | Transaction::FilterNewest);
    transaction->setupTransaction(t);
    setTransaction(Transaction::RoleWhatProvides, transaction);
    connect(transaction, &PkTransaction::finished, this, &PkInstallFontconfigResources::searchFinished, Qt::UniqueConnection);
    connect(transaction, &PkTransaction::package, this, &PkInstallFontconfigResources::addPackage);
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
    sendErrorFinished(Failed, QLatin1String("failed to search for provides"));
}

#include "moc_PkInstallFontconfigResources.cpp"
