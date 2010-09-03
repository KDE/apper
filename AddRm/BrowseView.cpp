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

#include <KpkPackageModel.h>
#include <KpkDelegate.h>
#include <KpkSearchableTreeView.h>

#include <QPackageKit>

#include <KFileDialog>
#include <KCategorizedSortFilterProxyModel>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QAbstractItemView>

#include <KDebug>

using namespace PackageKit;

BrowseView::BrowseView(QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);

    m_packageView = new KpkSearchableTreeView;
    m_packageView->setAlternatingRowColors(true);
    m_packageView->setRootIsDecorated(false);
    m_packageView->setSortingEnabled(true);
    m_packageView->setFrameStyle(QFrame::NoFrame);
    m_packageView->setVerticalScrollBar(verticalScrollBar);


    m_model = new KpkPackageModel(this, m_packageView);
    KCategorizedSortFilterProxyModel *proxy = new KCategorizedSortFilterProxyModel(this);
    proxy->setSourceModel(m_model);
    proxy->setDynamicSortFilter(true);
    proxy->setCategorizedModel(true);
    proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxy->setSortRole(KpkPackageModel::SortRole);

    m_packageView->setModel(proxy);
    m_packageView->sortByColumn(0, Qt::AscendingOrder);
    m_packageView->header()->setDefaultAlignment(Qt::AlignCenter);


    QGraphicsScene *scene = new QGraphicsScene(graphicsView);
    m_proxyWidget = scene->addWidget(m_packageView);
    KpkDelegate *delegate = new KpkDelegate(m_packageView);
    delegate->setViewport(graphicsView->viewport());
    m_packageView->setItemDelegate(delegate);
    connect(delegate, SIGNAL(showExtendItem(const QModelIndex &)),
            this, SLOT(showExtendItem(const QModelIndex &)));
    connect(proxy, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
            this, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)));

    exportInstalledPB->setIcon(KIcon("document-export"));
    importInstalledPB->setIcon(KIcon("document-import"));



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
    graphicsView->setScene(scene);


//     graphicsView->fitInView(m_proxyWidget, Qt::KeepAspectRatio);
}

BrowseView::~BrowseView()
{
}

bool BrowseView::showPageHeader() const
{
    return false;
}

KpkPackageModel* BrowseView::model() const
{
    return m_model;
}

void BrowseView::showExtendItem(const QModelIndex &)
{
    kDebug() << "foo;";
    QGraphicsBlurEffect *blurEffect = new QGraphicsBlurEffect(this);
    blurEffect->setBlurRadius(0);
    QPropertyAnimation *animation = new QPropertyAnimation(blurEffect, "blurRadius");
    animation->setDuration(1000);
    animation->setStartValue(qreal(0.5));
    animation->setEndValue(qreal(5));
    animation->setEasingCurve(QEasingCurve::OutQuart);

    animation->start();
    m_proxyWidget->setGraphicsEffect(blurEffect);
}

void BrowseView::showInstalledPanel(bool visible)
{
    installedF->setVisible(visible);
}

void BrowseView::setCategoryModel(QAbstractItemModel *model)
{
    categoryView->setModel(model);
}

void BrowseView::setParentCategory(const QModelIndex &index)
{
    categoryView->setRootIndex(index);
    // Display the category view if the index has child items
    categoryF->setVisible(categoryView->model()->rowCount(index));
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

void BrowseView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateSceneEvent();
}

bool BrowseView::event(QEvent *event)
{
     switch (event->type()) {
         case QEvent::Paint:
         case QEvent::PolishRequest:
         case QEvent::Polish:
             updateSceneEvent();
             break;
         default:
             break;
     }

     return QWidget::event(event);
}

void BrowseView::updateSceneEvent()
{
    graphicsView->setSceneRect(QRect(QPoint(0, 0), graphicsView->viewport()->size()));
    m_packageView->resize(graphicsView->viewport()->size());
}

#include "BrowseView.moc"
