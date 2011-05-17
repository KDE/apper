/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
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

#include "Updater.h"

#include "UpdateDetails.h"
#include "DistroUpgrade.h"
#include "CheckableHeader.h"
#include "KpkMacros.h"

#include <version.h>

#include <KMenu>
#include <KGenericFactory>
#include <KPixmapSequence>
#include <KAboutData>

#include <ApplicationsDelegate.h>
#include <KpkStrings.h>
#include <KpkIcons.h>
#include <KpkSimulateModel.h>
#include <KpkRequirements.h>
#include <KpkPackageModel.h>

#include <QSortFilterProxyModel>
#include <QDBusConnection>

#include <KMessageBox>
#include <KDebug>

#include <Daemon>

Updater::Updater(Transaction::Roles roles, QWidget *parent) :
    QWidget(parent),
    m_roles(roles),
    m_selected(false),
    m_updatesT(0)
{
    setupUi(this);

    //initialize the model, delegate, client and  connect it's signals
    m_header = new CheckableHeader(Qt::Horizontal, this);
    m_header->setCheckBoxVisible(false);
    packageView->setHeader(m_header);
    packageView->setHeaderHidden(true);

    m_updatesModel = new KpkPackageModel(this);
    m_updatesModel->setCheckable(true);
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_updatesModel);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setSortRole(KpkPackageModel::SortRole);
    packageView->setModel(proxyModel);

    m_delegate = new ApplicationsDelegate(packageView);
    m_delegate->setCheckable(true);
    packageView->setItemDelegate(m_delegate);
    packageView->sortByColumn(0, Qt::AscendingOrder);
    connect(m_header, SIGNAL(toggled(bool)),
            m_updatesModel, SLOT(setAllChecked(bool)));
    connect(m_updatesModel, SIGNAL(changed(bool)),
            this, SLOT(checkEnableUpdateButton()));

    // This must be set AFTER the model is set, otherwise it doesn't work
    m_header->setResizeMode(0, QHeaderView::ResizeToContents);
    m_header->setResizeMode(1, QHeaderView::ResizeToContents);
    m_header->setStretchLastSection(true);

    // Setup the busy cursor
    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(packageView->viewport());

    // hide distro Upgrade container and line
    distroUpgradesSA->hide();
    line->hide();

    KConfig config("KPackageKit");
    KConfigGroup viewGroup(&config, "ViewGroup");
    m_showPackageVersion = new QAction(i18n("Show Versions"), this);
    m_showPackageVersion->setCheckable(true);
    connect(m_showPackageVersion, SIGNAL(toggled(bool)),
            this, SLOT(showVersions(bool)));
    m_showPackageVersion->setChecked(viewGroup.readEntry("ShowVersions", false));
    showVersions(m_showPackageVersion->isChecked());

    // Arch
    m_showPackageArch = new QAction(i18n("Show Architectures"), this);
    m_showPackageArch->setCheckable(true);
    connect(m_showPackageArch, SIGNAL(toggled(bool)),
            this, SLOT(showArchs(bool)));
    m_showPackageArch->setChecked(viewGroup.readEntry("ShowArchs", false));
    showArchs(m_showPackageArch->isChecked());
}

Updater::~Updater()
{
    KConfig config("KPackageKit");
    KConfigGroup viewGroup(&config, "ViewGroup");
    viewGroup.writeEntry("ShowVersions", m_showPackageVersion->isChecked());
    viewGroup.writeEntry("ShowArchs", m_showPackageArch->isChecked());
}

void Updater::setSelected(bool selected)
{
    m_selected = selected;
}

void Updater::showVersions(bool enabled)
{
    packageView->header()->setSectionHidden(1, !enabled);
}

void Updater::showArchs(bool enabled)
{
    packageView->header()->setSectionHidden(2, !enabled);
}

void Updater::on_packageView_clicked(const QModelIndex &index)
{
    QString    pkgId   = index.data(KpkPackageModel::IdRole).toString();
    Package::Info pkgInfo = static_cast<Package::Info>(index.data(KpkPackageModel::InfoRole).toUInt());
    updateDetails->setPackage(pkgId, pkgInfo);
}

