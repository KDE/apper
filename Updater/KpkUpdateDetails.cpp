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

#include "KpkUpdateDetails.h"

#include <KpkStrings.h>

#include <KMessageBox>
#include <KPixmapSequence>

#include <KDebug>

KpkUpdateDetails::KpkUpdateDetails(QSharedPointer<PackageKit::Package>package, QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);

    // only the model package has the right m_info
    m_info = package->info();
    Transaction *t = Client::instance()->getUpdateDetail(package);
    if (t->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
    } else {
        connect(t, SIGNAL(updateDetail(PackageKit::Client::UpdateInfo)),
                this, SLOT(updateDetail(PackageKit::Client::UpdateInfo)));
        connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                this, SLOT(updateDetailFinished()));
    }

    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(descriptionKTB->viewport());
    m_busySeq->start();
}

KpkUpdateDetails::~KpkUpdateDetails()
{
}

void KpkUpdateDetails::updateDetail(PackageKit::Client::UpdateInfo info)
{
    //format and show description
    QString description;

    // update type (ie Security Update)
    if (m_info == Enum::InfoEnhancement) {
        description += i18n("This update will add new features and expand functionality.") + "<br/>";
    } else if (m_info == Enum::InfoBugfix) {
        description += i18n("This update will fix bugs and other non-critical problems.") + "<br/>";
    } else if (m_info == Enum::InfoImportant) {
        description += i18n("This update is important as it may solve critical problems.") + "<br/>";
    } else if (m_info == Enum::InfoSecurity) {
        description += i18n("This update is needed to fix a security vulnerability with this package.") + "<br/>";
    } else if (m_info == Enum::InfoBlocked) {
        description += i18n("This update is blocked.") + "<br/>";
    }

    // Issued and Updated
    if (!info.issued.toString().isEmpty() && !info.updated.toString().isEmpty()) {
        description += i18n("This notification was issued on %1 and last updated on %2.",
                            KGlobal::locale()->formatDate(info.issued.date(), KLocale::ShortDate),
                            KGlobal::locale()->formatDate(info.updated.date(), KLocale::ShortDate))
                       + "<br/>";
    } else if (!info.issued.toString().isEmpty()) {
        description += i18n("This notification was issued on %1.",
                            KGlobal::locale()->formatDate(info.issued.date(), KLocale::ShortDate))
                       + "<br/>";
    }

    // Description
    if (!info.updateText.isEmpty()) {
        description += info.updateText.replace('\n', "<br/>")
                       + "<br/>";
    }

    // links
    //  Vendor
    if (!info.vendorUrl.isEmpty()) {
        description += i18np("For more information about this update please visit this website:",
                             "For more information about this update please visit these websites:",
                             info.vendorUrl.split(';').size() % 2)
                       + "<br/>" + getLinkList(info.vendorUrl) + "<br/>";
    }

    //  Bugzilla
    if (!info.bugzillaUrl.isEmpty()) {
        description += i18np("For more information about bugs fixed by this update please visit this website:",
                             "For more information about bugs fixed by this update please visit these websites:",
                             info.bugzillaUrl .split(';').size() % 2)
                       + "<br/>" + getLinkList(info.bugzillaUrl) + "<br/>";
    }

    //  CVE
    if (!info.cveUrl.isEmpty()) {
        description += i18np("For more information about this security update please visit this website:",
                             "For more information about this security update please visit these websites:",
                             info.cveUrl .split(';').size() % 2)
                       + "<br/>" + getLinkList(info.cveUrl) + "<br/>";
    }

    // Notice (about the need for a reboot)
    if (info.restart == Enum::RestartSession) {
        description += i18n("The computer will have to be restarted after the update for the changes to take effect.")
                       + "<br/>";
    } else if (info.restart == Enum::RestartSystem) {
        description += i18n("You will need to log out and back in after the update for the changes to take effect.")
                       + "<br/>";
    }

    // State
    if (info.state == Enum::UpdateStateUnstable) {
        description += i18n("The classifaction of this update is unstable which means it is not designed for production use.")
                       + "<br/>";
    } else if (info.state == Enum::UpdateStateTesting) {
        description += i18n("This is a test update, and is not designed for normal use. Please report any problems or regressions you encounter.")
                       + "<br/>";
    }

    // only show changelog if we didn't have any update text
    if (info.updateText.isEmpty() && !info.changelog.isEmpty()) {
        description += i18n("The developer logs will be shown as no description is available for this update:")
                       + "<br/>";
        description += info.changelog.replace('\n', "<br/>") + "<br/>";
    }

    // Updates (lists of packages that are updated)
    if (info.updates.size()) {
        description += "<b>" + i18n("Updates:") + "</b><br/>";
        QStringList updates;
        foreach (const QSharedPointer<PackageKit::Package> &p, info.updates) {
             updates += QString::fromUtf8("\xE2\x80\xA2 ") + p->name() + " - " + p->version();
        }
        description += updates.join("<br/") + "</ul><br/>";
    }

    // Obsoletes (lists of packages that are obsoleted)
    if (info.obsoletes.size()) {
        description += "</b>" + i18n("Obsoletes:") + "</b><br/>";
        QStringList obsoletes;
        foreach (const QSharedPointer<PackageKit::Package> &p, info.obsoletes) {
             obsoletes += QString::fromUtf8("\xE2\x80\xA2 ") + p->name() + " - " + p->version();
        }
        description += obsoletes.join("<br/") + "</ul><br/>";
    }

    // Repository (this is the repository the package comes from)
    if (!info.package->data().isEmpty()) {
         description += "<b>" + i18n("Repository:") + "</b> " + info.package->data() + "<br/>";
    }

    descriptionKTB->setHtml(description);
    m_busySeq->stop();
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
        ret = "<a href=\"" + linkList.at(i) + QString::fromUtf8("\">\xE2\x80\xA2 ")
              + linkList.at(i + 1) + "</a>";
    }
    return ret;
}

void KpkUpdateDetails::updateDetailFinished()
{
    if (descriptionKTB->document()->toPlainText().isEmpty()) {
        descriptionKTB->setPlainText(i18n("No update description was found."));
    }
}

#include "KpkUpdateDetails.moc"
