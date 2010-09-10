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

#include "KpkPackageDetails.h"

#include <KpkPackageModel.h>
#include <KpkSimplePackageModel.h>
#include <KpkStrings.h>
#include <KpkIcons.h>

#include <KMessageBox>

#include <KPixmapSequence>
#include <QTextDocument>
#include <QPlainTextEdit>
#include <QPainter>

#include <KIO/Job>
#include <KMenu>

#include <KDebug>

#include <QGraphicsDropShadowEffect>
#include "GraphicsOpacityDropShadowEffect.h"

#define BLUR_RADIUS 15

Q_DECLARE_METATYPE(KPixmapSequenceOverlayPainter**)

KpkPackageDetails::KpkPackageDetails(QWidget *parent)
 : QWidget(parent),
   m_busySeqDetails(0),
   m_busySeqFiles(0),
   m_busySeqDepends(0),
   m_busySeqRequires(0),
   m_display(false),
   m_transaction(0),
   m_hasDetails(false)
{
    setupUi(this);
//     stackedWidget->hide();

    Enum::Roles roles = Client::instance()->actions();
    // Create a stacked layout to put the views in
    m_viewLayout = new QStackedLayout(stackedWidget);

    KMenu *menu = new KMenu(i18n("Display"), this);
    m_actionGroup = new QActionGroup(this);
    QAction *action = 0;

    // we check to see which roles are supported by the backend
    // if so we ask for information and create the containers
    if (roles & Enum::RoleGetFiles) {
        action = menu->addAction(i18n("File List"));
        action->setCheckable(true);
        action->setData(Enum::RoleGetFiles);
        m_actionGroup->addAction(action);
        filesPTE = new QPlainTextEdit(this);
        m_viewLayout->addWidget(filesPTE);
    }

    if (roles & Enum::RoleGetRequires) {
        action = menu->addAction(i18n("Required By"));
        action->setCheckable(true);
        action->setData(Enum::RoleGetRequires);
        m_actionGroup->addAction(action);
        requiredByLV = new QListView(this);
        requiredByLV->setModel(m_pkg_model_req = new KpkSimplePackageModel(this));
        m_viewLayout->addWidget(requiredByLV);
    }

    if (roles & Enum::RoleGetDepends) {
        action = menu->addAction(i18n("Depends On"));
        action->setCheckable(true);
        action->setData(Enum::RoleGetDepends);
        m_actionGroup->addAction(action);
        dependsOnLV = new QListView(this);
        dependsOnLV->setModel(m_pkg_model_dep = new KpkSimplePackageModel(this));
        m_viewLayout->addWidget(dependsOnLV);
    }

    if (roles & Enum::RoleGetDetails) {
        action = menu->addAction(i18n("Details"));
        action->setCheckable(true);
        action->setData(Enum::RoleGetDetails);
        m_actionGroup->addAction(action);
        m_viewLayout->addWidget(descriptionW);
        descriptionW->setWidgetResizable(true);
        action->setChecked(true);
    }

    if (action){
        action->setChecked(true);
        connect(m_actionGroup, SIGNAL(triggered(QAction *)),
                this, SLOT(actionActivated(QAction *)));
        // Set the menu
        menuTB->setMenu(menu);
    }

    m_busySeqDetails = new KPixmapSequenceOverlayPainter(this);
    m_busySeqDetails->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeqDetails->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeqDetails->setWidget(stackedWidget);

    // Setup the opacit effect that makes the descriptio transparent
    // after finished it checks in display() to see if it shouldn't show
    // up again. The property animation is always the same, the only different thing
    // is the the Forward or Backward property
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(stackedWidget);
    effect->setOpacity(0);
    stackedWidget->setGraphicsEffect(effect);
    m_fadeDetails = new QPropertyAnimation(effect, "opacity");
    m_fadeDetails->setDuration(500);
    m_fadeDetails->setStartValue(qreal(0));
    m_fadeDetails->setEndValue(qreal(1));
    connect(m_fadeDetails, SIGNAL(finished()), this, SLOT(display()));


    // It's is impossible due to some limitation in Qt to set two effects on the same
    // Widget
    m_fadeScreenshot = new QPropertyAnimation(effect, "opacity");
//

    GraphicsOpacityDropShadowEffect *shadow = new GraphicsOpacityDropShadowEffect(screenshotL);
    shadow->setOpacity(0);
// //     QGraphicsDropShadowEffect *shadow2 = new QGraphicsDropShadowEffect(this);
// //     shadow->setColor(QColor(Qt::blue));
    shadow->setBlurRadius(BLUR_RADIUS);
    shadow->setOffset(2);
// //     shadow2->setColor(QColor(Qt::blue));
// //     shadow2->setBlurRadius(15);
// //     shadow2->setOffset(2);
    screenshotL->setGraphicsEffect(shadow);

    m_fadeScreenshot = new QPropertyAnimation(shadow, "opacity");
    m_fadeScreenshot->setDuration(500);
    m_fadeScreenshot->setStartValue(qreal(0));
    m_fadeScreenshot->setEndValue(qreal(1));
    connect(m_fadeScreenshot, SIGNAL(finished()), this, SLOT(display()));

}

