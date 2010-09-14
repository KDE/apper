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

#include "BrowseView.h"

#include "KpkPackageDetails.h"

#include <ApplicationsDelegate.h>
#include <KpkPackageModel.h>
#include <KpkSearchableTreeView.h>

#include <QPackageKit>

#include <KFileDialog>
#include <KCategorizedSortFilterProxyModel>
#include <KMenu>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QAbstractItemView>

#include <KDebug>

using namespace PackageKit;

BrowseView::BrowseView(QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);

    connect(categoryView, SIGNAL(activated(const QModelIndex &)),
            this, SIGNAL(categoryActivated(const QModelIndex &)));

    m_model = new KpkPackageModel(this, packageView);
    m_proxy = new KCategorizedSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setDynamicSortFilter(true);
    m_proxy->setCategorizedModel(true);
    m_proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setSortRole(KpkPackageModel::SortRole);
    m_proxy->setFilterRole(KpkPackageModel::ApplicationFilterRole);

    packageView->setModel(m_proxy);
    packageView->sortByColumn(0, Qt::AscendingOrder);
    packageView->header()->setDefaultAlignment(Qt::AlignCenter);
    packageView->header()->setStretchLastSection(false);
    packageView->header()->setResizeMode(0, QHeaderView::ResizeToContents);
    packageView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
    packageView->header()->setResizeMode(2, QHeaderView::Stretch);


//     m_scene = new QGraphicsScene(graphicsView);
//     m_proxyWidget = m_scene->addWidget(packageView);
    ApplicationsDelegate *delegate = new ApplicationsDelegate(packageView);
    delegate->setExtendPixmapWidth(0);
//     delegate->setViewport(graphicsView->viewport());
//     delegate->setContractPixmap(SmallIcon("help-about"));
    packageView->setItemDelegate(delegate);
    connect(delegate, SIGNAL(showExtendItem(const QModelIndex &)),
            this, SLOT(showExtendItem(const QModelIndex &)));
    connect(m_proxy, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
            this, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)));

    exportInstalledPB->setIcon(KIcon("document-export"));
    importInstalledPB->setIcon(KIcon("document-import"));

//     packageDetails->layout()->setCon


//     QGraphicsBlurEffect *blurEffect = new QGraphicsBlurEffect(this);

// importInstalledPB->setGraphicsEffect(blurEffect);
//     QGraphicsProxyWidget *opaItem = scene->addWidget(new QGroupBox);
//     QGraphicsOpacityEffect *opaci = new QGraphicsOpacityEffect(this);
//     opaItem->setGraphicsEffect(opaci);

//     QPushButton *blurButton = new QPushButton("Blur");
//     QGraphicsProxyWidget *blurItem = scene->addWidget(blurButton);
//     blurItem->setPos(-blurButton->width()/2, -10-blurButton->height());

//     QGraphicsBlurEffect *blurEffect = new QGraphicsBlurEffect(this);
//     blurButton->setGraphicsEffect(blurEffect); // button disapears
//     blurItem->setGraphicsEffect(blurEffect); // works
    //view.setGraphicsEffect(blurEffect); // works first, then does not work

// QGraphicsBlurEffect *blurEffect = new QGraphicsBlurEffect(this);
//     m_proxyWidget->setGraphicsEffect(blurEffect);
//     graphicsView->setScene(m_scene);


//     graphicsView->fitInView(m_proxyWidget, Qt::KeepAspectRatio);
//     packageDetails->hide();
    KConfig config("KPackageKit");
    KConfigGroup viewGroup(&config, "ViewGroup");
    m_showPackageVersion = new QAction(i18n("Show Versions"), this);
    m_showPackageVersion->setCheckable(true);
    connect(m_showPackageVersion, SIGNAL(toggled(bool)),
            this, SLOT(showVersions(bool)));
    m_showPackageVersion->setChecked(viewGroup.readEntry("ShowVersions", false));
    showVersions(m_showPackageVersion->isChecked());
}

BrowseView::~BrowseView()
{
    KConfig config("KPackageKit");
    KConfigGroup viewGroup(&config, "ViewGroup");
    viewGroup.writeEntry("ShowVersions", m_showPackageVersion->isChecked());
}

bool BrowseView::showPageHeader() const
{
    return false;
}

KpkPackageModel* BrowseView::model() const
{
    return m_model;
}

