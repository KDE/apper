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

#include "PackageDetails.h"
#include "ui_PackageDetails.h"

#include "ScreenShotViewer.h"

#include <PackageModel.h>
#include <PkStrings.h>
#include <PkIcons.h>

#include <KMessageBox>

#include <KSycocaEntry>
#include <KIconLoader>
#include <KService>
#include <KServiceGroup>
#include <KDesktopFile>
#include <QTemporaryFile>
#include <KPixmapSequence>
#include <QTextDocument>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <QPainter>
#include <QAbstractAnimation>
#include <QStringBuilder>

#include <KFormat>
#include <KIO/Job>
#include <QMenu>
#include <QDir>

#include <QLoggingCategory>

#include <Daemon>
#include <Transaction>

#include <config.h>

#ifdef HAVE_APPSTREAM
#include <AppStream.h>
#endif

#include "GraphicsOpacityDropShadowEffect.h"

#define BLUR_RADIUS 15
#define FINAL_HEIGHT 210

using namespace PackageKit;

Q_DECLARE_LOGGING_CATEGORY(APPER)

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
    ui->hideTB->setIcon(QIcon::fromTheme(QLatin1String("window-close")));
    connect(ui->hideTB, SIGNAL(clicked()), this, SLOT(hide()));

    auto menu = new QMenu(i18n("Display"), this);
    m_actionGroup = new QActionGroup(this);

    // we check to see which roles are supported by the backend
    // if so we ask for information and create the containers
    descriptionAction = menu->addAction(i18n("Description"));
    descriptionAction->setCheckable(true);
    descriptionAction->setData(PackageKit::Transaction::RoleGetDetails);
    m_actionGroup->addAction(descriptionAction);
    ui->descriptionW->setWidgetResizable(true);

    dependsOnAction = menu->addAction(i18n("Depends On"));
    dependsOnAction->setCheckable(true);
    dependsOnAction->setData(PackageKit::Transaction::RoleDependsOn);
    m_actionGroup->addAction(dependsOnAction);
    // Sets a transparent background
    QWidget *dependsViewport = ui->dependsOnLV->viewport();
    QPalette dependsPalette = dependsViewport->palette();
    dependsPalette.setColor(dependsViewport->backgroundRole(), Qt::transparent);
    dependsPalette.setColor(dependsViewport->foregroundRole(), dependsPalette.color(QPalette::WindowText));
    dependsViewport->setPalette(dependsPalette);

    m_dependsModel = new PackageModel(this);
    m_dependsProxy = new QSortFilterProxyModel(this);
    m_dependsProxy->setDynamicSortFilter(true);
    m_dependsProxy->setSortRole(PackageModel::SortRole);
    m_dependsProxy->setSourceModel(m_dependsModel);
    ui->dependsOnLV->setModel(m_dependsProxy);
    ui->dependsOnLV->sortByColumn(0, Qt::AscendingOrder);
    ui->dependsOnLV->header()->setDefaultAlignment(Qt::AlignCenter);
    ui->dependsOnLV->header()->setSectionResizeMode(PackageModel::NameCol, QHeaderView::ResizeToContents);
    ui->dependsOnLV->header()->setSectionResizeMode(PackageModel::VersionCol, QHeaderView::ResizeToContents);
    ui->dependsOnLV->header()->setSectionResizeMode(PackageModel::ArchCol, QHeaderView::Stretch);
    ui->dependsOnLV->header()->hideSection(PackageModel::ActionCol);
    ui->dependsOnLV->header()->hideSection(PackageModel::CurrentVersionCol);
    ui->dependsOnLV->header()->hideSection(PackageModel::OriginCol);
    ui->dependsOnLV->header()->hideSection(PackageModel::SizeCol);

    requiredByAction = menu->addAction(i18n("Required By"));
    requiredByAction->setCheckable(true);
    requiredByAction->setData(PackageKit::Transaction::RoleRequiredBy);
    m_actionGroup->addAction(requiredByAction);
    // Sets a transparent background
    QWidget *requiredViewport = ui->requiredByLV->viewport();
    QPalette requiredPalette = requiredViewport->palette();
    requiredPalette.setColor(requiredViewport->backgroundRole(), Qt::transparent);
    requiredPalette.setColor(requiredViewport->foregroundRole(), requiredPalette.color(QPalette::WindowText));
    requiredViewport->setPalette(requiredPalette);

    m_requiresModel = new PackageModel(this);
    m_requiresProxy = new QSortFilterProxyModel(this);
    m_requiresProxy->setDynamicSortFilter(true);
    m_requiresProxy->setSortRole(PackageModel::SortRole);
    m_requiresProxy->setSourceModel(m_requiresModel);
    ui->requiredByLV->setModel(m_requiresProxy);
    ui->requiredByLV->sortByColumn(0, Qt::AscendingOrder);
    ui->requiredByLV->header()->setDefaultAlignment(Qt::AlignCenter);
    ui->requiredByLV->header()->setSectionResizeMode(PackageModel::NameCol, QHeaderView::ResizeToContents);
    ui->requiredByLV->header()->setSectionResizeMode(PackageModel::VersionCol, QHeaderView::ResizeToContents);
    ui->requiredByLV->header()->setSectionResizeMode(PackageModel::ArchCol, QHeaderView::Stretch);
    ui->requiredByLV->header()->hideSection(PackageModel::ActionCol);
    ui->requiredByLV->header()->hideSection(PackageModel::CurrentVersionCol);
    ui->requiredByLV->header()->hideSection(PackageModel::OriginCol);
    ui->requiredByLV->header()->hideSection(PackageModel::SizeCol);

    fileListAction = menu->addAction(i18n("File List"));
    fileListAction->setCheckable(true);
    fileListAction->setData(PackageKit::Transaction::RoleGetFiles);
    m_actionGroup->addAction(fileListAction);
    // Sets a transparent background
    QWidget *actionsViewport = ui->filesPTE->viewport();
    QPalette palette = actionsViewport->palette();
    palette.setColor(actionsViewport->backgroundRole(), Qt::transparent);
    palette.setColor(actionsViewport->foregroundRole(), palette.color(QPalette::WindowText));
    actionsViewport->setPalette(palette);

    // Set the menu
    ui->menuTB->setMenu(menu);
    ui->menuTB->setIcon(QIcon::fromTheme(QLatin1String("help-about")));
    connect(m_actionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(actionActivated(QAction*)));

    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KIconLoader::global()->loadPixmapSequence(QLatin1String("process-working"), KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(ui->stackedWidget);

    // Setup the opacit effect that makes the descriptio transparent
    // after finished it checks in display() to see if it shouldn't show
    // up again. The property animation is always the same, the only different thing
    // is the Forward or Backward property
    auto effect = new QGraphicsOpacityEffect(ui->stackedWidget);
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
    auto shadow = new GraphicsOpacityDropShadowEffect(ui->screenshotL);
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
    connect(m_expandPanel, SIGNAL(finished()), this, SLOT(display()));
}