KpkPackageDetails::~KpkPackageDetails()
{
}

void KpkPackageDetails::setDisplayDetails(bool value)
{
    m_display = value;
    display();
}

void KpkPackageDetails::setPackage(const QSharedPointer<PackageKit::Package> &package,
                                   const QModelIndex &index)
{
    if (!m_package.isNull() &&
        m_package->id() == package->id()) {
        return;
    } else if (m_display) {
        fadeOut();
    }
    m_package       = package;
    m_hasDetails    = false;
//     m_hasScreenshot = false;

    QString pkgName     = index.data(KpkPackageModel::IdRole).toString().split(';')[0];
    QString pkgIconPath = index.data(KpkPackageModel::IconPathRole).toString();
    m_currentIcon       = KpkIcons::getIcon(pkgIconPath, "package").pixmap(64, 64);

    // disconnect the transaction
    // so that we don't get old descriptions
    if (m_transaction) {
        disconnect(m_transaction, SIGNAL(details(const QSharedPointer<PackageKit::Package> &)),
                this, SLOT(description(const QSharedPointer<PackageKit::Package> &)));
        m_transaction = 0;
    }

    //
    m_currentScreenshot = "http://screenshots.debian.net/thumbnail/" + pkgName;
    if (m_screenshotPath.contains(m_currentScreenshot)) {
        display();
    } else{
        m_tempFile = new KTemporaryFile;
        m_tempFile->setPrefix("appget");
        m_tempFile->setSuffix(".png");
        m_tempFile->open();
        KIO::FileCopyJob *job = KIO::file_copy(m_currentScreenshot,
                                               m_tempFile->fileName(),
                                               -1,
                                               KIO::Overwrite | KIO::HideProgressInfo);
        connect(job, SIGNAL(result(KJob *)),
                this, SLOT(resultJob(KJob *)));
    }

    if (m_actionGroup->checkedAction()) {
        actionActivated(m_actionGroup->checkedAction());
    }

    // create the description transaction
//     m_transaction = Client::instance()->getDetails(m_package);
//     if (m_transaction->error()) {
//         KMessageBox::sorry(this, KpkStrings::daemonError(m_transaction->error()));
//     } else {
//         m_busySeqDetails->start();
//         connect(m_transaction, SIGNAL(details(const QSharedPointer<PackageKit::Package> &)),
//                 this, SLOT(description(const QSharedPointer<PackageKit::Package> &)));
//     }
}

void KpkPackageDetails::actionActivated(QAction *action)
{
    // Check to see if we don't already have package details
    if (action->data().toUInt() == Enum::RoleGetDetails &&
        m_package->hasDetails()) {
        description(m_package);
        return;
    }

    m_transaction = new Transaction(QString(), this);
    switch (action->data().toUInt()) {
    case Enum::RoleGetDetails:
        connect(m_transaction, SIGNAL(details(const QSharedPointer<PackageKit::Package> &)),
                this, SLOT(description(const QSharedPointer<PackageKit::Package> &)));
        m_transaction->getDetails(m_package);
        break;
    case Enum::RoleGetDepends:
        connect(m_transaction, SIGNAL(package(const QSharedPointer<PackageKit::Package> &)),
                m_pkg_model_dep, SLOT(addPackage(const QSharedPointer<PackageKit::Package> &)));
        m_transaction->getDepends(m_package, PackageKit::Enum::NoFilter, false);
        break;
    case Enum::RoleGetRequires:
        connect(m_transaction, SIGNAL(package(const QSharedPointer<PackageKit::Package> &)),
                m_pkg_model_req, SLOT(addPackage(const QSharedPointer<PackageKit::Package> &)));
        m_transaction->getRequires(m_package, PackageKit::Enum::NoFilter, false);
        break;
    case Enum::RoleGetFiles:
        connect(m_transaction, SIGNAL(files(const QSharedPointer<PackageKit::Package> &, const QStringList &)),
                this, SLOT(files(const QSharedPointer<PackageKit::Package> &, const QStringList &)));
        m_transaction->getFiles(m_package);
        break;
    }

    if (m_transaction->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(m_transaction->error()));
    } else {
        m_busySeqDetails->start();
        connect(m_transaction, SIGNAL(details(const QSharedPointer<PackageKit::Package> &)),
                this, SLOT(description(const QSharedPointer<PackageKit::Package> &)));
    }
}

