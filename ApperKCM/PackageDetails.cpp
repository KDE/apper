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

#include "PackageDetails.h"
#include "ui_PackageDetails.h"

#include "ScreenShotViewer.h"

#include <PackageModel.h>
#include <PkStrings.h>
#include <PkIcons.h>

#include <KMessageBox>

#include <KService>
#include <KServiceGroup>
#include <KDesktopFile>
#include <KTemporaryFile>
#include <KPixmapSequence>
#include <QTextDocument>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QPainter>
#include <QAbstractAnimation>
#include <QStringBuilder>

#include <KIO/Job>
#include <KMenu>

#include <KDebug>

#include <Daemon>
#include <Transaction>

#include <config.h>

#ifdef HAVE_APPSTREAM
#include <AppStream/AppStreamDb.h>
#endif

#include "GraphicsOpacityDropShadowEffect.h"

#define BLUR_RADIUS 15
#define FINAL_HEIGHT 210

using namespace PackageKit;

Q_DECLARE_METATYPE(KPixmapSequenceOverlayPainter**)

PackageDetails::PackageDetails(QWidget *parent)
 : QWidget(parent),
   ui(new Ui::PackageDetails),
   m_busySeq(0),
   m_display(false),
   m_hideVersion(false),
   m_hideArch(false),
   m_transaction(0),
   m_hasDetails(false),
   m_hasFileList(false)
{
    ui->setupUi(this);
    connect(ui->hideTB, SIGNAL(clicked()), this, SLOT(hide()));
}