void PackageDetails::init(PackageKit::Transaction::Roles roles)
{
//    qCDebug();

    bool setChecked = true;
    if (roles & PackageKit::Transaction::RoleGetDetails) {
        descriptionAction->setEnabled(true);
        descriptionAction->setChecked(setChecked);
        setChecked = false;
    } else {
        descriptionAction->setEnabled(false);
        descriptionAction->setChecked(false);
    }

    if (roles & PackageKit::Transaction::RoleDependsOn) {
        dependsOnAction->setEnabled(true);
        dependsOnAction->setChecked(setChecked);
        setChecked = false;
    } else {
        dependsOnAction->setEnabled(false);
        dependsOnAction->setChecked(false);
    }

    if (roles & PackageKit::Transaction::RoleRequiredBy) {
        requiredByAction->setEnabled(true);
        requiredByAction->setChecked(setChecked);
        setChecked = false;
    } else {
        requiredByAction->setEnabled(false);
        requiredByAction->setChecked(false);
    }

    if (roles & PackageKit::Transaction::RoleGetFiles) {
        fileListAction->setEnabled(true);
        fileListAction->setChecked(setChecked);
        setChecked = false;
    } else {
        fileListAction->setEnabled(false);
        fileListAction->setChecked(false);
    }

}

PackageDetails::~PackageDetails()
{
    delete ui;
}

void PackageDetails::setPackage(const QModelIndex &index)
{
    qCDebug(APPER) << index;
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
    qCDebug(APPER) << "appId" << appId << "m_package" << m_packageID;

    QString pkgIconPath = index.data(PackageModel::IconRole).toString();
    m_currentIcon       = PkIcons::getIcon(pkgIconPath, QString()).pixmap(64, 64);
    m_appName           = index.data(PackageModel::NameRole).toString();

    m_currentScreenshot = thumbnail(Transaction::packageName(m_packageID));
    qCDebug(APPER) << "current thumbnail" << m_currentScreenshot;
    if (!m_currentScreenshot.isEmpty()) {
        if (m_screenshotPath.contains(m_currentScreenshot)) {
            display();
        } else {
            auto tempFile = new QTemporaryFile(QDir::tempPath() + QLatin1String("/apper.XXXXXX.png"));
            tempFile->open();
            KIO::FileCopyJob *job = KIO::file_copy(m_currentScreenshot,
                                                   QUrl::fromLocalFile(tempFile->fileName()),
                                                   -1,
                                                   KIO::Overwrite | KIO::HideProgressInfo);
            connect(job, &KIO::FileCopyJob::result, this, &PackageDetails::resultJob);
        }
    }

    if (m_actionGroup->checkedAction()) {
        actionActivated(m_actionGroup->checkedAction());
    }
}

