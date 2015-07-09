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

#include <KDebug>

PkInstallGStreamerResources::PkInstallGStreamerResources(uint xid,
                                                         const QStringList &resources,
                                                         const QString &interaction,
                                                         const QDBusMessage &message,
                                                         QWidget *parent)
    : SessionTask(xid, interaction, message, parent)
{
    setWindowTitle(i18n("Install GStreamer Resources"));

    IntroDialog *introDialog = new IntroDialog(this);
    QStandardItemModel *model = new QStandardItemModel(this);
    introDialog->setModel(model);
    connect(introDialog, SIGNAL(continueChanged(bool)),
            this, SLOT(enableButtonOk(bool)));
    setMainWidget(introDialog);

    bool encoder = false;
    bool decoder = false;
    // Resources are strings like "ID3 tag|gstreamer0.10(decoder-application/x-id3)()(64bit)"
    foreach (const QString &codec, resources) {
        if (codec.contains("|gstreamer0.10(decoder")) {
            decoder = true;
        } else if (codec.contains("|gstreamer0.10(encoder")) {
            encoder = true;
        }

        QStandardItem *item = new QStandardItem(codec.section('|', 0, 0));
        item->setIcon(QIcon::fromTheme("x-kde-nsplugin-generated").pixmap(32, 32));
        item->setFlags(Qt::ItemIsEnabled);
        model->appendRow(item);

        m_resources << codec.section('|', 1, -1);
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
    PkTransaction *transaction = new PkTransaction(this);
    Transaction *t;
    t = Daemon::whatProvides(m_resources,
                             Transaction::FilterNotInstalled | Transaction::FilterArch | Transaction::FilterNewest);
    transaction->setupTransaction(t);
    setTransaction(Transaction::RoleWhatProvides, transaction);
    connect(transaction, SIGNAL(finished(PkTransaction::ExitStatus)),
            this, SLOT(searchFinished(PkTransaction::ExitStatus)), Qt::UniqueConnection);
    connect(transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
}

void PkInstallGStreamerResources::notFound()
{
    if (showWarning()) {
        QString mgs = i18n("No results found");
        setInfo(mgs,
                i18n("Could not find plugin "
                     "in any configured software source"));
    }
    sendErrorFinished(NoPackagesFound, "failed to find codec");
}

#include "PkInstallGStreamerResources.moc"
