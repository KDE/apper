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

#include "PkInstallGStreamerResources.h"

#include "IntroDialog.h"
#include <Daemon>

#include <PkStrings.h>

#include <QStandardItemModel>
#include <KLocalizedString>

#include <QLoggingCategory>

PkInstallGStreamerResources::PkInstallGStreamerResources(uint xid,
                                                         const QStringList &resources,
                                                         const QString &interaction,
                                                         const QDBusMessage &message,
                                                         QWidget *parent)
    : SessionTask(xid, interaction, message, parent)
{
    setWindowTitle(i18n("Install GStreamer Resources"));

    auto introDialog = new IntroDialog(this);
    auto model = new QStandardItemModel(this);
    introDialog->setModel(model);
    connect(introDialog, &IntroDialog::continueChanged, this, &PkInstallGStreamerResources::enableButtonOk);
    setMainWidget(introDialog);

    bool encoder = false;
    bool decoder = false;
    // Resources are strings like "ID3 tag|gstreamer0.10(decoder-application/x-id3)()(64bit)"
    for (const QString &codec : resources) {
        if (codec.contains(QLatin1String("|gstreamer0.10(decoder"))) {
            decoder = true;
        } else if (codec.contains(QLatin1String("|gstreamer0.10(encoder"))) {
            encoder = true;
        }

        auto item = new QStandardItem(codec.section(QLatin1Char('|'), 0, 0));
        item->setIcon(QIcon::fromTheme(QLatin1String("x-kde-nsplugin-generated")).pixmap(32, 32));
        item->setFlags(Qt::ItemIsEnabled);
        model->appendRow(item);

        m_resources << codec.section(QLatin1Char('|'), 1, -1);
    }


    QString description;
    description = i18np("The following plugin is required. "
                        "Do you want to search for this now?",
                        "The following plugins are required. "
                        "Do you want to search for these now?",
                        m_resources.size());
    enableButtonOk(true);

    QString title;
    // this will come from DBus interface
    if (parentTitle.isNull()) {
        if (decoder && !encoder) {
            // TRANSLATORS: a random program which we can't get the name wants to decode something
            title = i18np("A program requires an additional plugin to decode this file",
                          "A program requires additional plugins to decode this file",
                          m_resources.size());
        } else if (!decoder && encoder) {
            // TRANSLATORS: a random program which we can't get the name wants to encode something
            title = i18np("A program requires an additional plugin to encode this file",
                          "A program requires additional plugins to encode this file",
                          m_resources.size());
        } else {
            // TRANSLATORS: a random program which we can't get the name wants to do something (unknown)
            title = i18np("A program requires an additional plugin for this operation",
                          "A program requires additional plugins for this operation",
                          m_resources.size());
        }
    } else {
        if (decoder && !encoder) {
            // TRANSLATORS: a program wants to decode something (unknown) -- string is a program name, e.g. "Movie Player"
            title = i18np("%2 requires an additional plugin to decode this file",
                        "%2 requires additional plugins to decode this file",
                        m_resources.size(),
                        parentTitle);
        } else if (!decoder && encoder) {
            // TRANSLATORS: a program wants to encode something (unknown) -- string is a program name, e.g. "Movie Player"
            title = i18np("%2 requires an additional plugin to encode this file",
                        "%2 requires additional plugins to encode this file",
                        m_resources.size(),
                        parentTitle);
        } else {
            // TRANSLATORS: a program wants to do something (unknown) -- string is a program name, e.g. "Movie Player"
            title = i18np("%2 requires an additional plugin for this operation",
                        "%2 requires additional plugins for this operation",
                        m_resources.size(),
                        parentTitle);
        }
    }

    introDialog->setDescription(description);
    setTitle(title);
}

PkInstallGStreamerResources::~PkInstallGStreamerResources()
{
}

void PkInstallGStreamerResources::search()
{
    auto transaction = new PkTransaction(this);
    Transaction *t;
    t = Daemon::whatProvides(m_resources,
                             Transaction::FilterNotInstalled | Transaction::FilterArch | Transaction::FilterNewest);
    transaction->setupTransaction(t);
    setTransaction(Transaction::RoleWhatProvides, transaction);
    connect(transaction, &PkTransaction::finished, this, &PkInstallGStreamerResources::searchFinished, Qt::UniqueConnection);
    connect(transaction, &PkTransaction::package, this, &PkInstallGStreamerResources::addPackage);
}

void PkInstallGStreamerResources::notFound()
{
    if (showWarning()) {
        QString mgs = i18n("No results found");
        setInfo(mgs,
                i18n("Could not find plugin "
                     "in any configured software source"));
    }
    sendErrorFinished(NoPackagesFound, QLatin1String("failed to find codec"));
}

#include "moc_PkInstallGStreamerResources.cpp"
