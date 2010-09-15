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

#include <QGraphicsOpacityEffect>

#include <KDebug>

#define FINAL_HEIGHT 160

KpkUpdateDetails::KpkUpdateDetails(QWidget *parent)
 : QWidget(parent),
   m_show(false)
{
    setupUi(this);

    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(descriptionKTB->viewport());

    QWidget *actionsViewport = descriptionKTB->viewport();
    QPalette palette = actionsViewport->palette();
    palette.setColor(actionsViewport->backgroundRole(), Qt::transparent);
    palette.setColor(actionsViewport->foregroundRole(), palette.color(QPalette::WindowText));
    actionsViewport->setPalette(palette);

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(descriptionKTB);
    effect->setOpacity(0);
    descriptionKTB->setGraphicsEffect(effect);
    m_fadeDetails = new QPropertyAnimation(effect, "opacity");
    m_fadeDetails->setDuration(500);
    m_fadeDetails->setStartValue(qreal(0));
    m_fadeDetails->setEndValue(qreal(1));
    connect(m_fadeDetails, SIGNAL(finished()), this, SLOT(display()));


    QPropertyAnimation *anim1 = new QPropertyAnimation(this, "maximumSize");
    anim1->setDuration(500);
    anim1->setEasingCurve(QEasingCurve::OutQuart);
    anim1->setStartValue(QSize(QWIDGETSIZE_MAX, 0));
    anim1->setEndValue(QSize(QWIDGETSIZE_MAX, FINAL_HEIGHT));
    QPropertyAnimation *anim2 = new QPropertyAnimation(this, "minimumSize");
    anim2->setDuration(500);
    anim2->setEasingCurve(QEasingCurve::OutQuart);
    anim2->setStartValue(QSize(QWIDGETSIZE_MAX, 0));
    anim2->setEndValue(QSize(QWIDGETSIZE_MAX, FINAL_HEIGHT));

    m_expandPanel = new QParallelAnimationGroup;
    m_expandPanel->addAnimation(anim1);
    m_expandPanel->addAnimation(anim2);
    connect(m_expandPanel, SIGNAL(finished()), this, SLOT(display()));

}

KpkUpdateDetails::~KpkUpdateDetails()
{
}

void KpkUpdateDetails::setPackage(const QString &packageId, Enum::Info updateInfo)
{
    if (m_packageId == packageId) {
        return;
    }
    m_show       = true;
    m_packageId  = packageId;
    m_updateInfo = updateInfo;
    m_currentDescription.clear();

    QSharedPointer<Package> package = QSharedPointer<Package>(new Package(m_packageId,
                                                                          Enum::UnknownInfo,
                                                                          QString()));;
    Transaction *transaction = new Transaction(QString());
    connect(transaction, SIGNAL(updateDetail(PackageKit::Client::UpdateInfo)),
            this, SLOT(updateDetail(PackageKit::Client::UpdateInfo)));
    connect(transaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            this, SLOT(display()));
    transaction->getUpdateDetail(package);
    if (transaction->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(transaction->error()));
    } else {
        if (maximumSize().height() == 0) {
            // Expand the panel
            m_expandPanel->setDirection(QAbstractAnimation::Forward);
            m_expandPanel->start();
        } else if (m_fadeDetails->currentValue().toReal() != 0) {
            // Hide the old description
            m_fadeDetails->setDirection(QAbstractAnimation::Backward);
            m_fadeDetails->start();
        }
        m_busySeq->start();
    }
}

void KpkUpdateDetails::hide()
{
    m_show = false;
    m_packageId.clear();
    if (maximumSize().height() == FINAL_HEIGHT &&
        m_fadeDetails->currentValue().toReal() == 1) {
        m_fadeDetails->setDirection(QAbstractAnimation::Backward);
        m_fadeDetails->start();
    } else if (maximumSize().height() == FINAL_HEIGHT &&
               m_fadeDetails->currentValue().toReal() == 0) {
        m_expandPanel->setDirection(QAbstractAnimation::Backward);
        m_expandPanel->start();
    }
}

void KpkUpdateDetails::display()
{
    if (!m_show) {
        hide();
        return;
    }

    if (maximumSize().height() == FINAL_HEIGHT &&
        !m_currentDescription.isEmpty() &&
        m_fadeDetails->currentValue().toReal() == 0) {
        descriptionKTB->setHtml(m_currentDescription);

        m_fadeDetails->setDirection(QAbstractAnimation::Forward);
        m_fadeDetails->start();
    }
}