void BrowseView::showVersions(bool enabled)
{
    packageView->header()->setSectionHidden(1, !enabled);
}

void BrowseView::on_packageView_customContextMenuRequested(const QPoint &pos)
{
    KMenu *menu = new KMenu(this);
    menu->addAction(m_showPackageVersion);
    menu->exec(packageView->mapToGlobal(pos));
    delete menu;
}

void BrowseView::on_packageView_activated(const QModelIndex &index)
{
    if (index.column() == 2) {
        return;
    }
//     packageDetails->show();
//     kDebug() << "foo;";
//     packageView->setMouseTracking(false);
//     QGraphicsBlurEffect *blurEffect = new QGraphicsBlurEffect(this);
//     blurEffect->setBlurRadius(0);
//      packageView->viewport()->setGraphicsEffect(blurEffect);
//
//     QPropertyAnimation *animation = new QPropertyAnimation(blurEffect, "blurRadius");
//     animation->setDuration(1000);
//     animation->setStartValue(qreal(0.5));
//     animation->setEndValue(qreal(10));
//     animation->setEasingCurve(QEasingCurve::OutQuart);
//     animation->start();
// m_oldDetails = m_details;
// m_details = new KpkPackageDetails;
// proxy
QModelIndex origIndex = m_proxy->mapToSource(index);
packageDetails->setPackage(origIndex);

if (packageDetails->minimumSize().height() == 210) {
//     if (m_details->graphicsEffect()) {
//         QPropertyAnimation *anim1 = new QPropertyAnimation(m_oldDetails->graphicsEffect(), "opacity");
//         anim1->setDuration(1000);
//         anim1->setEndValue(qreal(0));
//         anim1->start();
//         connect(anim1, SIGNAL(finished()), m_oldDetails, SLOT(deleteLater()));
//         connect(anim1, SIGNAL(finished()), this, SLOT(animationFinished()));
//         return;
//     }
    return;
}


//     m_details->setPackage(m_model->package(index), index);

 QPropertyAnimation *anim1 = new QPropertyAnimation(packageDetails, "maximumSize");
 anim1->setDuration(500);
 anim1->setEasingCurve(QEasingCurve::OutQuart);
 anim1->setStartValue(QSize(QWIDGETSIZE_MAX, 0));
 anim1->setEndValue(QSize(QWIDGETSIZE_MAX, 210));
  QPropertyAnimation *anim2 = new QPropertyAnimation(packageDetails, "minimumSize");
 anim2->setDuration(500);
 anim2->setEasingCurve(QEasingCurve::OutQuart);
 anim2->setStartValue(QSize(QWIDGETSIZE_MAX, 0));
 anim2->setEndValue(QSize(QWIDGETSIZE_MAX, 210));
//
//  anim1->start();
//      packageDetails->show();

QParallelAnimationGroup *group = new QParallelAnimationGroup;
 group->addAnimation(anim1);
 group->addAnimation(anim2);
connect(group, SIGNAL(finished()), this, SLOT(animationFinished()));
 group->start();

//     KpkPackageDetails *details = new KpkPackageDetails(m_model->package(index), index, Client::instance()->actions());
//     packageView->setLayout(new QGridLayout());
//     packageView->layout()->addWidget(details);
// // //     QTabWidget *tab = new QTabWidget;
// // //     tab->addTab(details, "details");
//     details->setAttribute(Qt::WA_NoSystemBackground, true);
// // //     QGraphicsProxyWidget *proxyWidget = m_scene->addWidget(details);
// // //     proxyWidget->setAttribute(Qt::WA_NoSystemBackground, true);
// // //     tab->setAutoFillBackground(false);
// // //     proxyWidget->setAutoFillBackground(false);
// // //     proxyWidget->setParent(m_proxyWidget);
// //
// // //     QGraphicsRectItem *itemBox = m_scene->addRect(QRectF(0, 0, 400, 800));
// // //     QGraphicsTextItem *itemText = new QGraphicsTextItem(index.data(KpkPackageModel::SummaryRole).toString(), itemBox);
// // //     QGraphicsItemGroup *group = m_scene->createItemGroup(QList<QGraphicsItem *>() << itemBox << itemText);
// // //     group->setPos(20, 20);
//     QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(details);
// // //     QGraphicsDropShadowEffect *shadow2 = new QGraphicsDropShadowEffect(this);
// // //     shadow->setColor(QColor(Qt::blue));
//     shadow->setBlurRadius(15);
//     shadow->setOffset(2);
// // //     shadow2->setColor(QColor(Qt::blue));
// // //     shadow2->setBlurRadius(15);
// // //     shadow2->setOffset(2);
//     details->setGraphicsEffect(shadow);
// //     details->show();
// // //     itemText->setTextWidth(350);
// // //     itemBox->setGraphicsEffect(shadow2);
}

