/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
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

#include "Updater.h"
#include "ui_Updater.h"

#include "UpdateDetails.h"
#include "DistroUpgrade.h"
#include "CheckableHeader.h"

#include <KMenu>
#include <KGenericFactory>
#include <KPixmapSequence>
#include <KAboutData>

#include <ApplicationsDelegate.h>
#include <PkStrings.h>
#include <PkIcons.h>
#include <PackageModel.h>
#include <ApplicationSortFilterModel.h>

#include <QDBusConnection>

#include <KGlobalSettings>
#include <KMessageBox>
#include <KDebug>

#include <Daemon>

Updater::Updater(Transaction::Roles roles, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Updater),
    m_roles(roles),
    m_selected(true),
    m_updatesT(0)
{
    ui->setupUi(this);
    updatePallete();
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
            this, SLOT(updatePallete()));

    m_updatesModel = new PackageModel(this);
    m_updatesModel->setCheckable(true);
    ApplicationSortFilterModel *proxyModel = new ApplicationSortFilterModel(this);
    proxyModel->setSourceModel(m_updatesModel);
    ui->packageView->setModel(proxyModel);

    m_delegate = new ApplicationsDelegate(ui->packageView);
    m_delegate->setCheckable(true);
    ui->packageView->setItemDelegate(m_delegate);
    ui->packageView->sortByColumn(PackageModel::NameCol, Qt::AscendingOrder);
    connect(m_updatesModel, SIGNAL(changed(bool)),
            this, SLOT(checkEnableUpdateButton()));

    //initialize the model, delegate, client and  connect it's signals
    m_header = new CheckableHeader(Qt::Horizontal, this);
    connect(m_header, SIGNAL(toggled(bool)),
            m_updatesModel, SLOT(setAllChecked(bool)));
    m_header->setCheckBoxVisible(false);
    m_header->setDefaultAlignment(Qt::AlignCenter);
    ui->packageView->setHeaderHidden(false);
    ui->packageView->setHeader(m_header);

    // This must be set AFTER the model is set, otherwise it doesn't work
    m_header->setResizeMode(PackageModel::NameCol, QHeaderView::Stretch);
    m_header->setResizeMode(PackageModel::VersionCol, QHeaderView::ResizeToContents);
    m_header->setResizeMode(PackageModel::CurrentVersionCol, QHeaderView::ResizeToContents);
    m_header->setResizeMode(PackageModel::ArchCol, QHeaderView::ResizeToContents);
    m_header->setResizeMode(PackageModel::OriginCol, QHeaderView::ResizeToContents);
    m_header->setResizeMode(PackageModel::SizeCol, QHeaderView::ResizeToContents);
    m_header->setStretchLastSection(false);

    // Setup the busy cursor
    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(ui->packageView->viewport());

    // hide distro Upgrade container and line
    ui->distroUpgrade->hide();

    KConfig config("apper");
    KConfigGroup viewGroup(&config, "UpdateView");

    // versions
    ui->packageView->header()->setSectionHidden(PackageModel::VersionCol, true);
    m_showPackageVersion = new QAction(i18n("Show Versions"), this);
    m_showPackageVersion->setCheckable(true);
    connect(m_showPackageVersion, SIGNAL(toggled(bool)), this, SLOT(showVersions(bool)));
    m_showPackageVersion->setChecked(viewGroup.readEntry("ShowVersions", true));

    // versions
    ui->packageView->header()->setSectionHidden(PackageModel::CurrentVersionCol, true);
    m_showPackageCurrentVersion = new QAction(i18n("Show Current Versions"), this);
    m_showPackageCurrentVersion->setCheckable(true);
    connect(m_showPackageCurrentVersion, SIGNAL(toggled(bool)), this, SLOT(showCurrentVersions(bool)));
    m_showPackageCurrentVersion->setChecked(viewGroup.readEntry("ShowCurrentVersions", false));

    // Arch
    ui->packageView->header()->setSectionHidden(PackageModel::ArchCol, true);
    m_showPackageArch = new QAction(i18n("Show Architectures"), this);
    m_showPackageArch->setCheckable(true);
    connect(m_showPackageArch, SIGNAL(toggled(bool)), this, SLOT(showArchs(bool)));
    m_showPackageArch->setChecked(viewGroup.readEntry("ShowArchs", false));

    // Origin
    ui->packageView->header()->setSectionHidden(PackageModel::OriginCol, true);
    m_showPackageOrigin = new QAction(i18n("Show Origins"), this);
    m_showPackageOrigin->setCheckable(true);
    connect(m_showPackageOrigin, SIGNAL(toggled(bool)), this, SLOT(showOrigins(bool)));
    m_showPackageOrigin->setChecked(viewGroup.readEntry("ShowOrigins", false));

    // Sizes
    ui->packageView->header()->setSectionHidden(PackageModel::SizeCol, true);
    m_showPackageSize = new QAction(i18n("Show Sizes"), this);
    m_showPackageSize->setCheckable(true);
    connect(m_showPackageSize, SIGNAL(toggled(bool)), this, SLOT(showSizes(bool)));
    m_showPackageSize->setChecked(viewGroup.readEntry("ShowSizes", true));
}

