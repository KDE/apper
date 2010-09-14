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

#include <KService>
#include <KServiceGroup>
#include <KDesktopFile>
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
   m_busySeq(0),
   m_display(false),
   m_transaction(0),
   m_hasDetails(false),
   m_hasFileList(false),
   m_dependsModel(0),
   m_requiresModel(0)
{
    setupUi(this);

    Enum::Roles roles = Client::instance()->actions();
    // Create a stacked layout to put the views in
    m_viewLayout = new QStackedLayout(stackedWidget);

    KMenu *menu = new KMenu(i18n("Display"), this);
    m_actionGroup = new QActionGroup(this);
    QAction *action = 0;

    // we check to see which roles are supported by the backend
    // if so we ask for information and create the containers
    if (roles & Enum::RoleGetDetails) {
        action = menu->addAction(i18n("Description"));
        action->setCheckable(true);
        action->setData(Enum::RoleGetDetails);
        m_actionGroup->addAction(action);
        m_viewLayout->addWidget(descriptionW);
        descriptionW->setWidgetResizable(true);
    }

    if (roles & Enum::RoleGetDepends) {
        action = menu->addAction(i18n("Depends On"));
        action->setCheckable(true);
        action->setData(Enum::RoleGetDepends);
        m_actionGroup->addAction(action);
        dependsOnLV = new QListView(stackedWidget);
        dependsOnLV->setFrameShape(QFrame::NoFrame);
        // Sets a transparent background
        QWidget *actionsViewport = dependsOnLV->viewport();
        QPalette palette = actionsViewport->palette();
        palette.setColor(actionsViewport->backgroundRole(), Qt::transparent);
        palette.setColor(actionsViewport->foregroundRole(), palette.color(QPalette::WindowText));
        actionsViewport->setPalette(palette);
        m_viewLayout->addWidget(dependsOnLV);
    }

    if (roles & Enum::RoleGetRequires) {
        action = menu->addAction(i18n("Required By"));
        action->setCheckable(true);
        action->setData(Enum::RoleGetRequires);
        m_actionGroup->addAction(action);
        requiredByLV = new QListView(stackedWidget);
        requiredByLV->setFrameShape(QFrame::NoFrame);
        // Sets a transparent background
        QWidget *actionsViewport = requiredByLV->viewport();
        QPalette palette = actionsViewport->palette();
        palette.setColor(actionsViewport->backgroundRole(), Qt::transparent);
        palette.setColor(actionsViewport->foregroundRole(), palette.color(QPalette::WindowText));
        actionsViewport->setPalette(palette);
        m_viewLayout->addWidget(requiredByLV);
    }

    if (roles & Enum::RoleGetFiles) {
        action = menu->addAction(i18n("File List"));
        action->setCheckable(true);
        action->setData(Enum::RoleGetFiles);
        m_actionGroup->addAction(action);
        filesPTE = new QPlainTextEdit(stackedWidget);
        filesPTE->setFrameShape(QFrame::NoFrame);
        filesPTE->setReadOnly(true);
        // Sets a transparent background
        QWidget *actionsViewport = filesPTE->viewport();
        QPalette palette = actionsViewport->palette();
        palette.setColor(actionsViewport->backgroundRole(), Qt::transparent);
        palette.setColor(actionsViewport->foregroundRole(), palette.color(QPalette::WindowText));
        actionsViewport->setPalette(palette);
        m_viewLayout->addWidget(filesPTE);
    }

    // Check to se if we have any action
    if (m_actionGroup->actions().isEmpty()) {
        menuTB->hide();
    } else {
        action = m_actionGroup->actions().first();
        action->setChecked(true);
        connect(m_actionGroup, SIGNAL(triggered(QAction *)),
                this, SLOT(actionActivated(QAction *)));
        // Set the menu
        menuTB->setMenu(menu);
        menuTB->setIcon(KIcon("help-about"));
    }

    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(stackedWidget);

    // Setup the opacit effect that makes the descriptio transparent
    // after finished it checks in display() to see if it shouldn't show
    // up again. The property animation is always the same, the only different thing
    // is the the Forward or Backward property
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(stackedWidget);
    effect->setOpacity(0);
    stackedWidget->setVisible(false);
    stackedWidget->setGraphicsEffect(effect);
    m_fadeStacked = new QPropertyAnimation(effect, "opacity");
    m_fadeStacked->setDuration(500);
    m_fadeStacked->setStartValue(qreal(0));
    m_fadeStacked->setEndValue(qreal(1));
    connect(m_fadeStacked, SIGNAL(finished()), this, SLOT(display()));


    // It's is impossible due to some limitation in Qt to set two effects on the same
    // Widget
    m_fadeScreenshot = new QPropertyAnimation(effect, "opacity");
    GraphicsOpacityDropShadowEffect *shadow = new GraphicsOpacityDropShadowEffect(screenshotL);
    shadow->setOpacity(0);
    shadow->setBlurRadius(BLUR_RADIUS);
    shadow->setOffset(2);
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
    if (value && !stackedWidget->isVisible()) {
        stackedWidget->setVisible(true);
    }
    display();
}

