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
#include <QPlainTextEdit>
#include <QTextDocument>
#include <KPixmapSequence>

#include <KIO/Job>

#include <QGraphicsDropShadowEffect>

Q_DECLARE_METATYPE(KPixmapSequenceOverlayPainter**)

KpkPackageDetails::KpkPackageDetails(QWidget *parent)
 : QWidget(parent),
   m_busySeqDetails(0),
   m_busySeqFiles(0),
   m_busySeqDepends(0),
   m_busySeqRequires(0)
{
    setupUi(this);
//     stackedWidget->hide();

    Enum::Roles roles = Client::instance()->actions();
    // Create a stacked layout to put the views in
//     m_viewLayout = new QStackedLayout(stackedWidget);

    // we check to see which roles are supported by the backend
    // if so we ask for information and create the containers
//     if (roles & Enum::RoleGetDetails) {
//         descriptionKTB = new KTextBrowser(this);
//         m_viewLayout->addWidget(descriptionKTB);
// //         descriptionTB->click();
//     } else {
//         descriptionTB->setEnabled(false);
//     }
// 
//     if (roles & Enum::RoleGetFiles) {
//         filesPTE = new QPlainTextEdit(this);
//         m_viewLayout->addWidget(filesPTE);
//         if (!m_viewLayout->count()) {
// //             fileListTB->click();
//         }
//     } else {
//         fileListTB->setEnabled(false);
//     }
// 
//     if (roles & Enum::RoleGetDepends) {
//         dependsOnLV = new QListView(this);
//         dependsOnLV->setModel(m_pkg_model_dep = new KpkSimplePackageModel(this));
//         m_viewLayout->addWidget(dependsOnLV);
//         if (!m_viewLayout->count()) {
// //             dependsOnTB->click();
//         }
//     } else {
//         dependsOnTB->setEnabled(false);
//     }
// 
//     if (roles & Enum::RoleGetRequires) {
//         requiredByLV = new QListView(this);
//         requiredByLV->setModel(m_pkg_model_req = new KpkSimplePackageModel(this));
//         m_viewLayout->addWidget(requiredByLV);
//         if (!m_viewLayout->count()) {
// //             requiredByTB->click();
//         }
//     } else {
//         requiredByTB->setEnabled(false);
//     }


        QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(screenshotL);
// //     QGraphicsDropShadowEffect *shadow2 = new QGraphicsDropShadowEffect(this);
// //     shadow->setColor(QColor(Qt::blue));
    shadow->setBlurRadius(15);
    shadow->setOffset(2);
// //     shadow2->setColor(QColor(Qt::blue));
// //     shadow2->setBlurRadius(15);
// //     shadow2->setOffset(2);
    screenshotL->setGraphicsEffect(shadow);
}

KpkPackageDetails::~KpkPackageDetails()
{
}

void KpkPackageDetails::setPackage(const QSharedPointer<PackageKit::Package> &package,
                                   const QModelIndex &index)
{
    m_package = package;

    QString pkgName = index.data(KpkPackageModel::IdRole).toString().split(';')[0];
//     summaryL->setText(index.data(KpkPackageModel::SummaryRole).toString());
    QString pkgIconPath   = index.data(KpkPackageModel::IconPathRole).toString();

    iconL->setPixmap(KpkIcons::getIcon(pkgIconPath, "package").pixmap(64, 64));

//     installPB->setIcon(KIcon(KIcon("go-down")));

    descriptionL->hide();
//     licenseL->hide();
    sizeL->hide();
    homepageL->hide();
//     descriptionTB->click();
    on_descriptionTB_clicked();

    m_tempFile = new KTemporaryFile;
    m_tempFile->setPrefix("appget");
    m_tempFile->setSuffix(".png");
    m_tempFile->open();

    KIO::FileCopyJob *job = KIO::file_copy("http://screenshots.debian.net/thumbnail/" + pkgName,
                               m_tempFile->fileName(), -1, KIO::Overwrite | KIO::HideProgressInfo);
    connect(job, SIGNAL(result(KJob *)),
            this, SLOT(resultJob(KJob *)));
}

void KpkPackageDetails::resultJob(KJob *job)
{
    if (!job->error()) {
        screenshotL->setPixmap(QPixmap(m_tempFile->fileName()));
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

void KpkPackageDetails::description(QSharedPointer<PackageKit::Package> p)
{
    if (m_busySeqDetails) {
        m_busySeqDetails->stop();
    }

    //format and show description
    Package::Details *details = p->details();

    if (!details->description().isEmpty()) {
        descriptionL->setText(details->description().replace('\n', "<br>"));
        descriptionL->show();
    }

    if (!details->url().isEmpty()) {
        homepageL->setText("<a href=\"" + details->url() + "\">" +
                           details->url() + "</a>");
        homepageL->show();
    }

    if (!details->license().isEmpty() && details->license() != "unknown") {
//         licenseL->setText(details->license());
//         licenseL->show();
    }

    if (details->group() != Enum::UnknownGroup) {
//         description += "<tr><td align=\"right\"><b>" + i18nc("Group of the package", "Group") + ":</b></td><td>"
//                     + KpkStrings::groups(details->group())
//                     + "</td></tr>";
    }

    if (details->size() > 0) {
        sizeL->setText(KGlobal::locale()->formatByteSize(details->size()));
        sizeL->show();
    }
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
    if (!m_busySeqDetails) {
        if (m_package->hasDetails()) {
            description(m_package);
            return;
        }

        // create the description transaction
        Transaction *t = Client::instance()->getDetails(m_package);
        if (t->error()) {
            KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
        } else {
//             setupSequence(t, &m_busySeqDetails, descriptionKTB->viewport());
            connect(t, SIGNAL(details(QSharedPointer<PackageKit::Package>)),
                    this, SLOT(description(QSharedPointer<PackageKit::Package>)));
        }
    }
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
            connect(t, SIGNAL(files(QSharedPointer<PackageKit::Package>, const QStringList &)),
                    this, SLOT(files(QSharedPointer<PackageKit::Package>, const QStringList &)));
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
