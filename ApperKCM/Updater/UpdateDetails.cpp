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

#include "UpdateDetails.h"

#include <PkStrings.h>

#include <KMessageBox>
#include <KPixmapSequence>

#include <QAbstractAnimation>
#include <QGraphicsOpacityEffect>

#include <Transaction>

#include <KDebug>

#define FINAL_HEIGHT 160

UpdateDetails::UpdateDetails(QWidget *parent)
 : QWidget(parent),
   m_show(false),
   m_transaction(0)
{
    setupUi(this);

    connect(hideTB, SIGNAL(clicked()), this, SLOT(hide()));

    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(this);

    QWidget *actionsViewport = descriptionKTB->viewport();
    QPalette palette = actionsViewport->palette();
    palette.setColor(actionsViewport->backgroundRole(), Qt::transparent);
    palette.setColor(actionsViewport->foregroundRole(), palette.color(QPalette::WindowText));
    actionsViewport->setPalette(palette);

    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(descriptionKTB);
    effect->setOpacity(0);
    descriptionKTB->setGraphicsEffect(effect);
    m_fadeDetails = new QPropertyAnimation(effect, "opacity", this);
    m_fadeDetails->setDuration(500);
    m_fadeDetails->setStartValue(qreal(0));
    m_fadeDetails->setEndValue(qreal(1));
    connect(m_fadeDetails, SIGNAL(finished()), this, SLOT(display()));


    QPropertyAnimation *anim1 = new QPropertyAnimation(this, "maximumSize", this);
    anim1->setDuration(500);
    anim1->setEasingCurve(QEasingCurve::OutQuart);
    anim1->setStartValue(QSize(QWIDGETSIZE_MAX, 0));
    anim1->setEndValue(QSize(QWIDGETSIZE_MAX, FINAL_HEIGHT));
    QPropertyAnimation *anim2 = new QPropertyAnimation(this, "minimumSize", this);
    anim2->setDuration(500);
    anim2->setEasingCurve(QEasingCurve::OutQuart);
    anim2->setStartValue(QSize(QWIDGETSIZE_MAX, 0));
    anim2->setEndValue(QSize(QWIDGETSIZE_MAX, FINAL_HEIGHT));

    m_expandPanel = new QParallelAnimationGroup(this);
    m_expandPanel->addAnimation(anim1);
    m_expandPanel->addAnimation(anim2);
    connect(m_expandPanel, SIGNAL(finished()), this, SLOT(display()));

}

UpdateDetails::~UpdateDetails()
{
}

void UpdateDetails::setPackage(const QString &packageId, Package::Info updateInfo)
{
    if (m_packageId == packageId) {
        return;
    }
    m_show       = true;
    m_packageId  = packageId;
    m_updateInfo = updateInfo;
    m_currentDescription.clear();
    if (m_transaction) {
        disconnect(m_transaction, SIGNAL(package(PackageKit::Package)),
                   this, SLOT(updateDetail(PackageKit::Package)));
        disconnect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
                   this, SLOT(display()));
    }

    Package package(m_packageId, Package::UnknownInfo, QString());
    m_transaction = new Transaction(this);
    connect(m_transaction, SIGNAL(package(PackageKit::Package)),
            this, SLOT(updateDetail(PackageKit::Package)));
    connect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
            this, SLOT(display()));
    m_transaction->getUpdateDetail(package);
    Transaction::InternalError error = m_transaction->error();
    if (error) {
        disconnect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   this, SLOT(display()));
        m_transaction = 0;
        KMessageBox::sorry(this, PkStrings::daemonError(error));
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

void UpdateDetails::hide()
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

