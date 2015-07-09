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

#include <Daemon>
#include <PkStrings.h>

#include <KMessageBox>
#include <KPixmapSequence>
#include <KLocalizedString>
#include <KGlobal>

#include <QAbstractAnimation>
#include <QGraphicsOpacityEffect>
#include <QStringBuilder>
#include <KIconLoader>

#include <Transaction>

#include <KDebug>

#define FINAL_HEIGHT 160

UpdateDetails::UpdateDetails(QWidget *parent)
 : QWidget(parent),
   m_show(false),
   m_transaction(0)
{
    setupUi(this);
    hideTB->setIcon(QIcon::fromTheme("window-close"));
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

void UpdateDetails::setPackage(const QString &packageId, Transaction::Info updateInfo)
{
    if (m_packageId == packageId) {
        return;
    }
    m_show       = true;
    m_packageId  = packageId;
    m_updateInfo = updateInfo;
    m_currentDescription.clear();
    if (m_transaction) {
        disconnect(m_transaction, SIGNAL(updateDetail(QString,QStringList,QStringList,QStringList,QStringList,QStringList,PackageKit::Transaction::Restart,QString,QString,PackageKit::Transaction::UpdateState,QDateTime,QDateTime)),
                   this, SLOT(updateDetail(QString,QStringList,QStringList,QStringList,QStringList,QStringList,PackageKit::Transaction::Restart,QString,QString,PackageKit::Transaction::UpdateState,QDateTime,QDateTime)));
        disconnect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   this, SLOT(display()));
    }

    m_transaction = Daemon::getUpdateDetail(m_packageId);
    connect(m_transaction, SIGNAL(updateDetail(QString,QStringList,QStringList,QStringList,QStringList,QStringList,PackageKit::Transaction::Restart,QString,QString,PackageKit::Transaction::UpdateState,QDateTime,QDateTime)),
            this, SLOT(updateDetail(QString,QStringList,QStringList,QStringList,QStringList,QStringList,PackageKit::Transaction::Restart,QString,QString,PackageKit::Transaction::UpdateState,QDateTime,QDateTime)));
    connect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(display()));

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
    kDebug() << sender();

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
    } else if (m_currentDescription.isEmpty()) {
        updateDetailFinished();
    }
}

