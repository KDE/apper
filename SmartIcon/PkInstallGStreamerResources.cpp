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

#include "PkInstallGStreamerResources.h"

#include <KpkReviewChanges.h>
#include <KpkTransaction.h>
#include <KpkStrings.h>

#include <KLocale>
#include <KMessageBox>

#include <KDebug>

PkInstallGStreamerResources::PkInstallGStreamerResources(uint xid,
                                                         const QStringList &resources,
                                                         const QString &interaction,
                                                         const QDBusMessage &message,
                                                         QWidget *parent)
 : KpkAbstractTask(xid, interaction, message, parent),
   m_resources(resources)
{
}

PkInstallGStreamerResources::~PkInstallGStreamerResources()
{
}

void PkInstallGStreamerResources::start()
{
    int ret = KMessageBox::Yes;
    QStringList niceNames;
    QStringList search;
    if (showConfirmSearch()) {
        bool encoder = false;
        bool decoder = false;
        foreach (const QString &codec, m_resources) {
            if (codec.contains("|gstreamer0.10(decoder")) {
                encoder = true;
            } else if (codec.contains("|gstreamer0.10(encoder")) {
                decoder = true;
            }
            niceNames << codec.section('|', 0, 0);
            search << codec.section('|', 1, -1);
        }

        QString message = i18np("The following plugin is required: <ul><li>%2</li></ul>"
                                "Do you want to search for this now?",
                                "The following plugins are required: <ul><li>%2</li></ul>"
                                "Do you want to search for these now?",
                                niceNames.size(),
                                niceNames.join("</li><li>"));

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
        QString msg = "<h3>" + title + "</h3>" + message;
        KGuiItem searchBt = KStandardGuiItem::yes();
        searchBt.setText(i18nc("Search for packages" ,"Search"));
        searchBt.setIcon(KIcon("edit-find"));
        ret = KMessageBox::questionYesNoWId(parentWId(), msg, title, searchBt);
    }

    if (ret == KMessageBox::Yes) {
        Transaction *t = Client::instance()->whatProvides(Enum::ProvidesCodec,
                                                          search,
                                                          Enum::FilterNotInstalled |
                                                          Enum::FilterArch |
                                                          Enum::FilterNewest);
        if (t->error()) {
            QString msg(i18n("Failed to search for provides"));
            KMessageBox::sorryWId(parentWId(), KpkStrings::daemonError(t->error()), msg);
            sendErrorFinished(InternalError, msg);
        } else {
            connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                    this, SLOT(whatProvidesFinished(PackageKit::Enum::Exit)));
            connect(t, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                    this, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
            if (showProgress()) {
                kTransaction()->setTransaction(t);
                kTransaction()->show();
            }
        }
    } else {
        sendErrorFinished(Cancelled, i18n("did not agree to search"));
    }
}

void PkInstallGStreamerResources::whatProvidesFinished(PackageKit::Enum::Exit status)
{
    kDebug() << "Finished.";
    if (status == Enum::ExitSuccess) {
        if (m_foundPackages.size()) {
            kTransaction()->hide();
            KpkReviewChanges *frm = new KpkReviewChanges(m_foundPackages, this, parentWId());
            if (frm->exec(operationModes()) == 0) {
                sendErrorFinished(Failed, "Transaction did not finish with success");
            } else {
                finishTaskOk();
            }
        } else {
            if (showWarning()) {
                KMessageBox::sorryWId(parentWId(),
                                      i18n("Could not find plugin "
                                           "in any configured software source"),
                                      i18n("Failed to search for plugin"));
            }
            sendErrorFinished(NoPackagesFound, "failed to find codec");
        }
    } else {
        sendErrorFinished(Failed, "what provides failed");
    }
}

void PkInstallGStreamerResources::addPackage(QSharedPointer<PackageKit::Package> package)
{
    m_foundPackages.append(package);
}

#include "PkInstallGStreamerResources.moc"
