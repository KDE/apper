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
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "KpkUpdateDetails.h"

#include <KpkStrings.h>

#include <KDebug>

KpkUpdateDetails::KpkUpdateDetails(PackageKit::Package *package, QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);

    // only the model package has the right state
    state = package->state();
    Transaction *t = Client::instance()->getUpdateDetail(package);
    connect(t, SIGNAL(updateDetail(PackageKit::Client::UpdateInfo)),
            this, SLOT(updateDetail(PackageKit::Client::UpdateInfo)));
    connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
            this, SLOT(updateDetailFinished(PackageKit::Transaction::ExitStatus, uint)));
}

KpkUpdateDetails::~KpkUpdateDetails()
{
//     kDebug() << "~KpkUpdateDetails()";
}

void KpkUpdateDetails::updateDetail(PackageKit::Client::UpdateInfo info)
{
    //format and show description
    QString description;
    description += "<table><tbody>";

    // update type (ie Security Update)
    if (state == Package::UnknownState) {
        state = Package::Normal;
    }
    description += "<tr><td align=\"right\"><b>" + i18n("Type") + ":</b></td><td>"
                   + KpkStrings::info(state)
                   + "</td></tr>";

    // state
    if (info.state != Client::UnknownUpgradeType) {
        description += "<tr><td align=\"right\"><b>" + i18nc("State of the upgrade (ie testing, unstable..)", "State") + ":</b></td><td>"
                    + KpkStrings::updateState(info.state)
                    + "</td></tr>";
    }

    // Issued
    if (!info.issued.toString().isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("Issued") + ":</b></td><td>"
                    + KGlobal::locale()->formatDate(info.issued.date(), KLocale::ShortDate)
                    + "</td></tr>";
    }

    // Updated
    if (!info.updated.toString().isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("Updated") + ":</b></td><td>"
                    + KGlobal::locale()->formatDate(info.updated.date(), KLocale::ShortDate)
                    + "</td></tr>";
    }

    // New version (ie package version)
    description += "<tr><td align=\"right\"><b>" + i18n("New version") + ":</b></td><td>" + info.package->name()
                + '-' + info.package->version()
                + "</td></tr>";

    // Updates (lists of packages that are updated)
    if (info.updates.size()) {
        QStringList updates;
        foreach (Package *p, info.updates) updates << p->name() + '-' + p->version();
        description += "<tr><td align=\"right\"><b>" + i18n("Updates") + ":</b></td><td>"
                    + updates.join("<br />")
                    + "</td></tr>";
    }

    // Obsoletes (lists of packages that are obsoleted)
    if (info.obsoletes.size()) {
        QStringList obsoletes;
        foreach (Package *p, info.obsoletes) obsoletes << p->id() + '-' + p->version();
        description += "<tr><td align=\"right\"><b>" + i18n("Obsoletes") + ":</b></td><td>"
                    + obsoletes.join("<br />")
                    + "</td></tr>";
    }

    // Repository (this is the repository the package come from)
    description += "<tr><td align=\"right\"><b>" + i18n("Repository") + ":</b></td><td>"
                + info.package->data()
                + "</td></tr>";

    // Description
    if (!info.updateText.isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("Description") + ":</b></td><td>"
                    + info.updateText.replace('\n', "<br />")
                    + "</td></tr>";
    }

    // ChangeLog
    if (!info.changelog.isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("Changes") + ":</b></td><td>"
                    + info.changelog.replace('\n', "<br />")
                    + "</td></tr>";
    }

    // links
    //  Vendor
    if (!info.vendorUrl.isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("Vendor")
                    + ":</b></td><td>" + getLinkList(info.vendorUrl) + "</td></tr>";
    }

    //  Bugzilla
    if (!info.bugzillaUrl.isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("Bugzilla")
                    + ":</b></td><td>" + getLinkList(info.bugzillaUrl) + "</td></tr>";
    }

    //  CVE
    if (!info.cveUrl.isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("CVE")
                    + ":</b></td><td>" + getLinkList(info.cveUrl) + "</td></tr>";
    }

    // Notice (about the need for a reboot)
    if (info.restart == Client::RestartSession || info.restart == Client::RestartSystem) {
        description += "<tr><td align=\"right\"><b>" + i18n("Notice") + ":</b></td><td>"
                    + KpkStrings::restartType(info.restart)
                    + "</td></tr>";
    }

    description += "</table></tbody>";
    descriptionKTB->setHtml(description);
}

QString KpkUpdateDetails::getLinkList(const QString &links) const
{
    QStringList linkList = links.split(';');
    int length = linkList.size();
    QString ret;

    // check for malformed strings with ';'
    if (length % 2 != 0) {
        kWarning() << "length not correct, correcting";
        length--;
    }

    for (int i = 0; i < length; i += 2) {
        if (!ret.isEmpty()) {
            ret += "<br />";
        }
        ret = "<a href=\"" + linkList.at(i) + "\">"
              + linkList.at(i + 1) + "</a>";
    }
    return ret;
}

void KpkUpdateDetails::updateDetailFinished(PackageKit::Transaction::ExitStatus status, uint runtime)
{
    Q_UNUSED(status)
    Q_UNUSED(runtime)
    if (descriptionKTB->document()->toPlainText().isEmpty()) {
       descriptionKTB->setPlainText(i18n("No update description was found."));
    }
}

#include "KpkUpdateDetails.moc"