void UpdateDetails::updateDetail(const QString &packageID,
                                 const QStringList &updates,
                                 const QStringList &obsoletes,
                                 const QStringList &vendorUrls,
                                 const QStringList &bugzillaUrls,
                                 const QStringList &cveUrls,
                                 PackageKit::Transaction::Restart restart,
                                 const QString &updateText,
                                 const QString &changelog,
                                 PackageKit::Transaction::UpdateState state,
                                 const QDateTime &issued,
                                 const QDateTime &updated)
{
    //format and show description
    QString description;

    // update type (ie Security Update)
    if (m_updateInfo == Transaction::InfoEnhancement) {
        description += "<p>" +
                       i18n("This update will add new features and expand functionality.") +
                       "</p>";
    } else if (m_updateInfo == Transaction::InfoBugfix) {
        description += "<p>" +
                       i18n("This update will fix bugs and other non-critical problems.") +
                       "</p>";
    } else if (m_updateInfo == Transaction::InfoImportant) {
        description += "<p>" +
                       i18n("This update is important as it may solve critical problems.") +
                       "</p>";
    } else if (m_updateInfo == Transaction::InfoSecurity) {
        description += "<p>" +
                       i18n("This update is needed to fix a security vulnerability with this package.") +
                       "</p>";
    } else if (m_updateInfo == Transaction::InfoBlocked) {
        description += "<p>" +
                       i18n("This update is blocked.") +
                       "</p>";
    }

    // Issued and Updated
    if (!issued.isNull() && !updated.isNull()) {
        description += "<p>" +
                       i18n("This notification was issued on %1 and last updated on %2.",
                            KGlobal::locale()->formatDateTime(issued, KLocale::ShortDate),
                            KGlobal::locale()->formatDateTime(updated, KLocale::ShortDate)) +
                       "</p>";
    } else if (!issued.isNull()) {
        description += "<p>" +
                       i18n("This notification was issued on %1.",
                            KGlobal::locale()->formatDateTime(issued, KLocale::ShortDate)) +
                       "</p>";
    }

    // Description
    if (!updateText.isEmpty()) {
        QString _updateText = updateText;
        _updateText.replace('\n', "<br/>");
        _updateText.replace(' ', "&nbsp;");
        description += "<p>" +
                       _updateText +
                       "</p>";
    }

    // links
    //  Vendor
    if (!vendorUrls.isEmpty()) {
        description += "<p>" +
                       i18np("For more information about this update please visit this website:",
                             "For more information about this update please visit these websites:",
                             vendorUrls.size()) + "<br/>" +
                       getLinkList(vendorUrls) +
                       "</p>";
    }

    //  Bugzilla
    if (!bugzillaUrls.isEmpty()) {
        description += "<p>" +
                       i18np("For more information about bugs fixed by this update please visit this website:",
                             "For more information about bugs fixed by this update please visit these websites:",
                             bugzillaUrls.size()) + "<br/>" +
                       getLinkList(bugzillaUrls) +
                       "</p>";
    }

    //  CVE
    if (!cveUrls.isEmpty()) {
        description += "<p>" +
                       i18np("For more information about this security update please visit this website:",
                             "For more information about this security update please visit these websites:",
                             cveUrls.size()) + "<br/>" +
                       getLinkList(cveUrls) +
                       "</p>";
    }

    // Notice (about the need for a reboot)
    if (restart == Transaction::RestartSystem) {
        description += "<p>" +
                       i18n("The computer will have to be restarted after the update for the changes to take effect.") +
                       "</p>";
    } else if (restart == Transaction::RestartSession) {
        description += "<p>" +
                       i18n("You will need to log out and back in after the update for the changes to take effect.") +
                       "</p>";
    }

    // State
    if (state == Transaction::UpdateStateUnstable) {
        description += "<p>" +
                       i18n("The classification of this update is unstable which means it is not designed for production use.") +
                       "</p>";
    } else if (state == Transaction::UpdateStateTesting) {
        description += "<p>" +
                       i18n("This is a test update, and is not designed for normal use. Please report any problems or regressions you encounter.") +
                       "</p>";
    }

    // only show changelog if we didn't have any update text
    if (updateText.isEmpty() && !changelog.isEmpty()) {
        QString _changelog = changelog;
        _changelog.replace('\n', "<br/>");
        _changelog.replace(' ', "&nbsp;");
        description += "<p>" +
                       i18n("The developer logs will be shown as no description is available for this update:") +
                       "<br/>" +
                       _changelog +
                       "</p>";
    }

    // Updates (lists of packages that are updated)
    if (!updates.isEmpty()) {
        description += "<p>" + i18n("Updates:") + "<br/>";
        QStringList _updates;
        foreach (const QString &pid, updates) {
             _updates += QString::fromUtf8("\xE2\x80\xA2 ") + Transaction::packageName(pid) + " - " + Transaction::packageVersion(pid);
        }
        description += _updates.join("<br/>") + "</p>";
    }

    // Obsoletes (lists of packages that are obsoleted)
    if (obsoletes.size()) {
        description += "<p></b>" + i18n("Obsoletes:") + "</b><br/>";
        QStringList _obsoletes;
        foreach (const QString &pid, obsoletes) {
             _obsoletes += QString::fromUtf8("\xE2\x80\xA2 ") + Transaction::packageName(pid) + " - " + Transaction::packageVersion(pid);
        }
        description += _obsoletes.join("<br>/") + "</p>";
    }

    // Repository (this is the repository the package comes from)
    if (!Transaction::packageData(packageID).isEmpty()) {
         description += "<p>" + i18n("Repository: %1", Transaction::packageData(packageID)) + "</p>";
    }

    m_currentDescription = description;
    m_busySeq->stop();
}

QString UpdateDetails::getLinkList(const QStringList &urls) const
{
    QString ret;
    foreach (const QString &url, urls) {
        if (!ret.isEmpty()) {
            ret += "<br/>";
        }
        ret += QString::fromUtf8(" \xE2\x80\xA2 <a href=\"") % url % QLatin1String("\">") % url % QLatin1String("</a>");
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
