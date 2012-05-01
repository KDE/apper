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

#include "UpdateDetails.h"
#include "DistroUpgrade.h"
#include "CheckableHeader.h"

#include <version.h>

#include <KMenu>
#include <KGenericFactory>
#include <KPixmapSequence>
#include <KAboutData>

#include <ApplicationsDelegate.h>
#include <PkStrings.h>
#include <PkIcons.h>
#include <PackageModel.h>

#include <QDBusConnection>

#include <KCategorizedSortFilterProxyModel>
#include <KGlobalSettings>
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
    updatePallete();
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
            this, SLOT(updatePallete()));

    m_updatesModel = new PackageModel(this);
    m_updatesModel->setCheckable(true);
    KCategorizedSortFilterProxyModel *proxyModel = new KCategorizedSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_updatesModel);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setCategorizedModel(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setSortRole(PackageModel::SortRole);
    proxyModel->setFilterRole(PackageModel::ApplicationFilterRole);
    packageView->setModel(proxyModel);

    m_delegate = new ApplicationsDelegate(packageView);
    m_delegate->setCheckable(true);
    packageView->setItemDelegate(m_delegate);
    packageView->sortByColumn(PackageModel::NameCol, Qt::AscendingOrder);
    connect(m_updatesModel, SIGNAL(changed(bool)),
            this, SLOT(checkEnableUpdateButton()));

    //initialize the model, delegate, client and  connect it's signals
    m_header = new CheckableHeader(Qt::Horizontal, this);
    connect(m_header, SIGNAL(toggled(bool)),
            m_updatesModel, SLOT(setAllChecked(bool)));
    m_header->setCheckBoxVisible(false);
    m_header->setDefaultAlignment(Qt::AlignCenter);
    packageView->setHeaderHidden(false);
    packageView->setHeader(m_header);

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
    m_busySeq->setWidget(packageView->viewport());

    // hide distro Upgrade container and line
    distroUpgradesSA->hide();
    line->hide();

    KConfig config("apper");
    KConfigGroup viewGroup(&config, "UpdateView");

    // versions
    packageView->header()->setSectionHidden(PackageModel::VersionCol, true);
    m_showPackageVersion = new QAction(i18n("Show Versions"), this);
    m_showPackageVersion->setCheckable(true);
    connect(m_showPackageVersion, SIGNAL(toggled(bool)), this, SLOT(showVersions(bool)));
    m_showPackageVersion->setChecked(viewGroup.readEntry("ShowVersions", true));

    // versions
    packageView->header()->setSectionHidden(PackageModel::CurrentVersionCol, true);
    m_showPackageCurrentVersion = new QAction(i18n("Show Current Versions"), this);
    m_showPackageCurrentVersion->setCheckable(true);
    connect(m_showPackageCurrentVersion, SIGNAL(toggled(bool)), this, SLOT(showCurrentVersions(bool)));
    m_showPackageCurrentVersion->setChecked(viewGroup.readEntry("ShowCurrentVersions", false));

    // Arch
    packageView->header()->setSectionHidden(PackageModel::ArchCol, true);
    m_showPackageArch = new QAction(i18n("Show Architectures"), this);
    m_showPackageArch->setCheckable(true);
    connect(m_showPackageArch, SIGNAL(toggled(bool)), this, SLOT(showArchs(bool)));
    m_showPackageArch->setChecked(viewGroup.readEntry("ShowArchs", false));

    // Origin
    packageView->header()->setSectionHidden(PackageModel::OriginCol, true);
    m_showPackageOrigin = new QAction(i18n("Show Origins"), this);
    m_showPackageOrigin->setCheckable(true);
    connect(m_showPackageOrigin, SIGNAL(toggled(bool)), this, SLOT(showOrigins(bool)));
    m_showPackageOrigin->setChecked(viewGroup.readEntry("ShowOrigins", false));

    // Sizes
    packageView->header()->setSectionHidden(PackageModel::SizeCol, true);
    m_showPackageSize = new QAction(i18n("Show Sizes"), this);
    m_showPackageSize->setCheckable(true);
    connect(m_showPackageSize, SIGNAL(toggled(bool)), this, SLOT(showSizes(bool)));
    m_showPackageSize->setChecked(viewGroup.readEntry("ShowSizes", true));
}

Updater::~Updater()
{
}

void Updater::setSelected(bool selected)
{
    m_selected = selected;
}

void Updater::showVersions(bool enabled)
{
    KConfig config("apper");
    KConfigGroup viewGroup(&config, "UpdateView");
    viewGroup.writeEntry("ShowVersions", enabled);
    packageView->header()->setSectionHidden(PackageModel::VersionCol, !enabled);
}

void Updater::showCurrentVersions(bool enabled)
{
    KConfig config("apper");
    KConfigGroup viewGroup(&config, "UpdateView");
    viewGroup.writeEntry("ShowCurrentVersions", enabled);
    packageView->header()->setSectionHidden(PackageModel::CurrentVersionCol, !enabled);
    if (enabled) {
        m_updatesModel->fetchCurrentVersions();
    }
}

void Updater::showArchs(bool enabled)
{
    KConfig config("apper");
    KConfigGroup viewGroup(&config, "UpdateView");
    viewGroup.writeEntry("ShowArchs", enabled);
    packageView->header()->setSectionHidden(PackageModel::ArchCol, !enabled);
}