void PackageDetails::init(PackageKit::Transaction::Roles roles)
{
    kDebug();
    KMenu *menu = new KMenu(i18n("Display"), this);
    m_actionGroup = new QActionGroup(this);
    QAction *action = 0;

    // we check to see which roles are supported by the backend
    // if so we ask for information and create the containers
    if (roles & PackageKit::Transaction::RoleGetDetails) {
        action = menu->addAction(i18n("Description"));
        action->setCheckable(true);
        action->setData(PackageKit::Transaction::RoleGetDetails);
        m_actionGroup->addAction(action);
        ui->descriptionW->setWidgetResizable(true);
    }

    if (roles & PackageKit::Transaction::RoleGetDepends) {
        action = menu->addAction(i18n("Depends On"));
        action->setCheckable(true);
        action->setData(PackageKit::Transaction::RoleGetDepends);
        m_actionGroup->addAction(action);
        // Sets a transparent background
        QWidget *actionsViewport = ui->dependsOnLV->viewport();
        QPalette palette = actionsViewport->palette();
        palette.setColor(actionsViewport->backgroundRole(), Qt::transparent);
        palette.setColor(actionsViewport->foregroundRole(), palette.color(QPalette::WindowText));
        actionsViewport->setPalette(palette);

        m_dependsModel = new PackageModel(this);
        m_dependsProxy = new QSortFilterProxyModel(this);
        m_dependsProxy->setDynamicSortFilter(true);
        m_dependsProxy->setSortRole(PackageModel::SortRole);
        m_dependsProxy->setSourceModel(m_dependsModel);
        ui->dependsOnLV->setModel(m_dependsProxy);
        ui->dependsOnLV->sortByColumn(0, Qt::AscendingOrder);
        ui->dependsOnLV->header()->setDefaultAlignment(Qt::AlignCenter);
        ui->dependsOnLV->header()->setResizeMode(PackageModel::NameCol, QHeaderView::ResizeToContents);
        ui->dependsOnLV->header()->setResizeMode(PackageModel::VersionCol, QHeaderView::ResizeToContents);
        ui->dependsOnLV->header()->setResizeMode(PackageModel::ArchCol, QHeaderView::Stretch);
        ui->dependsOnLV->header()->hideSection(PackageModel::ActionCol);
        ui->dependsOnLV->header()->hideSection(PackageModel::CurrentVersionCol);
        ui->dependsOnLV->header()->hideSection(PackageModel::OriginCol);
        ui->dependsOnLV->header()->hideSection(PackageModel::SizeCol);
    }

    if (roles & PackageKit::Transaction::RoleGetRequires) {
        action = menu->addAction(i18n("Required By"));
        action->setCheckable(true);
        action->setData(PackageKit::Transaction::RoleGetRequires);
        m_actionGroup->addAction(action);
        // Sets a transparent background
        QWidget *actionsViewport = ui->requiredByLV->viewport();
        QPalette palette = actionsViewport->palette();
        palette.setColor(actionsViewport->backgroundRole(), Qt::transparent);
        palette.setColor(actionsViewport->foregroundRole(), palette.color(QPalette::WindowText));
        actionsViewport->setPalette(palette);

        m_requiresModel = new PackageModel(this);
        m_requiresProxy = new QSortFilterProxyModel(this);
        m_requiresProxy->setDynamicSortFilter(true);
        m_requiresProxy->setSortRole(PackageModel::SortRole);
        m_requiresProxy->setSourceModel(m_requiresModel);
        ui->requiredByLV->setModel(m_requiresProxy);
        ui->requiredByLV->sortByColumn(0, Qt::AscendingOrder);
        ui->requiredByLV->header()->setDefaultAlignment(Qt::AlignCenter);
        ui->requiredByLV->header()->setResizeMode(PackageModel::NameCol, QHeaderView::ResizeToContents);
        ui->requiredByLV->header()->setResizeMode(PackageModel::VersionCol, QHeaderView::ResizeToContents);
        ui->requiredByLV->header()->setResizeMode(PackageModel::ArchCol, QHeaderView::Stretch);
        ui->requiredByLV->header()->hideSection(PackageModel::ActionCol);
        ui->requiredByLV->header()->hideSection(PackageModel::CurrentVersionCol);
        ui->requiredByLV->header()->hideSection(PackageModel::OriginCol);
        ui->requiredByLV->header()->hideSection(PackageModel::SizeCol);
    }

    if (roles & PackageKit::Transaction::RoleGetFiles) {
        action = menu->addAction(i18n("File List"));
        action->setCheckable(true);
        action->setData(PackageKit::Transaction::RoleGetFiles);
        m_actionGroup->addAction(action);
        // Sets a transparent background
        QWidget *actionsViewport = ui->filesPTE->viewport();
        QPalette palette = actionsViewport->palette();
        palette.setColor(actionsViewport->backgroundRole(), Qt::transparent);
        palette.setColor(actionsViewport->foregroundRole(), palette.color(QPalette::WindowText));
        actionsViewport->setPalette(palette);
    }

    // Check to se if we have any action
    if (m_actionGroup->actions().isEmpty()) {
        ui->menuTB->hide();
    } else {
        action = m_actionGroup->actions().first();
        action->setChecked(true);
        connect(m_actionGroup, SIGNAL(triggered(QAction*)),
                this, SLOT(actionActivated(QAction*)));
        // Set the menu
        ui->menuTB->setMenu(menu);
        ui->menuTB->setIcon(KIcon("help-about"));
    }

    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(ui->stackedWidget);

    // Setup the opacit effect that makes the descriptio transparent
    // after finished it checks in display() to see if it shouldn't show
    // up again. The property animation is always the same, the only different thing
    // is the the Forward or Backward property
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(ui->stackedWidget);
    effect->setOpacity(0);
    ui->stackedWidget->setGraphicsEffect(effect);
    m_fadeStacked = new QPropertyAnimation(effect, "opacity", this);
    m_fadeStacked->setDuration(500);
    m_fadeStacked->setStartValue(qreal(0));
    m_fadeStacked->setEndValue(qreal(1));
    connect(m_fadeStacked, SIGNAL(finished()), this, SLOT(display()));

    // It's is impossible due to some limitation in Qt to set two effects on the same
    // Widget
    m_fadeScreenshot = new QPropertyAnimation(effect, "opacity", this);
    GraphicsOpacityDropShadowEffect *shadow = new GraphicsOpacityDropShadowEffect(ui->screenshotL);
    shadow->setOpacity(0);
    shadow->setBlurRadius(BLUR_RADIUS);
    shadow->setOffset(2);
    shadow->setColor(QApplication::palette().dark().color());
    ui->screenshotL->setGraphicsEffect(shadow);

    m_fadeScreenshot = new QPropertyAnimation(shadow, "opacity", this);
    m_fadeScreenshot->setDuration(500);
    m_fadeScreenshot->setStartValue(qreal(0));
    m_fadeScreenshot->setEndValue(qreal(1));
    connect(m_fadeScreenshot, SIGNAL(finished()), this, SLOT(display()));

    // This pannel expanding
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

PackageDetails::~PackageDetails()
{
}

void PackageDetails::setPackage(const QModelIndex &index)
{
    kDebug() << index;
    QString appId = index.data(PackageModel::ApplicationId).toString();
    QString packageID = index.data(PackageModel::IdRole).toString();

    // if it's the same package and the same application, return
    if (packageID == m_packageID && appId == m_appId) {
        return;
    } else if (maximumSize().height() == 0) {
        // Expand the panel
        m_display = true;
        m_expandPanel->setDirection(QAbstractAnimation::Forward);
        m_expandPanel->start();
    } else {
        // Hide the old description
        fadeOut(PackageDetails::FadeScreenshot | PackageDetails::FadeStacked);
    }

    m_index       = index;
    m_appId       = appId;
    m_packageID   = packageID;
    m_hasDetails  = false;
    m_hasFileList = false;
    m_hasRequires = false;
    m_hasDepends  = false;
    kDebug() << "appId" << appId << "m_package" << m_packageID;

    QString pkgIconPath = index.data(PackageModel::IconRole).toString();
    m_currentIcon       = PkIcons::getIcon(pkgIconPath, QString()).pixmap(64, 64);
    m_appName           = index.data(PackageModel::NameRole).toString();

#ifdef HAVE_APPSTREAM
    m_currentScreenshot = AppStreamDb::instance()->thumbnail(Transaction::packageName(m_packageID));
    kDebug() << "current screenshot" << m_currentScreenshot;
#endif
    if (!m_currentScreenshot.isEmpty()) {
        if (m_screenshotPath.contains(m_currentScreenshot)) {
            display();
        } else {
            KTemporaryFile *tempFile = new KTemporaryFile;
            tempFile->setPrefix("appget");
            tempFile->setSuffix(".png");
            tempFile->open();
            KIO::FileCopyJob *job = KIO::file_copy(m_currentScreenshot,
                                                   tempFile->fileName(),
                                                   -1,
                                                   KIO::Overwrite | KIO::HideProgressInfo);
            connect(job, SIGNAL(result(KJob*)),
                    this, SLOT(resultJob(KJob*)));
        }
    }

    if (m_actionGroup->checkedAction()) {
        actionActivated(m_actionGroup->checkedAction());
    }
}

void PackageDetails::on_screenshotL_clicked()
{
    kDebug();
#ifndef HAVE_APPSTREAM
    return;
#else
    QString screenshot;

    screenshot = AppStreamDb::instance()->screenshot(Transaction::packageName(m_packageID));
    if (screenshot.isEmpty()) {
        return;
    }

    ScreenShotViewer *view = new ScreenShotViewer(screenshot);
    view->setWindowTitle(m_appName);
    view->show();
#endif
}

void PackageDetails::hidePackageVersion(bool hide)
{
    m_hideVersion = hide;
}

void PackageDetails::hidePackageArch(bool hide)
{
    m_hideArch = hide;
}

void PackageDetails::actionActivated(QAction *action)
{
    // don't fade the screenshot
    // if the package changed setPackage() fades both
    fadeOut(FadeStacked);
    kDebug();

    // disconnect the transaction
    // so that we don't get old data
    if (m_transaction) {
        disconnect(m_transaction, SIGNAL(details(QString,QString,PackageKit::Transaction::Group,QString,QString,qulonglong)),
                   this, SLOT(description(QString,QString,PackageKit::Transaction::Group,QString,QString,qulonglong)));
        disconnect(m_transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                   m_dependsModel, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
        disconnect(m_transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                   m_requiresModel, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
        disconnect(m_transaction, SIGNAL(files(QString,QStringList)),
                   this, SLOT(files(QString,QStringList)));
        disconnect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   this, SLOT(finished()));
        m_transaction = 0;
    }

    // Check to see if we don't already have the required data
    uint role = action->data().toUInt();
    switch (role) {
    case PackageKit::Transaction::RoleGetDetails:
        if (m_hasDetails) {
            description(m_detailsPackageID,
                        m_detailsLicense,
                        m_detailsGroup,
                        m_detailsDetail,
                        m_detailsUrl,
                        m_detailsSize);
            display();
            return;
        }
        break;
    case PackageKit::Transaction::RoleGetDepends:
        if (m_hasDepends) {
            display();
            return;
        }
        break;
    case PackageKit::Transaction::RoleGetRequires:
        if (m_hasRequires) {
            display();
            return;
        }
        break;
    case PackageKit::Transaction::RoleGetFiles:
        if (m_hasFileList) {
            display();
            return;
        }
        break;
    }

    // we don't have the data
    m_transaction = new PackageKit::Transaction(this);
    kDebug() << "New transaction";
    connect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(finished()));
    switch (role) {
    case PackageKit::Transaction::RoleGetDetails:
        connect(m_transaction, SIGNAL(details(QString,QString,PackageKit::Transaction::Group,QString,QString,qulonglong)),
                this, SLOT(description(QString,QString,PackageKit::Transaction::Group,QString,QString,qulonglong)));
        m_transaction->getDetails(m_packageID);
        break;
    case PackageKit::Transaction::RoleGetDepends:
        m_dependsModel->clear();
        connect(m_transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                m_dependsModel, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
        connect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                m_dependsModel, SLOT(finished()));
        m_transaction->getDepends(m_packageID, PackageKit::Transaction::FilterNone, false);
        break;
    case PackageKit::Transaction::RoleGetRequires:
        m_requiresModel->clear();
        connect(m_transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                m_requiresModel, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
        connect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                m_requiresModel, SLOT(finished()));
        m_transaction->getRequires(m_packageID, PackageKit::Transaction::FilterNone, false);
        break;
    case PackageKit::Transaction::RoleGetFiles:
        m_currentFileList.clear();
        connect(m_transaction, SIGNAL(files(QString,QStringList)),
                this, SLOT(files(QString,QStringList)));
        m_transaction->getFiles(m_packageID);
        break;
    }
    kDebug() <<"transaction running";

    PackageKit::Transaction::InternalError error = m_transaction->error();
    if (error) {
        disconnect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   this, SLOT(finished()));
        m_transaction = 0;
        kDebug() << "transaction running" << error << PkStrings::daemonError(error);
        KMessageBox::sorry(this, PkStrings::daemonError(error));
    } else {
        m_busySeq->start();
    }
}

