/***************************************************************************
 *   Copyright (C) 2009 by Daniel Nicoletti                                *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "KpkUpdateDetails.h"

#include <KpkStrings.h>

#include <QPlainTextEdit>
#include <QTextDocument>

KpkUpdateDetails::KpkUpdateDetails(PackageKit::Package *package, QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);

    Transaction *t = Client::instance()->getUpdateDetail(package);
    connect(t, SIGNAL(updateDetail(PackageKit::Client::UpdateInfo)),
            this, SLOT(updateDetail(PackageKit::Client::UpdateInfo)));
    connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
            this, SLOT(updateDetailFinished(PackageKit::Transaction::ExitStatus, uint)));
}

KpkUpdateDetails::~KpkUpdateDetails()
{
//     qDebug() << "~KpkUpdateDetails()";
}

void KpkUpdateDetails::updateDetail(PackageKit::Client::UpdateInfo info)
{
    //format and show description
    QString description;
    description += "<table><tbody>";
    description += "<tr><td align=\"right\"><b>" + i18n("New version") + ":</b></td><td>" + info.package->name()
                + "-" + info.package->version()
                + "</td></tr>";

    if (info.updates.size()) {
        QStringList updates;
        foreach (Package *p, info.updates) updates << p->name() + "-" + p->version();
        description += "<tr><td align=\"right\"><b>" + i18n("Updates") + ":</b></td><td>"
                    + updates.join(", ")
                    + "</td></tr>";
    }
    if (info.obsoletes.size()) {
        QStringList obsoletes;
        foreach (Package *p, info.obsoletes) obsoletes << p->id() + "-" + p->version();
        description += "<tr><td align=\"right\"><b>" + i18n("Obsoletes") + ":</b></td><td>"
                    + obsoletes.join(", ")
                    + "</td></tr>";
    }
    if (!info.updateText.isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("Details") + ":</b></td><td>"
                    + info.updateText.replace('\n', "<br />")
                    + "</td></tr>";
    }
    if (!info.vendorUrl.isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("Vendor Home Page")
                    + ":</b></td><td><a href=\"" + info.vendorUrl.section(';', 0, 0) + "\">"
                    + info.vendorUrl.section(';', -1)
                    + "</a></td></tr>";
    }
    if (!info.bugzillaUrl.isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("Bugzilla Home Page")
                    + ":</b></td><td><a href=\"" + info.bugzillaUrl.section(';', 0, 0) + "\">"
                    + info.bugzillaUrl.section(';', -1)
                    + "</a></td></tr>";
    }
    if (!info.cveUrl.isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("CVE Home Page")
                    + ":</b></td><td><a href=\"" + info.cveUrl.section(';', 0, 0) + "\">"
                    + info.cveUrl.section(';', -1)
                    + "</a></td></tr>";
    }
    if (!info.changelog.isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("Change Log") + ":</b></td><td>"
                    + info.changelog.replace('\n', "<br />")
                    + "</td></tr>";
    }
    if (info.state != Client::UnknownUpgradeType) {
        description += "<tr><td align=\"right\"><b>" + i18n("State") + ":</b></td><td>"
                    + KpkStrings::updateState(info.state)
                    + "</td></tr>";
    }
    if (info.restart != Client::UnknownRestartType) {
        description += "<tr><td align=\"right\"><b>" + i18n("Restart") + ":</b></td><td>"
                    + KpkStrings::restartTypeFuture(info.restart)
                    + "</td></tr>";
    }
    if (!info.issued.toString().isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("Issued") + ":</b></td><td>"
                    + KGlobal::locale()->formatDate(info.issued.date(), KLocale::ShortDate)
                    + "</td></tr>";
    }
    if (!info.updated.toString().isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("Updated") + ":</b></td><td>"
                    + KGlobal::locale()->formatDate(info.updated.date(), KLocale::ShortDate)
                    + "</td></tr>";
    }
    description += "</table></tbody>";
    descriptionKTB->setHtml(description);
}


void KpkUpdateDetails::updateDetailFinished(PackageKit::Transaction::ExitStatus status, uint runtime)
{
    Q_UNUSED(status)
    Q_UNUSED(runtime)
//     if (status == Transaction::Success)
//      descriptionDW->setVisible(true);
}

#include "KpkUpdateDetails.moc"