void BrowseView::animationFinished()
{
//     packageDetails->layout()->addWidget(m_details);
    packageDetails->setDisplayDetails(true);
}

void BrowseView::showInstalledPanel(bool visible)
{
    installedF->setVisible(visible);
}

KCategorizedSortFilterProxyModel* BrowseView::proxy() const
{
    return m_proxy;
}

void BrowseView::setCategoryModel(QAbstractItemModel *model)
{
    categoryView->setModel(model);
}

void BrowseView::setParentCategory(const QModelIndex &index)
{
    categoryView->setRootIndex(index);
    // Make sure the last item is not selected
    categoryView->selectionModel()->clearSelection();
    categoryView->horizontalScrollBar()->setValue(0);

    // Display the category view if the index has child items
    categoryF->setVisible(categoryView->model()->rowCount(index));
}

bool BrowseView::goBack()
{
    QModelIndex index = categoryView->rootIndex();
    if (index.parent().isValid()) {
        setParentCategory(index.parent());
        emit categoryActivated(index.parent());
        return false;
    }
    return true;
}

void BrowseView::hideCategory()
{
    categoryF->setVisible(false);
}

void BrowseView::on_categoryMvLeft_clicked()
{
    categoryView->horizontalScrollBar()->setValue(categoryView->horizontalScrollBar()->value() - 1);
}

void BrowseView::on_categoryMvRight_clicked()
{
    categoryView->horizontalScrollBar()->setValue(categoryView->horizontalScrollBar()->value() + 1);
}

void BrowseView::on_exportInstalledPB_clicked()
{
    // We will assume the installed model
    // is populated since the user is seeing it.
    QString fileName;
    fileName = KFileDialog::getSaveFileName(KUrl(), "*.catalog", this);
    if (fileName.isEmpty()) {
        return;
    }

    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);

    out << "[PackageKit Catalog]\n\n";
    out << "InstallPackages(" << Client::instance()->distroId() << ")=";
    QStringList packages;
    for (int i = 0; i < m_model->rowCount(); i++) {
        packages << m_model->data(m_model->index(i, 0),
                                  KpkPackageModel::NameRole).toString();
    }
    out << packages.join(";");
}

void BrowseView::on_importInstalledPB_clicked()
{
    QString fileName;
    fileName = KFileDialog::getOpenFileName(KUrl(), "*.catalog", this);
    if (fileName.isEmpty()) {
        return;
    }

    // send a DBus message to install this catalog
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.freedesktop.PackageKit",
                                             "/org/freedesktop/PackageKit",
                                             "org.freedesktop.PackageKit.Modify",
                                             "InstallCatalogs");
    message << static_cast<uint>(effectiveWinId());
    message << (QStringList() << fileName);
    message << QString();

    // This call must block otherwise this application closes before
    // smarticon is activated
    QDBusMessage reply = QDBusConnection::sessionBus().call(message, QDBus::Block);
}

void BrowseView::disableExportInstalledPB()
{
    exportInstalledPB->setEnabled(false);
}

void BrowseView::enableExportInstalledPB()
{
    exportInstalledPB->setEnabled(true);
}

// void BrowseView::resizeEvent(QResizeEvent *event)
// {
//     QWidget::resizeEvent(event);
//     updateSceneEvent();
// }

// bool BrowseView::event(QEvent *event)
// {
//      switch (event->type()) {
//          case QEvent::Paint:
//          case QEvent::PolishRequest:
//          case QEvent::Polish:
//              updateSceneEvent();
//              break;
//          default:
//              break;
//      }
//
//      return QWidget::event(event);
// }
//
// void BrowseView::updateSceneEvent()
// {
//     graphicsView->setSceneRect(QRect(QPoint(0, 0), graphicsView->viewport()->size()));
//     packageView->resize(graphicsView->viewport()->size());
// }

#include "BrowseView.moc"