//TODO: We should add some kind of configuration to let users show unstable distributions
//That way, by default, users only see stable ones.
void Updater::distroUpgrade(PackageKit::Transaction::DistroUpgrade type, const QString &name, const QString &description)
{
    Q_UNUSED(type)
    if (verticalLayout->count()) {
        QFrame *frame = new QFrame(this);
        frame->setFrameShape(QFrame::HLine);
        verticalLayout->insertWidget(0, frame);
    }
    DistroUpgrade *distro = new DistroUpgrade(this);
    verticalLayout->insertWidget(0, distro);
    distro->setComment(description);
    distro->setName(name);
    distroUpgradesSA->show();
    line->show();
}

bool Updater::hasChanges() const
{
    return m_updatesModel->hasChanges();
}

void Updater::checkEnableUpdateButton()
{
    emit changed(hasChanges());
    int selectedSize = m_updatesModel->selectedPackages().size();
    int updatesSize = m_updatesModel->rowCount();
    if (selectedSize == 0) {
        m_header->setCheckState(Qt::Unchecked);
    } else if (selectedSize == updatesSize) {
        m_header->setCheckState(Qt::Checked);
    } else {
        m_header->setCheckState(Qt::PartiallyChecked);
    }

    // if we don't have any upates let's disable the button
    m_header->setCheckBoxVisible(m_updatesModel->rowCount() != 0);
    packageView->setHeaderHidden(m_updatesModel->rowCount() == 0);
}

void Updater::load()
{
    // set focus on the updates view
    packageView->setFocus(Qt::OtherFocusReason);
    // If the model already has some packages
    // let's just clear the selection
    if (m_updatesModel->rowCount()) {
        m_updatesModel->setAllChecked(false);
    } else {
        getUpdates();
    }
}

void Updater::getUpdatesFinished()
{
    m_updatesT = 0;
    m_updatesModel->clearSelectedNotPresent();
    checkEnableUpdateButton();
}

QList<Package> Updater::packagesToUpdate() const
{
    return m_updatesModel->selectedPackages();
}

void Updater::getUpdates()
{
    if (m_updatesT) {
        // There is a getUpdates running ignore this call
        return;
    }

    // clears the model
    packageView->setHeaderHidden(true);
    m_updatesModel->clear();
    updateDetails->hide();
    m_updatesT = new Transaction(this);
    if (m_selected) {
        connect(m_updatesT, SIGNAL(package(const PackageKit::Package &)),
                m_updatesModel, SLOT(addSelectedPackage(const PackageKit::Package &)));
    } else {
        connect(m_updatesT, SIGNAL(package(const PackageKit::Package &)),
                m_updatesModel, SLOT(addPackage(const PackageKit::Package &)));
    }
    connect(m_updatesT, SIGNAL(errorCode(PackageKit::Transaction::Error, const QString &)),
            this, SLOT(errorCode(PackageKit::Transaction::Error, const QString &)));
    connect(m_updatesT, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
            this, SLOT(getUpdatesFinished()));
    connect(m_updatesT, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
            m_busySeq, SLOT(stop()));
    connect(m_updatesT, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
            m_updatesModel, SLOT(finished()));
    // get all updates
    m_updatesT->getUpdates();

    if (m_updatesT->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(m_updatesT->error()));
    } else {
        m_busySeq->start();
    }

    // Clean the distribution upgrades area
    QLayoutItem *child;
    while ((child = verticalLayout->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
    }
    distroUpgradesSA->hide();
    line->hide();

    if (m_roles & Transaction::RoleGetDistroUpgrades) {
        // Check for distribution Upgrades
        Transaction *t = new Transaction(this);
        connect(t, SIGNAL(distroUpgrade(PackageKit::Transaction::DistroUpgrade, const QString &, const QString &)),
                this, SLOT(distroUpgrade(PackageKit::Transaction::DistroUpgrade, const QString &, const QString &)));
        t->getDistroUpgrades();
    }
}

void Updater::on_packageView_customContextMenuRequested(const QPoint &pos)
{
    KMenu *menu = new KMenu(this);
    menu->addAction(m_showPackageVersion);
    menu->addAction(m_showPackageArch);
    QAction *action;
    action = menu->addAction(i18n("Check for new Updates"));
    action->setIcon(KIcon("view-refresh"));
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(refreshCache()));
    menu->exec(packageView->mapToGlobal(pos));
    delete menu;
}

void Updater::errorCode(PackageKit::Transaction::Error error, const QString &details)
{
    KMessageBox::detailedSorry(this,
                               KpkStrings::errorMessage(error),
                               details,
                               KpkStrings::error(error),
                               KMessageBox::Notify);
}

#include "Updater.moc"