Updater::~Updater()
{
    delete ui;
}

void Updater::showVersions(bool enabled)
{
    KConfig config("apper");
    KConfigGroup viewGroup(&config, "UpdateView");
    viewGroup.writeEntry("ShowVersions", enabled);
    ui->packageView->header()->setSectionHidden(PackageModel::VersionCol, !enabled);
}

void Updater::showCurrentVersions(bool enabled)
{
    KConfig config("apper");
    KConfigGroup viewGroup(&config, "UpdateView");
    viewGroup.writeEntry("ShowCurrentVersions", enabled);
    ui->packageView->header()->setSectionHidden(PackageModel::CurrentVersionCol, !enabled);
    if (enabled) {
        m_updatesModel->fetchCurrentVersions();
    }
}

void Updater::showArchs(bool enabled)
{
    KConfig config("apper");
    KConfigGroup viewGroup(&config, "UpdateView");
    viewGroup.writeEntry("ShowArchs", enabled);
    ui->packageView->header()->setSectionHidden(PackageModel::ArchCol, !enabled);
}

void Updater::showOrigins(bool enabled)
{
    KConfig config("apper");
    KConfigGroup viewGroup(&config, "UpdateView");
    viewGroup.writeEntry("showOrigins", enabled);
    ui->packageView->header()->setSectionHidden(PackageModel::OriginCol, !enabled);
}

void Updater::showSizes(bool enabled)
{
    KConfig config("apper");
    KConfigGroup viewGroup(&config, "UpdateView");
    viewGroup.writeEntry("ShowSizes", enabled);
    ui->packageView->header()->setSectionHidden(PackageModel::SizeCol, !enabled);
    if (enabled) {
        m_updatesModel->fetchSizes();
    }
}

void Updater::updatePallete()
{
    QPalette pal;
    pal.setColor(QPalette::Window, pal.base().color());
    pal.setColor(QPalette::WindowText, pal.text().color());
    ui->backgroundFrame->setPalette(pal);
}

void Updater::on_packageView_clicked(const QModelIndex &index)
{
    QString pkgId = index.data(PackageModel::IdRole).toString();
    Transaction::Info pkgInfo = index.data(PackageModel::InfoRole).value<Transaction::Info>();
    ui->updateDetails->setPackage(pkgId, pkgInfo);
}

//TODO: We should add some kind of configuration to let users show unstable distributions
//That way, by default, users only see stable ones.
void Updater::distroUpgrade(PackageKit::Transaction::DistroUpgrade type, const QString &name, const QString &description)
{
    // TODO name should be used to do a upgrade to a different type
    Q_UNUSED(name)
    if (type != Transaction::DistroUpgradeStable) {
        // Ignore unstable distros upgrades for now
        return;
    }

    ui->distroUpgrade->setName(description);
    ui->distroUpgrade->animatedShow();
}

bool Updater::hasChanges() const
{
    return m_updatesModel->hasChanges();
}