void Updater::showOrigins(bool enabled)
{
    KConfig config("apper");
    KConfigGroup viewGroup(&config, "UpdateView");
    viewGroup.writeEntry("showOrigins", enabled);
    packageView->header()->setSectionHidden(PackageModel::OriginCol, !enabled);
}

void Updater::showSizes(bool enabled)
{
    KConfig config("apper");
    KConfigGroup viewGroup(&config, "UpdateView");
    viewGroup.writeEntry("ShowSizes", enabled);
    packageView->header()->setSectionHidden(PackageModel::SizeCol, !enabled);
    if (enabled) {
        m_updatesModel->fetchSizes();
    }
}

void Updater::updatePallete()
{
    QPalette pal;
    pal.setColor(QPalette::Window, pal.base().color());
    backgroundFrame->setPalette(pal);
}

void Updater::on_packageView_clicked(const QModelIndex &index)
{
    QString    pkgId   = index.data(PackageModel::IdRole).toString();
    Package::Info pkgInfo = static_cast<Package::Info>(index.data(PackageModel::InfoRole).toUInt());
    updateDetails->setPackage(pkgId, pkgInfo);
}

//TODO: We should add some kind of configuration to let users show unstable distributions
//That way, by default, users only see stable ones.
void Updater::distroUpgrade(PackageKit::Transaction::DistroUpgrade type, const QString &name, const QString &description)
{
    if (type != Transaction::DistroUpgradeStable) {
        // Ignore unstable distros upgrades for now
        return;
    }

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

    unsigned long dwSize = m_updatesModel->downloadSize();
    if (dwSize) {
        emit downloadSize(i18n("Estimated download size: %1",
                               KGlobal::locale()->formatByteSize(dwSize)));
    } else {
        emit downloadSize(QString());
    }
    // if we don't have any upates let's disable the button
    m_header->setCheckBoxVisible(m_updatesModel->rowCount() != 0);
    packageView->setHeaderHidden(m_updatesModel->rowCount() == 0);
}

void Updater::load()
{
    // set focus on the updates view
    packageView->setFocus(Qt::OtherFocusReason);
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
        stackedWidget->setCurrentIndex(1);
        uint lastTime = Daemon::getTimeSinceAction(Transaction::RoleRefreshCache);
        unsigned long fifteen = 60 * 60 * 24 * 15;
        unsigned long tirty = 60 * 60 * 24 * 30;

        if (lastTime < fifteen) {
            titleL->setText(i18n("Your system is up to date"));
            descriptionL->setText(i18n("Verified %1 ago", KGlobal::locale()->prettyFormatDuration(lastTime * 1000)));
            iconL->setPixmap(KIcon("security-high").pixmap(128, 128));
        } else if (lastTime > fifteen && lastTime < tirty && lastTime != UINT_MAX) {
            titleL->setText(i18n("You have no updates"));
            descriptionL->setText(i18n("Verified %1 ago", KGlobal::locale()->prettyFormatDuration(lastTime * 1000)));
            iconL->setPixmap(KIcon("security-medium").pixmap(128, 128));
        } else {
            titleL->setText(i18n("Last check for updates was more than a month ago"));
            descriptionL->setText(i18n("It's strongly recommended that you check for new updates now"));
            iconL->setPixmap(KIcon("security-low").pixmap(128, 128));
        }
    }
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

    if (stackedWidget->currentIndex() != 0) {
        stackedWidget->setCurrentIndex(0);
    }

    // clears the model
    packageView->setHeaderHidden(true);
    m_updatesModel->clear();
    updateDetails->hide();
    m_updatesT = new Transaction(this);
    if (m_selected) {
        connect(m_updatesT, SIGNAL(package(PackageKit::Package)),
                m_updatesModel, SLOT(addSelectedPackage(PackageKit::Package)));
    } else {
        connect(m_updatesT, SIGNAL(package(PackageKit::Package)),
                m_updatesModel, SLOT(addPackage(PackageKit::Package)));
    }
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
    // get all updates
    m_updatesT->getUpdates();

    Transaction::InternalError error = m_updatesT->error();
    if (error) {
        disconnect(m_updatesT, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   this, SLOT(getUpdatesFinished()));
        disconnect(m_updatesT, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   m_busySeq, SLOT(stop()));
        disconnect(m_updatesT, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   m_updatesModel, SLOT(finished()));
        disconnect(m_updatesT, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   m_updatesModel, SLOT(fetchSizes()));
        m_updatesT = 0;
        KMessageBox::sorry(this, PkStrings::daemonError(error));
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
        connect(t, SIGNAL(distroUpgrade(PackageKit::Transaction::DistroUpgrade,QString,QString)),
                this, SLOT(distroUpgrade(PackageKit::Transaction::DistroUpgrade,QString,QString)));
        t->getDistroUpgrades();
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
    action = menu->addAction(i18n("Check for new Updates"));
    action->setIcon(KIcon("view-refresh"));
    connect(action, SIGNAL(triggered(bool)),
            this, SIGNAL(refreshCache()));
    menu->exec(packageView->viewport()->mapToGlobal(pos));
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