void KpkPackageDetails::setPackage(const QModelIndex &index)
{
    QString pkgId = index.data(KpkPackageModel::IdRole).toString();
    QString appId = index.data(KpkPackageModel::ApplicationId).toString();

    // if it's the same package and the same application, return
    if (!m_package.isNull() && pkgId == m_package->id() && appId == m_appId) {
        return;
    } else if (m_display) {
        fadeOut(KpkPackageDetails::FadeScreenshot | KpkPackageDetails::FadeStacked);
    }

    Enum::Info info = static_cast<Enum::Info>(index.data(KpkPackageModel::InfoRole).toUInt());

    m_package       = QSharedPointer<Package>(new Package(pkgId, info, QString()));;
    m_hasDetails    = false;
    m_hasFileList   = false;
    m_hasRequires   = false;
    m_hasDepends    = false;

    QString pkgName     = index.data(KpkPackageModel::IdRole).toString().split(';')[0];
    QString pkgIconPath = index.data(KpkPackageModel::IconRole).toString();
    m_appId             = index.data(KpkPackageModel::ApplicationId).toString();
    m_currentIcon       = KpkIcons::getIcon(pkgIconPath, "package").pixmap(64, 64);

    // TODO do not hard code...
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
}

void KpkPackageDetails::actionActivated(QAction *action)
{
    // don't fade the screenshot
    // if the package changed setPackage() fades both
    fadeOut(FadeStacked);

    // disconnect the transaction
    // so that we don't get old data
    if (m_transaction) {
        disconnect(m_transaction, SIGNAL(details(const QSharedPointer<PackageKit::Package> &)),
                   this, SLOT(description(const QSharedPointer<PackageKit::Package> &)));
        disconnect(m_transaction, SIGNAL(package(const QSharedPointer<PackageKit::Package> &)),
                   m_dependsModel, SLOT(addPackage(const QSharedPointer<PackageKit::Package> &)));
        disconnect(m_transaction, SIGNAL(package(const QSharedPointer<PackageKit::Package> &)),
                   m_requiresModel, SLOT(addPackage(const QSharedPointer<PackageKit::Package> &)));
        disconnect(m_transaction, SIGNAL(files(const QSharedPointer<PackageKit::Package> &, const QStringList &)),
                   this, SLOT(files(const QSharedPointer<PackageKit::Package> &, const QStringList &)));
        m_transaction = 0;
    }

    // Check to see if we don't already have the required data
    uint role = action->data().toUInt();
    if (role == Enum::RoleGetDetails &&
        m_package->hasDetails()) {
        description(m_package);
        return;
    }
    switch (role) {
    case Enum::RoleGetDetails:
        if (m_package->hasDetails()) {
            description(m_package);
            return;
        }
        break;
    case Enum::RoleGetDepends:
        if (m_hasDepends) {
            display();
            return;
        }
        break;
    case Enum::RoleGetRequires:
        if (m_hasRequires) {
            display();
            return;
        }
        break;
    case Enum::RoleGetFiles:
        if (m_hasFileList) {
            display();
            return;
        }
        break;
    }

    // we don't have the data
    m_transaction = new Transaction(QString(), this);
    switch (role) {
    case Enum::RoleGetDetails:
        connect(m_transaction, SIGNAL(details(const QSharedPointer<PackageKit::Package> &)),
                this, SLOT(description(const QSharedPointer<PackageKit::Package> &)));
        m_transaction->getDetails(m_package);
        break;
    case Enum::RoleGetDepends:
        if (m_dependsModel) {
            delete m_dependsModel;
        }
        m_dependsModel = new KpkSimplePackageModel(this);
        connect(m_transaction, SIGNAL(package(const QSharedPointer<PackageKit::Package> &)),
                m_dependsModel, SLOT(addPackage(const QSharedPointer<PackageKit::Package> &)));
        connect(m_transaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                this, SLOT(finished()));
        m_transaction->getDepends(m_package, PackageKit::Enum::NoFilter, false);
        break;
    case Enum::RoleGetRequires:
        if (m_requiresModel) {
            delete m_requiresModel;
        }
        m_requiresModel = new KpkSimplePackageModel(this);
        connect(m_transaction, SIGNAL(package(const QSharedPointer<PackageKit::Package> &)),
                m_requiresModel, SLOT(addPackage(const QSharedPointer<PackageKit::Package> &)));
        connect(m_transaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                this, SLOT(finished()));
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
        m_busySeq->start();
    }
}

void KpkPackageDetails::resultJob(KJob *job)
{
    kDebug();
    KIO::FileCopyJob *fJob = qobject_cast<KIO::FileCopyJob*>(job);
    if (!fJob->error()) {
        m_screenshotPath[fJob->srcUrl().url()] = fJob->destUrl().toLocalFile();
        display();
    }
}

