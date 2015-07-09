/***************************************************************************
 *   Copyright (C) 2009-2010 by Daniel Nicoletti                           *
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

#include "BrowseView.h"

#include "PackageDetails.h"
#include "CategoryModel.h"

#include <ApplicationsDelegate.h>
#include <ApplicationSortFilterModel.h>
#include <PackageModel.h>

#include <Daemon>

#include <KFileDialog>
#include <KPixmapSequence>
#include <KMenu>
#include <KIconLoader>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QAbstractItemView>
#include <QScrollBar>

#include <KDebug>

using namespace PackageKit;

BrowseView::BrowseView(QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);
    connect(categoryView, SIGNAL(clicked(QModelIndex)),
            this, SIGNAL(categoryActivated(QModelIndex)));

    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(packageView->viewport());

    m_model = new PackageModel(this);
    m_proxy = new ApplicationSortFilterModel(this);
    m_proxy->setSourceModel(m_model);

    packageView->setModel(m_proxy);
    packageView->sortByColumn(PackageModel::NameCol, Qt::AscendingOrder);
    packageView->header()->setDefaultAlignment(Qt::AlignCenter);
    packageView->header()->setStretchLastSection(false);
    packageView->header()->setResizeMode(PackageModel::NameCol, QHeaderView::Stretch);
    packageView->header()->setResizeMode(PackageModel::VersionCol, QHeaderView::ResizeToContents);
    packageView->header()->setResizeMode(PackageModel::ArchCol, QHeaderView::ResizeToContents);
    packageView->header()->setResizeMode(PackageModel::OriginCol, QHeaderView::ResizeToContents);
    packageView->header()->setResizeMode(PackageModel::SizeCol, QHeaderView::ResizeToContents);
    packageView->header()->setResizeMode(PackageModel::ActionCol, QHeaderView::ResizeToContents);

    // Hide current Version since it's useless for us
    packageView->header()->setSectionHidden(PackageModel::CurrentVersionCol, true);

    ApplicationsDelegate *delegate = new ApplicationsDelegate(packageView);
    packageView->setItemDelegate(delegate);

    exportInstalledPB->setIcon(KIcon("document-export"));
    importInstalledPB->setIcon(KIcon("document-import"));

    KConfig config("apper");
    KConfigGroup viewGroup(&config, "BrowseView");

    // Version
    packageView->header()->setSectionHidden(PackageModel::VersionCol, true);
    m_showPackageVersion = new QAction(i18n("Show Versions"), this);
    m_showPackageVersion->setCheckable(true);
    connect(m_showPackageVersion, SIGNAL(toggled(bool)), this, SLOT(showVersions(bool)));
    m_showPackageVersion->setChecked(viewGroup.readEntry("ShowApplicationVersions", true));

    // Arch
    packageView->header()->setSectionHidden(PackageModel::ArchCol, true);
    m_showPackageArch = new QAction(i18n("Show Architectures"), this);
    m_showPackageArch->setCheckable(true);
    connect(m_showPackageArch, SIGNAL(toggled(bool)), this, SLOT(showArchs(bool)));
    m_showPackageArch->setChecked(viewGroup.readEntry("ShowApplicationArchitectures", false));

    // Origin
    packageView->header()->setSectionHidden(PackageModel::OriginCol, true);
    m_showPackageOrigin = new QAction(i18n("Show Origins"), this);
    m_showPackageOrigin->setCheckable(true);
    connect(m_showPackageOrigin, SIGNAL(toggled(bool)), this, SLOT(showOrigins(bool)));
    m_showPackageOrigin->setChecked(viewGroup.readEntry("ShowApplicationOrigins", false));

    // Sizes
    packageView->header()->setSectionHidden(PackageModel::SizeCol, true);
    m_showPackageSizes = new QAction(i18n("Show Sizes"), this);
    m_showPackageSizes->setCheckable(true);
    connect(m_showPackageSizes, SIGNAL(toggled(bool)), this, SLOT(showSizes(bool)));
    m_showPackageSizes->setChecked(viewGroup.readEntry("ShowPackageSizes", false));


    // Ensure the index is visible when the packageDetails appears
    connect(packageDetails, SIGNAL(ensureVisible(QModelIndex)),
            this, SLOT(ensureVisible(QModelIndex)));
}

void BrowseView::init(Transaction::Roles roles)
{
    packageDetails->init(roles);
}

BrowseView::~BrowseView()
{
}

bool BrowseView::showPageHeader() const
{
    return false;
}

PackageModel* BrowseView::model() const
{
    return m_model;
}

void BrowseView::showVersions(bool enabled)
{
    KConfig config("apper");
    KConfigGroup viewGroup(&config, "BrowseView");
    viewGroup.writeEntry("ShowApplicationVersions", enabled);
    packageView->header()->setSectionHidden(PackageModel::VersionCol, !enabled);
    packageDetails->hidePackageVersion(enabled);
}

void BrowseView::showArchs(bool enabled)
{
    KConfig config("apper");
    KConfigGroup viewGroup(&config, "BrowseView");
    viewGroup.writeEntry("ShowApplicationArchitectures", enabled);
    packageView->header()->setSectionHidden(PackageModel::ArchCol, !enabled);
    packageDetails->hidePackageArch(enabled);
}

void BrowseView::showOrigins(bool enabled)
{
    KConfig config("apper");
    KConfigGroup viewGroup(&config, "BrowseView");
    viewGroup.writeEntry("ShowApplicationOrigins", enabled);
    packageView->header()->setSectionHidden(PackageModel::OriginCol, !enabled);
}

void BrowseView::showSizes(bool enabled)
{
    KConfig config("apper");
    KConfigGroup viewGroup(&config, "BrowseView");
    viewGroup.writeEntry("ShowPackageSizes", enabled);
    packageView->header()->setSectionHidden(PackageModel::SizeCol, !enabled);
    packageDetails->hidePackageArch(enabled);
    if (enabled) {
        m_model->fetchSizes();
    }
}

void BrowseView::on_packageView_customContextMenuRequested(const QPoint &pos)
{
    KMenu *menu = new KMenu(this);
    menu->addAction(m_showPackageVersion);
    menu->addAction(m_showPackageArch);
    menu->addAction(m_showPackageOrigin);
    menu->addAction(m_showPackageSizes);
    menu->exec(packageView->viewport()->mapToGlobal(pos));
    menu->deleteLater();
}

void BrowseView::on_packageView_clicked(const QModelIndex &index)
{
    if (index.column() == PackageModel::ActionCol) {
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

ApplicationSortFilterModel* BrowseView::proxy() const
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
        index = index.parent();
        // if it's valid we need to know if it wasn't a  PK root category
        if (index.data(CategoryModel::GroupRole).type() == QVariant::String) {
            QString category = index.data(CategoryModel::GroupRole).toString();
            if (!category.startsWith('@')) {
                return true;
            }
        }
        setParentCategory(index);
        emit categoryActivated(index);
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

bool BrowseView::isShowingSizes() const
{
    return m_showPackageSizes->isChecked();
}

void BrowseView::on_exportInstalledPB_clicked()
{
    // We will assume the installed model
    // is populated since the user is seeing it.
    QString fileName;
    fileName = KFileDialog::getSaveFileName(QUrl(),
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
    out << "InstallPackages(" << Daemon::global()->distroID() << ")=";
    QStringList packages;
    for (int i = 0; i < m_model->rowCount(); i++) {
        packages << m_model->data(m_model->index(i, 0),
                                  PackageModel::PackageName).toString();
    }
    out << packages.join(";");
}

void BrowseView::on_importInstalledPB_clicked()
{
    QString fileName;
    fileName = KFileDialog::getOpenFileName(QUrl(), "*.catalog", this);
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