void PackageDetails::on_screenshotL_clicked()
{
    const QUrl url = screenshot(Transaction::packageName(m_packageID));
    if (!url.isEmpty()) {
        auto view = new ScreenShotViewer(url);
        view->setWindowTitle(m_appName);
        view->show();
    }
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
    qCDebug(APPER);

    // disconnect the transaction
    // so that we don't get old data
    if (m_transaction) {
        disconnect(m_transaction, SIGNAL(details(PackageKit::Details)),
                   this, SLOT(description(PackageKit::Details)));
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
            description(m_details);
            display();
            return;
        }
        break;
    case PackageKit::Transaction::RoleDependsOn:
        if (m_hasDepends) {
            display();
            return;
        }
        break;
    case PackageKit::Transaction::RoleRequiredBy:
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
    qCDebug(APPER) << "New transaction";
    switch (role) {
    case PackageKit::Transaction::RoleGetDetails:
        m_transaction = Daemon::getDetails(m_packageID);
        connect(m_transaction, SIGNAL(details(PackageKit::Details)),
                SLOT(description(PackageKit::Details)));
        break;
    case PackageKit::Transaction::RoleDependsOn:
        m_dependsModel->clear();
        m_transaction = Daemon::dependsOn(m_packageID, PackageKit::Transaction::FilterNone, false);
        connect(m_transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                m_dependsModel, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
        connect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                m_dependsModel, SLOT(finished()));
        break;
    case PackageKit::Transaction::RoleRequiredBy:
        m_requiresModel->clear();
        m_transaction = Daemon::requiredBy(m_packageID, PackageKit::Transaction::FilterNone, false);
        connect(m_transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                m_requiresModel, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
        connect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                m_requiresModel, SLOT(finished()));
        break;
    case PackageKit::Transaction::RoleGetFiles:
        m_currentFileList.clear();
        m_transaction = Daemon::getFiles(m_packageID);
        connect(m_transaction, SIGNAL(files(QString,QStringList)),
                this, SLOT(files(QString,QStringList)));
        break;
    default:
        qWarning() << Q_FUNC_INFO << "Oops, unhandled role, please report" << role;
        return;
    }
    connect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(finished()));
    qCDebug(APPER) <<"transaction running";

    m_busySeq->start();
}

