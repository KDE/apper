/***************************************************************************
 *   Copyright (C) 2009-2018 by Daniel Nicoletti                           *
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
#include <KFormat>

#include <QAbstractAnimation>
#include <QGraphicsOpacityEffect>
#include <QStringBuilder>
#include <KIconLoader>

#include <Transaction>

#include <QLoggingCategory>

#define FINAL_HEIGHT 160

Q_DECLARE_LOGGING_CATEGORY(APPER)

UpdateDetails::UpdateDetails(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    hideTB->setIcon(QIcon::fromTheme(QLatin1String("window-close")));
    connect(hideTB, &QToolButton::clicked, this, &UpdateDetails::hide);

    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KIconLoader::global()->loadPixmapSequence(QLatin1String("process-working"), KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(this);

    QWidget *actionsViewport = descriptionKTB->viewport();
    QPalette palette = actionsViewport->palette();
    palette.setColor(actionsViewport->backgroundRole(), Qt::transparent);
    palette.setColor(actionsViewport->foregroundRole(), palette.color(QPalette::WindowText));
    actionsViewport->setPalette(palette);

    auto effect = new QGraphicsOpacityEffect(descriptionKTB);
    effect->setOpacity(0);
    descriptionKTB->setGraphicsEffect(effect);
    m_fadeDetails = new QPropertyAnimation(effect, "opacity", this);
    m_fadeDetails->setDuration(500);
    m_fadeDetails->setStartValue(qreal(0));
    m_fadeDetails->setEndValue(qreal(1));
    connect(m_fadeDetails, &QPropertyAnimation::finished, this, &UpdateDetails::display);


    auto anim1 = new QPropertyAnimation(this, "maximumSize", this);
    anim1->setDuration(500);
    anim1->setEasingCurve(QEasingCurve::OutQuart);
    anim1->setStartValue(QSize(QWIDGETSIZE_MAX, 0));
    anim1->setEndValue(QSize(QWIDGETSIZE_MAX, FINAL_HEIGHT));
    auto anim2 = new QPropertyAnimation(this, "minimumSize", this);
    anim2->setDuration(500);
    anim2->setEasingCurve(QEasingCurve::OutQuart);
    anim2->setStartValue(QSize(QWIDGETSIZE_MAX, 0));
    anim2->setEndValue(QSize(QWIDGETSIZE_MAX, FINAL_HEIGHT));

    m_expandPanel = new QParallelAnimationGroup(this);
    m_expandPanel->addAnimation(anim1);
    m_expandPanel->addAnimation(anim2);
    connect(m_expandPanel, &QParallelAnimationGroup::finished, this, &UpdateDetails::display);

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
        disconnect(m_transaction, &Transaction::updateDetail, this, &UpdateDetails::updateDetail);
        disconnect(m_transaction, &Transaction::finished, this, &UpdateDetails::display);
    }

    m_transaction = Daemon::getUpdateDetail(m_packageId);
    connect(m_transaction, &Transaction::updateDetail, this, &UpdateDetails::updateDetail);
    connect(m_transaction, &Transaction::finished, this, &UpdateDetails::display);

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
    qCDebug(APPER) << sender();

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
        description += QLatin1String("<p>") +
                       i18n("This update will add new features and expand functionality.") +
                       QLatin1String("</p>");
    } else if (m_updateInfo == Transaction::InfoBugfix) {
        description += QLatin1String("<p>") +
                       i18n("This update will fix bugs and other non-critical problems.") +
                       QLatin1String("</p>");
    } else if (m_updateInfo == Transaction::InfoImportant) {
        description += QLatin1String("<p>") +
                       i18n("This update is important as it may solve critical problems.") +
                       QLatin1String("</p>");
    } else if (m_updateInfo == Transaction::InfoSecurity) {
        description += QLatin1String("<p>") +
                       i18n("This update is needed to fix a security vulnerability with this package.") +
                       QLatin1String("</p>");
    } else if (m_updateInfo == Transaction::InfoBlocked) {
        description += QLatin1String("<p>") +
                       i18n("This update is blocked.") +
                       QLatin1String("</p>");
    }

    // Issued and Updated
    if (!issued.isNull() && !updated.isNull()) {
        description += QLatin1String("<p>") +
                       i18n("This notification was issued on %1 and last updated on %2.",
                            QLocale::system().toString(issued, QLocale::ShortFormat),
                            QLocale::system().toString(updated, QLocale::ShortFormat)) +
                       QLatin1String("</p>");
    } else if (!issued.isNull()) {
        description += QLatin1String("<p>") +
                       i18n("This notification was issued on %1.",
                            QLocale::system().toString(issued, QLocale::ShortFormat)) +
                       QLatin1String("</p>");
    }

    // Description
    if (!updateText.isEmpty()) {
        QString _updateText = updateText;
        _updateText.replace(QLatin1Char('\n'), QLatin1String("<br/>"));
        _updateText.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));
        description += QLatin1String("<p>") + _updateText + QLatin1String("</p>");
    }

    // links
    //  Vendor
    if (!vendorUrls.isEmpty()) {
        description += QLatin1String("<p>") +
                       i18np("For more information about this update please visit this website:",
                             "For more information about this update please visit these websites:",
                             vendorUrls.size()) + QLatin1String("<br/>") +
                       getLinkList(vendorUrls) +
                       QLatin1String("</p>");
    }

    //  Bugzilla
    if (!bugzillaUrls.isEmpty()) {
        description += QLatin1String("<p>") +
                       i18np("For more information about bugs fixed by this update please visit this website:",
                             "For more information about bugs fixed by this update please visit these websites:",
                             bugzillaUrls.size()) + QLatin1String("<br>") +
                       getLinkList(bugzillaUrls) +
                       QLatin1String("</p>");
    }

    //  CVE
    if (!cveUrls.isEmpty()) {
        description += QLatin1String("<p>") +
                       i18np("For more information about this security update please visit this website:",
                             "For more information about this security update please visit these websites:",
                             cveUrls.size()) + QLatin1String("<br>") +
                       getLinkList(cveUrls) +
                       QLatin1String("</p>");
    }

    // Notice (about the need for a reboot)
    if (restart == Transaction::RestartSystem) {
        description += QLatin1String("<p>") +
                       i18n("The computer will have to be restarted after the update for the changes to take effect.") +
                       QLatin1String("</p>");
    } else if (restart == Transaction::RestartSession) {
        description += QLatin1String("<p>") +
                       i18n("You will need to log out and back in after the update for the changes to take effect.") +
                       QLatin1String("</p>");
    }

    // State
    if (state == Transaction::UpdateStateUnstable) {
        description += QLatin1String("<p>") +
                       i18n("The classification of this update is unstable which means it is not designed for production use.") +
                       QLatin1String("</p>");
    } else if (state == Transaction::UpdateStateTesting) {
        description += QLatin1String("<p>") +
                       i18n("This is a test update, and is not designed for normal use. Please report any problems or regressions you encounter.") +
                       QLatin1String("</p>");
    }

    // only show changelog if we didn't have any update text
    if (updateText.isEmpty() && !changelog.isEmpty()) {
        QString _changelog = changelog;
        _changelog.replace(QLatin1Char('\n'), QLatin1String("<br/>"));
        _changelog.replace(QLatin1Char(' '), QLatin1String("&nbsp;"));
        description += QLatin1String("<p>") +
                       i18n("The developer logs will be shown as no description is available for this update:") +
                       QLatin1String("<br>") +
                       _changelog +
                       QLatin1String("</p>");
    }

    // Updates (lists of packages that are updated)
    if (!updates.isEmpty()) {
        description += QLatin1String("<p>") + i18n("Updates:") + QLatin1String("<br>");
        QStringList _updates;
        for (const QString &pid : updates) {
             _updates += QString::fromUtf8("\xE2\x80\xA2 ") + Transaction::packageName(pid) + QLatin1String(" - ") + Transaction::packageVersion(pid);
        }
        description += _updates.join(QLatin1String("<br>")) + QLatin1String("</p>");
    }

    // Obsoletes (lists of packages that are obsoleted)
    if (obsoletes.size()) {
        description += QLatin1String("<p></b>") + i18n("Obsoletes:") + QLatin1String("</b><br/>");
        QStringList _obsoletes;
        for (const QString &pid : obsoletes) {
             _obsoletes += QString::fromUtf8("\xE2\x80\xA2 ") + Transaction::packageName(pid) + QLatin1String(" - ") + Transaction::packageVersion(pid);
        }
        description += _obsoletes.join(QLatin1String("<br>/")) + QLatin1String("</p>");
    }

    // Repository (this is the repository the package comes from)
    if (!Transaction::packageData(packageID).isEmpty()) {
         description += QLatin1String("<p>") + i18n("Repository: %1", Transaction::packageData(packageID)) + QLatin1String("</p>");
    }

    m_currentDescription = description;
    m_busySeq->stop();
}

QString UpdateDetails::getLinkList(const QStringList &urls) const
{
    QString ret;
    for (const QString &url : urls) {
        if (!ret.isEmpty()) {
            ret += QLatin1String("<br>");
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

#include "moc_UpdateDetails.cpp"
