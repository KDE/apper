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

#include <QPackageKit>

#include <KFileDialog>
#include <KPixmapSequence>
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

    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(packageView->viewport());

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
    packageView->header()->setResizeMode(2, QHeaderView::ResizeToContents);
    packageView->header()->setResizeMode(3, QHeaderView::Stretch);
    packageView->header()->setResizeMode(4, QHeaderView::ResizeToContents);

    ApplicationsDelegate *delegate = new ApplicationsDelegate(packageView);
    packageView->setItemDelegate(delegate);

    exportInstalledPB->setIcon(KIcon("document-export"));
    importInstalledPB->setIcon(KIcon("document-import"));

    KConfig config("KPackageKit");
    KConfigGroup viewGroup(&config, "ViewGroup");
    m_showPackageVersion = new QAction(i18n("Show Versions"), this);
    m_showPackageVersion->setCheckable(true);
    connect(m_showPackageVersion, SIGNAL(toggled(bool)),
            this, SLOT(showVersions(bool)));
    m_showPackageVersion->setChecked(viewGroup.readEntry("ShowApplicationVersions", false));
    showVersions(m_showPackageVersion->isChecked());
    // Arch
    m_showPackageArch = new QAction(i18n("Show Architectures"), this);
    m_showPackageArch->setCheckable(true);
    connect(m_showPackageArch, SIGNAL(toggled(bool)),
            this, SLOT(showArchs(bool)));
    m_showPackageArch->setChecked(viewGroup.readEntry("ShowApplicationVersions", false));
    showArchs(m_showPackageArch->isChecked());

    // Ensure the index is visible when the packageDetails appears
    connect(packageDetails, SIGNAL(ensureVisible(const QModelIndex &)),
            this, SLOT(ensureVisible(const QModelIndex &)));
}

BrowseView::~BrowseView()
{
    KConfig config("KPackageKit");
    KConfigGroup viewGroup(&config, "ViewGroup");
    viewGroup.writeEntry("ShowApplicationVersions", m_showPackageVersion->isChecked());
    viewGroup.writeEntry("ShowApplicationArchitectures", m_showPackageArch->isChecked());
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
    packageDetails->hidePackageVersion(enabled);
}

void BrowseView::showArchs(bool enabled)
{
    packageView->header()->setSectionHidden(2, !enabled);
    packageDetails->hidePackageArch(enabled);
}

void BrowseView::on_packageView_customContextMenuRequested(const QPoint &pos)
{
    KMenu *menu = new KMenu(this);
    menu->addAction(m_showPackageVersion);
    menu->addAction(m_showPackageArch);
    menu->exec(packageView->mapToGlobal(pos));
    delete menu;
}

void BrowseView::on_packageView_activated(const QModelIndex &index)
{
    if (index.column() == 4) {
        return;
    }

    QModelIndex origIndex = m_proxy->mapToSource(index);
    packageDetails->setPackage(origIndex);
}

void BrowseView::ensureVisible(const QModelIndex &index)
{
    QModelIndex proxIndex = m_proxy->mapFromSource(index);
    packageView->scrollTo(proxIndex);
}

void BrowseView::showInstalledPanel(bool visible)
{
    installedF->setVisible(visible);
}

KCategorizedSortFilterProxyModel* BrowseView::proxy() const
{
    return m_proxy;
}

KPixmapSequenceOverlayPainter* BrowseView::busyCursor() const
{
    return m_busySeq;
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
    packageDetails->hide();
    QModelIndex index = categoryView->rootIndex();
    if (index.parent().isValid()) {
        setParentCategory(index.parent());
        emit categoryActivated(index.parent());
        return false;
    }
    return true;
}

void BrowseView::on_categoryMvLeft_clicked()
{
    categoryView->horizontalScrollBar()->setValue(categoryView->horizontalScrollBar()->value() - 1);
}

void BrowseView::on_categoryMvRight_clicked()
{
    categoryView->horizontalScrollBar()->setValue(categoryView->horizontalScrollBar()->value() + 1);
}

void BrowseView::cleanUi()
{
    packageDetails->hide();
    categoryF->setVisible(false);
}

void BrowseView::on_exportInstalledPB_clicked()
{
    // We will assume the installed model
    // is populated since the user is seeing it.
    QString fileName;
    fileName = KFileDialog::getSaveFileName(KUrl(),
                                            "*.catalog",
                                            this,
                                            QString(),
                                            KFileDialog::ConfirmOverwrite);
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
                                  KpkPackageModel::PackageName).toString();
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

#include "BrowseView.moc"