void Updater::checkEnableUpdateButton()
{
    emit changed(hasChanges());
    int selectedSize = m_updatesModel->selectedPackagesToInstall().size();
    int updatesSize = m_updatesModel->rowCount();
    if (selectedSize == 0) {
        m_header->setCheckState(Qt::Unchecked);
    } else if (selectedSize == updatesSize) {
        m_header->setCheckState(Qt::Checked);
    } else {
        m_header->setCheckState(Qt::PartiallyChecked);
    }

    unsigned long dwSize = m_updatesModel->downloadSize();
    if (dwSize) {
        emit downloadSize(i18n("Estimated download size: %1",
                               KGlobal::locale()->formatByteSize(dwSize)));
    } else {
        emit downloadSize(QString());
    }
    // if we don't have any upates let's disable the button
    m_header->setCheckBoxVisible(m_updatesModel->rowCount() != 0);
    ui->packageView->setHeaderHidden(m_updatesModel->rowCount() == 0);
}

void Updater::load()
{
    // set focus on the updates view
    ui->packageView->setFocus(Qt::OtherFocusReason);
    emit downloadSize(QString());
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
    if (m_updatesModel->rowCount() == 0) {
        // Set the info page
        ui->stackedWidget->setCurrentIndex(1);
        uint lastTime = Daemon::global()->getTimeSinceAction(Transaction::RoleRefreshCache);
        ui->titleL->setText(PkStrings::lastCacheRefreshTitle(lastTime));
        ui->descriptionL->setText(PkStrings::lastCacheRefreshSubTitle(lastTime));
        ui->iconL->setPixmap(KIcon(PkIcons::lastCacheRefreshIconName(lastTime)).pixmap(128, 128));
    }
}

QStringList Updater::packagesToUpdate() const
{
    return m_updatesModel->selectedPackagesToInstall();
}

void Updater::getUpdates()
{
    if (m_updatesT) {
        // There is a getUpdates running ignore this call
        return;
    }

    if (ui->stackedWidget->currentIndex() != 0) {
        ui->stackedWidget->setCurrentIndex(0);
    }

    // clears the model
    ui->packageView->setHeaderHidden(true);
    m_updatesModel->clear();
    ui->updateDetails->hide();
    m_updatesT = Daemon::getUpdates();
    connect(m_updatesT, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            m_updatesModel, SLOT(addSelectedPackage(PackageKit::Transaction::Info,QString,QString)));
    connect(m_updatesT, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            this, SLOT(errorCode(PackageKit::Transaction::Error,QString)));
    connect(m_updatesT, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            m_busySeq, SLOT(stop()));
    connect(m_updatesT, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            m_updatesModel, SLOT(finished()));
    // This is required to estimate download size
    connect(m_updatesT, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            m_updatesModel, SLOT(fetchSizes()));
    if (m_showPackageCurrentVersion->isChecked()) {
        connect(m_updatesT, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                m_updatesModel, SLOT(fetchCurrentVersions()));
    }
    connect(m_updatesT, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(getUpdatesFinished()));
    m_busySeq->start();

    // Hide the distribution upgrade information
    ui->distroUpgrade->animatedHide();

    if (m_roles & Transaction::RoleGetDistroUpgrades) {
        // Check for distribution Upgrades
        Transaction *t = Daemon::getDistroUpgrades();
        connect(t, SIGNAL(distroUpgrade(PackageKit::Transaction::DistroUpgrade,QString,QString)),
                this, SLOT(distroUpgrade(PackageKit::Transaction::DistroUpgrade,QString,QString)));
        connect(t, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                t, SLOT(deleteLater()));
    }
}

void Updater::on_packageView_customContextMenuRequested(const QPoint &pos)
{
    KMenu *menu = new KMenu(this);
    menu->addAction(m_showPackageVersion);
    menu->addAction(m_showPackageCurrentVersion);
    menu->addAction(m_showPackageArch);
    menu->addAction(m_showPackageOrigin);
    menu->addAction(m_showPackageSize);
    QAction *action;
    action = menu->addAction(i18n("Check for new updates"));
    action->setIcon(KIcon("view-refresh"));
    connect(action, SIGNAL(triggered(bool)),
            this, SIGNAL(refreshCache()));
    menu->exec(ui->packageView->viewport()->mapToGlobal(pos));
    delete menu;
}

void Updater::errorCode(PackageKit::Transaction::Error error, const QString &details)
{
    KMessageBox::detailedSorry(this,
                               PkStrings::errorMessage(error),
                               details,
                               PkStrings::error(error),
                               KMessageBox::Notify);
}

#include "Updater.moc"