void PackageDetails::resultJob(KJob *job)
{
    KIO::FileCopyJob *fJob = qobject_cast<KIO::FileCopyJob*>(job);
    if (!fJob->error()) {
        m_screenshotPath[fJob->srcUrl().url()] = fJob->destUrl().toLocalFile();
        display();
    }
}

void PackageDetails::hide()
{
    m_display = false;
    // Clean the old description otherwise if the user selects the same
    // package the pannel won't expand
    m_packageID.clear();
    m_appId.clear();

    if (maximumSize().height() == FINAL_HEIGHT) {
        if (m_fadeStacked->currentValue().toReal() == 0 &&
            m_fadeScreenshot->currentValue().toReal() == 0) {
            // Screen shot and description faded let's shrink the pannel
            m_expandPanel->setDirection(QAbstractAnimation::Backward);
            m_expandPanel->start();
        } else {
            // Hide current description
            fadeOut(PackageDetails::FadeScreenshot | PackageDetails::FadeStacked);
        }
    }
}

void PackageDetails::fadeOut(FadeWidgets widgets)
{
    // Fade out only if needed
    if ((widgets & FadeStacked) && m_fadeStacked->currentValue().toReal() != 0) {
        m_fadeStacked->setDirection(QAbstractAnimation::Backward);
        m_fadeStacked->start();
    }

    // Fade out the screenshot only if needed
    if ((widgets & FadeScreenshot) && m_fadeScreenshot->currentValue().toReal() != 0) {
        ui->screenshotL->unsetCursor();
        m_fadeScreenshot->setDirection(QAbstractAnimation::Backward);
        m_fadeScreenshot->start();
    }
}