void UpdateDetails::display()
{
    // set transaction to 0 as if PK crashes
    // UpdateDetails won't be emmited
    m_transaction = 0;

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

void UpdateDetails::updateDetail(const PackageKit::Package &package)
{
    //format and show description
    QString description;

    // update type (ie Security Update)
    if (m_updateInfo == Package::InfoEnhancement) {
        description += "<p>" +
                       i18n("This update will add new features and expand functionality.") +
                       "</p>";
    } else if (m_updateInfo == Package::InfoBugfix) {
        description += "<p>" +
                       i18n("This update will fix bugs and other non-critical problems.") +
                       "</p>";
    } else if (m_updateInfo == Package::InfoImportant) {
        description += "<p>" +
                       i18n("This update is important as it may solve critical problems.") +
                       "</p>";
    } else if (m_updateInfo == Package::InfoSecurity) {
        description += "<p>" +
                       i18n("This update is needed to fix a security vulnerability with this package.") +
                       "</p>";
    } else if (m_updateInfo == Package::InfoBlocked) {
        description += "<p>" +
                       i18n("This update is blocked.") +
                       "</p>";
    }

    // Issued and Updated
    if (!package.issued().toString().isEmpty() && !package.updated().toString().isEmpty()) {
        description += "<p>" +
                       i18n("This notification was issued on %1 and last updated on %2.",
                            KGlobal::locale()->formatDateTime(package.issued(), KLocale::ShortDate),
                            KGlobal::locale()->formatDateTime(package.updated(), KLocale::ShortDate)) +
                       "</p>";
    } else if (!package.issued().toString().isEmpty()) {
        description += "<p>" +
                       i18n("This notification was issued on %1.",
                            KGlobal::locale()->formatDateTime(package.issued(), KLocale::ShortDate)) +
                       "</p>";
    }

    // Description
    if (!package.updateText().isEmpty()) {
        QString updateText = package.updateText();
        updateText.replace('\n', "<br/>");
        description += "<p><pre>" +
                       updateText +
                       "</pre></p>";
    }

    // links
    //  Vendor
    if (!package.vendorUrl().isEmpty()) {
        description += "<p>" +
                       i18np("For more information about this update please visit this website:",
                             "For more information about this update please visit these websites:",
                             package.vendorUrl().split(';').size() % 2) + "<br/>" +
                       getLinkList(package.vendorUrl()) +
                       "</p>";
    }

    //  Bugzilla
    if (!package.bugzillaUrl().isEmpty()) {
        description += "<p>" +
                       i18np("For more information about bugs fixed by this update please visit this website:",
                             "For more information about bugs fixed by this update please visit these websites:",
                             package.bugzillaUrl().split(';').size() % 2) + "<br/>" +
                       getLinkList(package.bugzillaUrl()) +
                       "</p>";
    }

    //  CVE
    if (!package.cveUrl().isEmpty()) {
        description += "<p>" +
                       i18np("For more information about this security update please visit this website:",
                             "For more information about this security update please visit these websites:",
                             package.cveUrl().split(';').size() % 2) + "<br/>" +
                       getLinkList(package.cveUrl()) +
                       "</p>";
    }

    // Notice (about the need for a reboot)
    if (package.restart() == Package::RestartSystem) {
        description += "<p>" +
                       i18n("The computer will have to be restarted after the update for the changes to take effect.") +
                       "</p>";
    } else if (package.restart() == Package::RestartSession) {
        description += "<p>" +
                       i18n("You will need to log out and back in after the update for the changes to take effect.") +
                       "</p>";
    }

    // State
    if (package.state() == Package::UpdateStateUnstable) {
        description += "<p>" +
                       i18n("The classification of this update is unstable which means it is not designed for production use.") +
                       "</p>";
    } else if (package.state() == Package::UpdateStateTesting) {
        description += "<p>" +
                       i18n("This is a test update, and is not designed for normal use. Please report any problems or regressions you encounter.") +
                       "</p>";
    }

    // only show changelog if we didn't have any update text
    if (package.updateText().isEmpty() && !package.changelog().isEmpty()) {
        QString changelog = package.changelog();
        changelog.replace('\n', "<br/>");
        description += "<p>" +
                       i18n("The developer logs will be shown as no description is available for this update:") +
                       "<br/><pre>" +
                       changelog +
                       "</pre></p>";
    }

    // Updates (lists of packages that are updated)
    if (package.updates().size()) {
        description += "<p>" + i18n("Updates:") + "<br/>";
        QStringList updates;
        foreach (const Package &p, package.updates()) {
             updates += QString::fromUtf8("\xE2\x80\xA2 ") + p.name() + " - " + p.version();
        }
        description += updates.join("<br/>") + "</p>";
    }

    // Obsoletes (lists of packages that are obsoleted)
    if (package.obsoletes().size()) {
        description += "<p></b>" + i18n("Obsoletes:") + "</b><br/>";
        QStringList obsoletes;
        foreach (const Package &p, package.obsoletes()) {
             obsoletes += QString::fromUtf8("\xE2\x80\xA2 ") + p.name() + " - " + p.version();
        }
        description += obsoletes.join("<br>/") + "</p>";
    }

    // Repository (this is the repository the package comes from)
    if (!package.data().isEmpty()) {
         description += "<p>" + i18n("Repository:") + ' ' + package.data() + "</p>";
    }

    m_currentDescription = description;
    m_busySeq->stop();
}

QString UpdateDetails::getLinkList(const QString &links) const
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
        ret += QString::fromUtf8(" \xE2\x80\xA2 <a href=\"") + linkList.at(i) + "\">"
               + linkList.at(i + 1) + "</a>";
    }
    return ret;
}

void UpdateDetails::updateDetailFinished()
{
    if (descriptionKTB->document()->toPlainText().isEmpty()) {
        descriptionKTB->setPlainText(i18n("No update description was found."));
    }
}

#include "UpdateDetails.moc"