void PackageDetails::resultJob(KJob *job)
{
    KIO::FileCopyJob *fJob = qobject_cast<KIO::FileCopyJob*>(job);
    if (!fJob->error()) {
        m_screenshotPath[fJob->srcUrl()] = fJob->destUrl().toLocalFile();
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
            case PackageKit::Transaction::RoleDependsOn:
                if (m_hasDepends) {
                    if (ui->stackedWidget->currentWidget() != ui->pageDepends) {
                        ui->stackedWidget->setCurrentWidget(ui->pageDepends);
                    }
                    fadeIn = true;
                }
                break;
            case PackageKit::Transaction::RoleRequiredBy:
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
                        ui->filesPTE->insertPlainText(m_currentFileList.join(QLatin1Char('\n')));
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

    if (!m_detailsDescription.isEmpty()) {
        ui->descriptionL->setText(m_detailsDescription.replace(QLatin1Char('\n'), QLatin1String("<br>")));
        ui->descriptionL->show();
    } else {
        ui->descriptionL->clear();
    }

    if (!m_details.url().isEmpty()) {
        ui->homepageL->setText(QLatin1String("<a href=\"") + m_details.url() + QLatin1String("\">") +
                               m_details.url() + QLatin1String("</a>"));
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
        path.append(QString(QLatin1String("<img width=\"16\" heigh=\"16\"src=\"%1\"/>"))
                    .arg(KIconLoader::global()->iconPath(QLatin1String("kde"), KIconLoader::Small)));
        path.append(QString(QLatin1String("&nbsp;%1 <img width=\"16\" heigh=\"16\" src=\"%2\"/>&nbsp;%3"))
                    .arg(QString::fromUtf8("➜"))
                    .arg(KIconLoader::global()->iconPath(QLatin1String("applications-other"), KIconLoader::Small))
                    .arg(i18n("Applications")));
        for (int i = 0; i < ret.size(); i++) {
            path.append(QString(QLatin1String("&nbsp;%1&nbsp;<img width=\"16\" heigh=\"16\" src=\"%2\"/>&nbsp;%3"))
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

    if (!m_details.license().isEmpty() && m_details.license() != QLatin1String("unknown")) {
        // We have a license, check if we have and should show show package version
        if (!m_hideVersion && !Transaction::packageVersion(m_details.packageId()).isEmpty()) {
            ui->licenseL->setText(Transaction::packageVersion(m_details.packageId()) + QLatin1String(" - ") + m_details.license());
        } else {
            ui->licenseL->setText(m_details.license());
        }
        ui->licenseL->show();
    } else if (!m_hideVersion) {
        ui->licenseL->setText(Transaction::packageVersion(m_details.packageId()));
        ui->licenseL->show();
    } else {
        ui->licenseL->hide();
    }

    if (m_details.size() > 0) {
        QString size = KFormat().formatByteSize(m_details.size());
        if (!m_hideArch && !Transaction::packageArch(m_details.packageId()).isEmpty()) {
            ui->sizeL->setText(size % QLatin1String(" (") % Transaction::packageArch(m_details.packageId()) % QLatin1Char(')'));
        } else {
            ui->sizeL->setText(size);
        }
        ui->sizeL->show();
    } else if (!m_hideArch && !Transaction::packageArch(m_details.packageId()).isEmpty()) {
        ui->sizeL->setText(Transaction::packageArch(m_details.packageId()));
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

    KServiceGroup::List list = root->entries(false /* sorted */,
                                                   true /* exclude no display entries */,
                                                   false /* allow separators */);

    //! TODO: Port to KF5 properly
    Q_UNUSED(menuId)
#if 0
    for (KServiceGroup::List::ConstIterator it = list.begin(); it != list.end(); it++) {
        KSycocaEntry::Ptr = (*it);

        if (p->isType(KST_KService)) {
            KService *service = static_cast<KService *>(p.get());

            if (service->noDisplay()) {
                continue;
            }

//             qCDebug(APPER) << menuId << service->menuId();
            if (service->menuId() == menuId) {
                QPair<QString, QString> pair;
                pair.first  = service->name();
                pair.second = service->icon();
                ret << pair;
//                 qCDebug(APPER) << "FOUND!";
                return ret;
            }
        } else if (p->isType(KST_KServiceGroup)) {
            KServiceGroup *serviceGroup = static_cast<KServiceGroup *>(p.get());

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
#endif

    return ret;
}

QUrl PackageDetails::thumbnail(const QString &pkgName) const
{
#ifndef HAVE_APPSTREAM
    Q_UNUSED(pkgName)
    return QUrl();
#else
    return AppStreamHelper::instance()->thumbnail(pkgName);
#endif
}

QUrl PackageDetails::screenshot(const QString &pkgName) const
{
#ifndef HAVE_APPSTREAM
    Q_UNUSED(pkgName)
    return QUrl();
#else
    return AppStreamHelper::instance()->screenshot(pkgName);
#endif
}

void PackageDetails::description(const PackageKit::Details &details)
{
    qCDebug(APPER) << details;
    m_details = details;
    m_detailsDescription = details.description();
    m_hasDetails = true;

#ifdef HAVE_APPSTREAM
    // check if we have application details from Appstream data
    // FIXME: The whole AppStream handling sucks badly, since it was added later
    // and on to of the package-based model. So we can't respect the "multiple apps
    // in one package" case here.
    const QList<AppStream::Component> apps = AppStreamHelper::instance()->applications(Transaction::packageName(m_packageID));
    for (const AppStream::Component &app : apps) {
        if (!app.description().isEmpty()) {
            m_detailsDescription = app.description();
            break;
        }
    }
#endif
}

void PackageDetails::finished()
{
    if (m_busySeq) {
        m_busySeq->stop();
    }
    m_transaction = 0;

    auto transaction = qobject_cast<PackageKit::Transaction*>(sender());
    qCDebug(APPER);
    if (transaction) {
        qCDebug(APPER) << transaction->role() << PackageKit::Transaction::RoleGetDetails;
        if (transaction->role() == PackageKit::Transaction::RoleGetDetails) {
            m_hasDetails = true;
        } else if (transaction->role() == PackageKit::Transaction::RoleGetFiles) {
            m_hasFileList = true;
        } else if (transaction->role() == PackageKit::Transaction::RoleRequiredBy) {
            m_hasRequires = true;
        } else if (transaction->role() == PackageKit::Transaction::RoleDependsOn) {
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

#include "moc_PackageDetails.cpp"