void KpkPackageDetails::resultJob(KJob *job)
{
    kDebug();
    KIO::FileCopyJob *fJob = qobject_cast<KIO::FileCopyJob*>(job);
    if (!fJob->error()) {
        kDebug() << fJob->srcUrl() << fJob->destUrl();
        kDebug() << fJob->srcUrl().url() << fJob->destUrl().toLocalFile();


        m_screenshotPath[fJob->srcUrl().url()] = fJob->destUrl().toLocalFile();
//         m_currentScreenshot = QPixmap(m_tempFile->fileName()).scaled(160,120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
//         m_hasScreenshot = true;
        kDebug() << "m_fadeScreenshot" << m_tempFile->fileName();

        display();
    }
}

void KpkPackageDetails::fadeOut()
{
    kDebug();
    // Fade out only if needed
    if (m_fadeDetails->currentValue().toReal() != 0) {
        m_fadeDetails->setDirection(QAbstractAnimation::Backward);
        m_fadeDetails->start();
    }

    // Fade out the screenshot only if needed
    if (m_fadeScreenshot->currentValue().toReal() != 0) {
        m_fadeScreenshot->setDirection(QAbstractAnimation::Backward);
        m_fadeScreenshot->start();
    }
}

void KpkPackageDetails::display()
{
    kDebug() << "m_fadeDetails" << m_fadeDetails->currentValue().toReal() << m_hasDetails << m_display;
    kDebug() << "m_fadeScreenshot" << m_fadeScreenshot->currentValue().toReal() << m_screenshotPath.contains(m_currentScreenshot) << m_display;
    // If we shouldn't be showing fade out
    if (!m_display) {
        fadeOut();
    } else {
        // Check to see if we have details and if we are
        // transparent
        if (m_fadeDetails->currentValue().toReal() == 0 &&
            m_hasDetails) {
            setupDescription();
            // Fade In
            m_fadeDetails->setDirection(QAbstractAnimation::Forward);
            m_fadeDetails->start();
        }

        // Check to see if we have a screen shot and if we are
        // transparent, and make shure the details are going
        // to be shown
        if (m_fadeScreenshot->currentValue().toReal() == 0 &&
            m_screenshotPath.contains(m_currentScreenshot) &&
            m_fadeDetails->direction() == QAbstractAnimation::Forward) {
            QPixmap pixmap;
            pixmap = QPixmap(m_screenshotPath[m_currentScreenshot])
                             .scaled(160,120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            screenshotL->setPixmap(pixmap);
            // Fade In
            m_fadeScreenshot->setDirection(QAbstractAnimation::Forward);
            m_fadeScreenshot->start();
        }
    }
}

void KpkPackageDetails::setupSequence(Transaction *transaction,
                                      KPixmapSequenceOverlayPainter **sequence,
                                      QWidget *widget)
{
    // setup the busy widget
    KPixmapSequenceOverlayPainter *seq = new KPixmapSequenceOverlayPainter(this);
    seq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    seq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    seq->setWidget(widget);
    seq->start();
    *sequence = seq;
    transaction->setProperty("BusySequence", qVariantFromValue(&*sequence));
    connect(transaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            this, SLOT(finished(PackageKit::Enum::Exit)));
}

void KpkPackageDetails::setupDescription()
{
    m_viewLayout->setCurrentWidget(descriptionW);
    //format and show description
    Package::Details *details = m_package->details();

    if (!details->description().isEmpty()) {
        descriptionL->setText(details->description().replace('\n', "<br>"));
        descriptionL->show();
    } else {
        descriptionL->clear();
    }

    if (!details->url().isEmpty()) {
        homepageL->setText("<a href=\"" + details->url() + "\">" +
                           details->url() + "</a>");
        homepageL->show();
    } else {
        homepageL->clear();
    }

    if (!details->license().isEmpty() && details->license() != "unknown") {
        licenseL->setText(details->license());
        licenseL->show();
    } else {
        licenseL->clear();
    }

//     if (details->group() != Enum::UnknownGroup) {
// //         description += "<tr><td align=\"right\"><b>" + i18nc("Group of the package", "Group") + ":</b></td><td>"
// //                     + KpkStrings::groups(details->group())
// //                     + "</td></tr>";
//     }

    if (details->size() > 0) {
        sizeL->setText(KGlobal::locale()->formatByteSize(details->size()));
        sizeL->show();
    } else {
        sizeL->clear();
    }

    iconL->setPixmap(m_currentIcon);
}

void KpkPackageDetails::description(const QSharedPointer<PackageKit::Package> &package)
{
    if (m_busySeqDetails) {
        m_busySeqDetails->stop();
    }

    m_package     = package;
    m_hasDetails  = true;
    m_transaction = 0;

    display();
}

void KpkPackageDetails::finished(PackageKit::Enum::Exit status)
{
    KPixmapSequenceOverlayPainter **sequence;
    sequence = sender()->property("BusySequence").value<KPixmapSequenceOverlayPainter**>();
    delete *sequence;
    if (status != Enum::ExitSuccess) {
        *sequence = 0;
    }
}

void KpkPackageDetails::on_descriptionTB_clicked()
{
//     m_viewLayout->setCurrentWidget(descriptionKTB);
//     if (!m_busySeqDetails) {
//         // disconnect the transaction
//         // so that we don't get old descriptions
//         if (m_transaction) {
//             disconnect(m_transaction, SIGNAL(details(const QSharedPointer<PackageKit::Package> &)),
//                     this, SLOT(description(const QSharedPointer<PackageKit::Package> &)));
//             m_transaction = 0;
//         }
//
//         // Check to see if we don't already have package details
//         if (m_package->hasDetails()) {
//             description(m_package);
//             return;
//         }
//
//         // create the description transaction
//         m_transaction = Client::instance()->getDetails(m_package);
//         if (m_transaction->error()) {
//             KMessageBox::sorry(this, KpkStrings::daemonError(m_transaction->error()));
//         } else {
//             setupSequence(m_transaction, &m_busySeqDetails, this);
//             connect(m_transaction, SIGNAL(details(const QSharedPointer<PackageKit::Package> &)),
//                     this, SLOT(description(const QSharedPointer<PackageKit::Package> &)));
//         }
//     }
}

void KpkPackageDetails::files(QSharedPointer<PackageKit::Package> package, const QStringList &files)
{
    Q_UNUSED(package)
    m_busySeqFiles->stop();
    filesPTE->clear();
    for (int i = 0; i < files.size(); ++i) {
        filesPTE->appendPlainText(files.at(i));
    }
}

void KpkPackageDetails::on_fileListTB_clicked()
{
    m_viewLayout->setCurrentWidget(filesPTE);
    if (!m_busySeqFiles) {
        // create the files transaction
        Transaction *t = Client::instance()->getFiles(m_package);
        if (t->error()) {
            KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
        } else {
            setupSequence(t, &m_busySeqFiles, filesPTE->viewport());
            connect(t, SIGNAL(files(const QSharedPointer<PackageKit::Package> &, const QStringList &)),
                    this, SLOT(files(const QSharedPointer<PackageKit::Package> &, const QStringList &)));
        }
    }
}

void KpkPackageDetails::on_dependsOnTB_clicked()
{
    m_viewLayout->setCurrentWidget(dependsOnLV);
    if (!m_busySeqDepends) {
        // create a transaction for the dependecies not recursive
        Transaction *t = Client::instance()->getDepends(m_package, PackageKit::Enum::NoFilter, false);
        m_pkg_model_dep->clear();
        if (t->error()) {
            KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
        } else {
            setupSequence(t, &m_busySeqDepends, dependsOnLV->viewport());
            connect(t, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                    m_busySeqDepends, SLOT(stop()));
            connect(t, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                    m_pkg_model_dep, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
        }
    }
}

void KpkPackageDetails::on_requiredByTB_clicked()
{
    m_viewLayout->setCurrentWidget(requiredByLV);
    if (!m_busySeqRequires) {
         // create a transaction for the requirements not recursive
        Transaction *t = Client::instance()->getRequires(m_package, PackageKit::Enum::NoFilter, false);
        m_pkg_model_req->clear();
        if (t->error()) {
            KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
        } else {
            setupSequence(t, &m_busySeqRequires, requiredByLV->viewport());
            connect(t, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                    m_busySeqRequires, SLOT(stop()));
            connect(t, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                    m_pkg_model_req, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
        }
    }
}

#include "KpkPackageDetails.moc"