void KpkPackageDetails::fadeOut(FadeWidgets widgets)
{
    // Fade out only if needed
    if ((widgets & FadeStacked) && m_fadeStacked->currentValue().toReal() != 0) {
        m_fadeStacked->setDirection(QAbstractAnimation::Backward);
        m_fadeStacked->start();
    }

    // Fade out the screenshot only if needed
    if ((widgets & FadeScreenshot) && m_fadeScreenshot->currentValue().toReal() != 0) {
        m_fadeScreenshot->setDirection(QAbstractAnimation::Backward);
        m_fadeScreenshot->start();
    }
}

void KpkPackageDetails::display()
{
    // If we shouldn't be showing fade out
    if (!m_display) {
        fadeOut(FadeScreenshot | FadeStacked);
    } else {
        // Check to see if the stacked widget is transparent
        if (m_fadeStacked->currentValue().toReal() == 0 &&
            m_actionGroup->checkedAction())
        {
            bool fadeIn = false;
            switch (m_actionGroup->checkedAction()->data().toUInt()) {
            case Enum::RoleGetDetails:
                if (m_hasDetails) {
                    setupDescription();
                    fadeIn = true;
                }
                break;
            case Enum::RoleGetDepends:
                if (m_hasDepends) {
                    QAbstractItemModel *currentModel = dependsOnLV->model();
                    if (currentModel != m_dependsModel) {
                        dependsOnLV->setModel(m_dependsModel);
                        delete currentModel;
                    }
                    if (m_viewLayout->currentWidget() != dependsOnLV) {
                        m_viewLayout->setCurrentWidget(dependsOnLV);
                    }
                    fadeIn = true;
                }
                break;
            case Enum::RoleGetRequires:
                if (m_hasRequires) {
                    QAbstractItemModel *currentModel = requiredByLV->model();
                    if (currentModel != m_requiresModel) {
                        requiredByLV->setModel(m_requiresModel);
                        delete currentModel;
                    }
                    if (m_viewLayout->currentWidget() != requiredByLV) {
                        m_viewLayout->setCurrentWidget(requiredByLV);
                    }
                    fadeIn = true;
                }
                break;
            case Enum::RoleGetFiles:
                if (m_hasFileList) {
                    filesPTE->clear();
                    filesPTE->insertPlainText(m_currentFileList.join("\n"));
                    if (m_viewLayout->currentWidget() != filesPTE) {
                        m_viewLayout->setCurrentWidget(filesPTE);
                    }
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
        // transparent, and make shure the details are going
        // to be shown
        if (m_fadeScreenshot->currentValue().toReal() == 0 &&
            m_screenshotPath.contains(m_currentScreenshot) &&
            m_fadeStacked->direction() == QAbstractAnimation::Forward) {
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

void KpkPackageDetails::setupDescription()
{
    if (m_viewLayout->currentWidget() != descriptionW) {
        m_viewLayout->setCurrentWidget(descriptionW);
    }

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
        homepageL->hide();
    }

    if (!details->license().isEmpty() && details->license() != "unknown") {
        licenseL->setText(details->license());
        licenseL->show();
    } else {
        licenseL->hide();
    }

    // Let's try to find the application's path in human user
    // readable easiest form :D
    kDebug() << m_appId;
    KService::Ptr service = KService::serviceByDesktopName(m_appId);
    QVector<QPair<QString, QString> > ret;
    if (service) {
        ret = locateApplication(QString(), service->menuId());
    }
    if (ret.isEmpty()) {
        pathL->hide();
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
        pathL->setText(path);
        pathL->show();
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
        sizeL->hide();
    }

    iconL->setPixmap(m_currentIcon);
}

QVector<QPair<QString, QString> > KpkPackageDetails::locateApplication(const QString &_relPath, const QString &menuId) const
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

void KpkPackageDetails::description(const QSharedPointer<PackageKit::Package> &package)
{
    if (m_busySeq) {
        m_busySeq->stop();
    }

    m_package     = package;
    m_hasDetails  = true;
    m_transaction = 0;

    display();
}

void KpkPackageDetails::finished()
{
    if (m_busySeq) {
        m_busySeq->stop();
    }
    m_transaction     = 0;

    Transaction *transaction = qobject_cast<Transaction*>(sender());
    if (transaction) {
        if (transaction->role() == Enum::RoleGetDepends) {
            m_hasDepends = true;
        } else if (transaction->role() == Enum::RoleGetRequires) {
            m_hasRequires = true;
        } else {
            return;
        }
        display();
    }
}

void KpkPackageDetails::files(QSharedPointer<PackageKit::Package> package, const QStringList &files)
{
    Q_UNUSED(package)
    kDebug();
    if (m_busySeq) {
        m_busySeq->stop();
    }

    m_currentFileList = files;
    m_hasFileList     = true;
    m_transaction     = 0;

    display();
}


#include "KpkPackageDetails.moc"