void KpkUpdateDetails::updateDetail(PackageKit::Client::UpdateInfo info)
{
    //format and show description
    QString description;

    // update type (ie Security Update)
    if (m_updateInfo == Enum::InfoEnhancement) {
        description += "<p>" +
                       i18n("This update will add new features and expand functionality.") +
                       "</p>";
    } else if (m_updateInfo == Enum::InfoBugfix) {
        description += "<p>" +
                       i18n("This update will fix bugs and other non-critical problems.") +
                       "</p>";
    } else if (m_updateInfo == Enum::InfoImportant) {
        description += "<p>" +
                       i18n("This update is important as it may solve critical problems.") +
                       "</p>";
    } else if (m_updateInfo == Enum::InfoSecurity) {
        description += "<p>" +
                       i18n("This update is needed to fix a security vulnerability with this package.") +
                       "</p>";
    } else if (m_updateInfo == Enum::InfoBlocked) {
        description += "<p>" +
                       i18n("This update is blocked.") +
                       "</p>";
    }

    // Issued and Updated
    if (!info.issued.toString().isEmpty() && !info.updated.toString().isEmpty()) {
        description += "<p>" +
                       i18n("This notification was issued on %1 and last updated on %2.",
                            KGlobal::locale()->formatDate(info.issued.date(), KLocale::ShortDate),
                            KGlobal::locale()->formatDate(info.updated.date(), KLocale::ShortDate)) +
                       "</p>";
    } else if (!info.issued.toString().isEmpty()) {
        description += "<p>" +
                       i18n("This notification was issued on %1.",
                            KGlobal::locale()->formatDate(info.issued.date(), KLocale::ShortDate)) +
                       "</p>";
    }

    // Description
    if (!info.updateText.isEmpty()) {
        description += "<p>" +
                       info.updateText.replace('\n', "<br/>") +
                       "</p>";
    }

    // links
    //  Vendor
    if (!info.vendorUrl.isEmpty()) {
        description += "<p>" +
                       i18np("For more information about this update please visit this website:",
                             "For more information about this update please visit these websites:",
                             info.vendorUrl.split(';').size() % 2) + "<br/>" +
                       getLinkList(info.vendorUrl) +
                       "</p>";
    }

    //  Bugzilla
    if (!info.bugzillaUrl.isEmpty()) {
        description += "<p>" +
                       i18np("For more information about bugs fixed by this update please visit this website:",
                             "For more information about bugs fixed by this update please visit these websites:",
                             info.bugzillaUrl .split(';').size() % 2) + "<br/>" +
                       getLinkList(info.bugzillaUrl) +
                       "</p>";
    }

    //  CVE
    if (!info.cveUrl.isEmpty()) {
        description += "<p>" +
                       i18np("For more information about this security update please visit this website:",
                             "For more information about this security update please visit these websites:",
                             info.cveUrl .split(';').size() % 2) + "<br/>" +
                       getLinkList(info.cveUrl) +
                       "</p>";
    }

    // Notice (about the need for a reboot)
    if (info.restart == Enum::RestartSession) {
        description += "<p>" +
                       i18n("The computer will have to be restarted after the update for the changes to take effect.") +
                       "</p>";
    } else if (info.restart == Enum::RestartSystem) {
        description += "<p>" +
                       i18n("You will need to log out and back in after the update for the changes to take effect.") +
                       "</p>";
    }

    // State
    if (info.state == Enum::UpdateStateUnstable) {
        description += "<p>" +
                       i18n("The classification of this update is unstable which means it is not designed for production use.") +
                       "</p>";
    } else if (info.state == Enum::UpdateStateTesting) {
        description += "<p>" +
                       i18n("This is a test update, and is not designed for normal use. Please report any problems or regressions you encounter.") +
                       "</p>";
    }

    // only show changelog if we didn't have any update text
    if (info.updateText.isEmpty() && !info.changelog.isEmpty()) {
        description += "<p>" +
                       i18n("The developer logs will be shown as no description is available for this update:") +
                       "<br/>" +
                       info.changelog.replace('\n', "<br/>") +
                       "</p>";
    }

    // Updates (lists of packages that are updated)
    if (info.updates.size()) {
        description += "<p>" + i18n("Updates:") + "<br/>";
        QStringList updates;
        foreach (const QSharedPointer<PackageKit::Package> &p, info.updates) {
             updates += QString::fromUtf8("\xE2\x80\xA2 ") + p->name() + " - " + p->version();
        }
        description += updates.join("<br/>") + "</p>";
    }

    // Obsoletes (lists of packages that are obsoleted)
    if (info.obsoletes.size()) {
        description += "<p></b>" + i18n("Obsoletes:") + "</b><br/>";
        QStringList obsoletes;
        foreach (const QSharedPointer<PackageKit::Package> &p, info.obsoletes) {
             obsoletes += QString::fromUtf8("\xE2\x80\xA2 ") + p->name() + " - " + p->version();
        }
        description += obsoletes.join("<br>/") + "</p>";
    }

    // Repository (this is the repository the package comes from)
    if (!info.package->data().isEmpty()) {
         description += "<p>" + i18n("Repository:") + ' ' + info.package->data() + "</p>";
    }

    m_currentDescription = description;
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
            ret += "<br/>";
        }
        ret = QString::fromUtf8(" \xE2\x80\xA2 <a href=\"") + linkList.at(i) + "\">"
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