void PackageDetails::display()
{
    // If we shouldn't be showing hide the pannel
    if (!m_display) {
        hide();
    } else if (maximumSize().height() == FINAL_HEIGHT) {
        emit ensureVisible(m_index);

        // Check to see if the stacked widget is transparent
        if (m_fadeStacked->currentValue().toReal() == 0 &&
            m_actionGroup->checkedAction())
        {
            bool fadeIn = false;
            switch (m_actionGroup->checkedAction()->data().toUInt()) {
            case PackageKit::Transaction::RoleGetDetails:
                if (m_hasDetails) {
                    setupDescription();
                    fadeIn = true;
                }
                break;
            case PackageKit::Transaction::RoleGetDepends:
                if (m_hasDepends) {
                    if (ui->stackedWidget->currentWidget() != ui->pageDepends) {
                        ui->stackedWidget->setCurrentWidget(ui->pageDepends);
                    }
                    fadeIn = true;
                }
                break;
            case PackageKit::Transaction::RoleGetRequires:
                if (m_hasRequires) {
                    if (ui->stackedWidget->currentWidget() != ui->pageRequired) {
                        ui->stackedWidget->setCurrentWidget(ui->pageRequired);
                    }
                    fadeIn = true;
                }
                break;
            case PackageKit::Transaction::RoleGetFiles:
                if (m_hasFileList) {
                    ui->filesPTE->clear();
                    if (m_currentFileList.isEmpty()) {
                        ui->filesPTE->insertPlainText(i18n("No files were found."));
                    } else {
                        m_currentFileList.sort();
                        ui->filesPTE->insertPlainText(m_currentFileList.join("\n"));
                    }

                    if (ui->stackedWidget->currentWidget() != ui->pageFiles) {
                        ui->stackedWidget->setCurrentWidget(ui->pageFiles);
                    }
                    ui->filesPTE->verticalScrollBar()->setValue(0);
                    fadeIn = true;
                }
                break;
            }

            if (fadeIn) {
                // Fade In
                m_fadeStacked->setDirection(QAbstractAnimation::Forward);
                m_fadeStacked->start();
            }
        }

        // Check to see if we have a screen shot and if we are
        // transparent, and make sure the details are going
        // to be shown
        if (m_fadeScreenshot->currentValue().toReal() == 0 &&
            m_screenshotPath.contains(m_currentScreenshot) &&
            m_fadeStacked->direction() == QAbstractAnimation::Forward) {
            QPixmap pixmap;
            pixmap = QPixmap(m_screenshotPath[m_currentScreenshot])
                             .scaled(160,120, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->screenshotL->setPixmap(pixmap);
            ui->screenshotL->setCursor(Qt::PointingHandCursor);
            // Fade In
            m_fadeScreenshot->setDirection(QAbstractAnimation::Forward);
            m_fadeScreenshot->start();
        }
    }
}

void PackageDetails::setupDescription()
{
    if (ui->stackedWidget->currentWidget() != ui->pageDescription) {
        ui->stackedWidget->setCurrentWidget(ui->pageDescription);
    }

    if (!m_hasDetails) {
        // Oops we don't have any details
        ui->descriptionL->setText(i18n("Could not fetch software details"));
        ui->descriptionL->show();

        // Hide stuff so we don't display outdated data
        ui->homepageL->hide();
        ui->pathL->hide();
        ui->licenseL->hide();
        ui->sizeL->hide();
        ui->iconL->clear();
    }

    if (!m_detailsDetail.isEmpty()) {
        ui->descriptionL->setText(m_detailsDetail.replace('\n', "<br>"));
        ui->descriptionL->show();
    } else {
        ui->descriptionL->clear();
    }

    if (!m_detailsUrl.isEmpty()) {
        ui->homepageL->setText("<a href=\"" + m_detailsUrl + "\">" +
                               m_detailsUrl + "</a>");
        ui->homepageL->show();
    } else {
        ui->homepageL->hide();
    }

    // Let's try to find the application's path in human user
    // readable easiest form :D
    KService::Ptr service = KService::serviceByDesktopName(m_appId);
    QVector<QPair<QString, QString> > ret;
    if (service) {
        ret = locateApplication(QString(), service->menuId());
    }
    if (ret.isEmpty()) {
        ui->pathL->hide();
    } else {
        QString path;
        path.append(QString("<img width=\"16\" heigh=\"16\"src=\"%1\"/>")
                    .arg(KIconLoader::global()->iconPath("kde", KIconLoader::Small)));
        path.append(QString("&nbsp;%1 <img width=\"16\" heigh=\"16\" src=\"%2\"/>&nbsp;%3")
                    .arg(QString::fromUtf8("➜"))
                    .arg(KIconLoader::global()->iconPath("applications-other", KIconLoader::Small))
                    .arg(i18n("Applications")));
        for (int i = 0; i < ret.size(); i++) {
            path.append(QString("&nbsp;%1&nbsp;<img width=\"16\" heigh=\"16\" src=\"%2\"/>&nbsp;%3")
                        .arg(QString::fromUtf8("➜"))
                        .arg(KIconLoader::global()->iconPath(ret.at(i).second, KIconLoader::Small))
                        .arg(ret.at(i).first));
        }
        ui->pathL->setText(path);
        ui->pathL->show();
    }

//     if (details->group() != Package::UnknownGroup) {
// //         description += "<tr><td align=\"right\"><b>" + i18nc("Group of the package", "Group") + ":</b></td><td>"
// //                     + PkStrings::groups(details->group())
// //                     + "</td></tr>";
//     }

    if (!m_detailsLicense.isEmpty() && m_detailsLicense != "unknown") {
        // We have a license, check if we have and should show show package version
        if (!m_hideVersion && !Transaction::packageVersion(m_detailsPackageID).isEmpty()) {
            ui->licenseL->setText(Transaction::packageVersion(m_detailsPackageID) + " - " + m_detailsLicense);
        } else {
            ui->licenseL->setText(m_detailsLicense);
        }
        ui->licenseL->show();
    } else if (!m_hideVersion) {
        ui->licenseL->setText(Transaction::packageVersion(m_detailsPackageID));
        ui->licenseL->show();
    } else {
        ui->licenseL->hide();
    }

    if (m_detailsSize > 0) {
        QString size = KGlobal::locale()->formatByteSize(m_detailsSize);
        if (!m_hideArch && !Transaction::packageArch(m_detailsPackageID).isEmpty()) {
            ui->sizeL->setText(size % QLatin1String(" (") % Transaction::packageArch(m_detailsPackageID) % QLatin1Char(')'));
        } else {
            ui->sizeL->setText(size);
        }
        ui->sizeL->show();
    } else if (!m_hideArch && !Transaction::packageArch(m_detailsPackageID).isEmpty()) {
        ui->sizeL->setText(Transaction::packageArch(m_detailsPackageID));
    } else {
        ui->sizeL->hide();
    }

    if (m_currentIcon.isNull()) {
        ui->iconL->clear();
    } else {
        ui->iconL->setPixmap(m_currentIcon);
    }
}

QVector<QPair<QString, QString> > PackageDetails::locateApplication(const QString &_relPath, const QString &menuId) const
{
    QVector<QPair<QString, QString> > ret;
    KServiceGroup::Ptr root = KServiceGroup::group(_relPath);

    if (!root || !root->isValid()) {
        return ret;
    }

    const KServiceGroup::List list = root->entries(false /* sorted */,
                                                   true /* exclude no display entries */,
                                                   false /* allow separators */);

    for (KServiceGroup::List::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        const KSycocaEntry::Ptr p = (*it);

        if (p->isType(KST_KService)) {
            const KService::Ptr service = KService::Ptr::staticCast(p);

            if (service->noDisplay()) {
                continue;
            }

//             kDebug() << menuId << service->menuId();
            if (service->menuId() == menuId) {
                QPair<QString, QString> pair;
                pair.first  = service->name();
                pair.second = service->icon();
                ret << pair;
//                 kDebug() << "FOUND!";
                return ret;
            }
        } else if (p->isType(KST_KServiceGroup)) {
            const KServiceGroup::Ptr serviceGroup = KServiceGroup::Ptr::staticCast(p);

            if (serviceGroup->noDisplay() || serviceGroup->childCount() == 0) {
                continue;
            }

            QVector<QPair<QString, QString> > found;
            found = locateApplication(serviceGroup->relPath(), menuId);
            if (!found.isEmpty()) {
                QPair<QString, QString> pair;
                pair.first  = serviceGroup->caption();
                pair.second = serviceGroup->icon();
                ret << pair;
                ret << found;
                return ret;
            }
        } else {
            kWarning(250) << "KServiceGroup: Unexpected object in list!";
            continue;
        }
    }

    return ret;
}

void PackageDetails::description(const QString &packageID,
                                 const QString &license,
                                 PackageKit::Transaction::Group group,
                                 const QString &detail,
                                 const QString &url,
                                 qulonglong size)
{
    kDebug() << packageID;
    m_detailsPackageID = packageID;
    m_detailsLicense = license;
    m_detailsGroup = group;
    m_detailsDetail = detail;
    m_detailsUrl = url;
    m_detailsSize = size;
    m_hasDetails = true;
}

void PackageDetails::finished()
{
    if (m_busySeq) {
        m_busySeq->stop();
    }
    m_transaction = 0;

    PackageKit::Transaction *transaction;
    transaction = qobject_cast<PackageKit::Transaction*>(sender());
    kDebug();
    if (transaction) {
        kDebug() << transaction->role() << PackageKit::Transaction::RoleGetDetails;
        if (transaction->role() == PackageKit::Transaction::RoleGetDetails) {
            m_hasDetails = true;
        } else if (transaction->role() == PackageKit::Transaction::RoleGetFiles) {
            m_hasFileList = true;
        } else if (transaction->role() == PackageKit::Transaction::RoleGetRequires) {
            m_hasRequires = true;
        } else if (transaction->role() == PackageKit::Transaction::RoleGetDepends) {
            m_hasDepends  = true;
        } else {
            return;
        }

        display();
    }
}

void PackageDetails::files(const QString &packageID, const QStringList &files)
{
    Q_UNUSED(packageID)
    m_currentFileList = files;
}

#include "PackageDetails.moc"
