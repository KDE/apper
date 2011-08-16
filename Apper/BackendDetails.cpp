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
#include "BackendDetails.h"

#include <KIcon>

#include <Daemon>

using namespace PackageKit;

BackendDetails::BackendDetails(QWidget *parent)
  : KDialog(parent)
{
    setWindowTitle(i18n("Backend Details"));

    setupUi(mainWidget());
    setButtons(KDialog::Close);
    setWindowIcon(KIcon("help-about"));

    // GENERAL - Setup backend name and author
    nameL->setText(Daemon::backendName());
    descriptionL->setText(Daemon::backendDescription());
    authorL->setText(Daemon::backendAuthor());
    distroL->setText(Daemon::distroId());

    // METHODS - Setup backend supported methods
    Transaction::Roles actions = Daemon::actions();
    getUpdatesCB->setChecked(actions & Transaction::RoleGetUpdates);
    getDistroUpgradesCB->setChecked(actions & Transaction::RoleGetDistroUpgrades);
    refreshCacheCB->setChecked(actions & Transaction::RoleRefreshCache);
    updateSystemCB->setChecked(actions & Transaction::RoleUpdateSystem);
    searchNameCB->setChecked(actions & Transaction::RoleSearchName);
    searchDetailsCB->setChecked(actions & Transaction::RoleSearchDetails);
    searchGroupCB->setChecked(actions & Transaction::RoleSearchGroup);
    searchFileCB->setChecked(actions & Transaction::RoleSearchFile);
    cancelCB->setChecked(actions & Transaction::RoleCancel);
    resolveCB->setChecked(actions & Transaction::RoleResolve);

    updatePackageCB->setChecked(actions & Transaction::RoleUpdatePackages);
    installPackageCB->setChecked(actions & Transaction::RoleInstallPackages);
    removePackageCB->setChecked(actions & Transaction::RoleRemovePackages);
    getDependsCB->setChecked(actions & Transaction::RoleGetDepends);
    getRequiresCB->setChecked(actions & Transaction::RoleGetRequires);
    getUpdateDetailCB->setChecked(actions & Transaction::RoleGetUpdateDetail);
    getDescriptionCB->setChecked(actions & Transaction::RoleGetDetails);
    getFilesCB->setChecked(actions & Transaction::RoleRefreshCache);
    installFileCB->setChecked(actions & Transaction::RoleInstallFiles);

    getRepositoryListCB->setChecked(actions & Transaction::RoleGetRepoList);
    repositoryEnableCB->setChecked(actions & Transaction::RoleRepoEnable);
    repositorySetEnableCB->setChecked(actions & Transaction::RoleRepoSetData);
    whatProvidesCB->setChecked(actions & Transaction::RoleWhatProvides);
    getPackagesCB->setChecked(actions & Transaction::RoleGetPackages);
    simulateInstallFilesCB->setChecked(actions & Transaction::RoleSimulateInstallFiles);
    simulateInstallPackagesCB->setChecked(actions & Transaction::RoleSimulateInstallPackages);
    simulateRemovePackagesCB->setChecked(actions & Transaction::RoleSimulateRemovePackages);
    simulateUpdatePackagesCB->setChecked(actions & Transaction::RoleSimulateUpdatePackages);
    upgradeSystemCB->setChecked(actions & Transaction::RoleUpgradeSystem);

    // FILTERS - Setup filters
    Transaction::Filters filters = Daemon::filters();
    installedCB->setChecked(filters & Transaction::FilterInstalled);
    guiCB->setChecked(filters & Transaction::FilterGui);

    developmentCB->setChecked(filters & Transaction::FilterDevel);
    freeCB->setChecked(filters & Transaction::FilterFree);

    visibleCB->setChecked(filters & Transaction::FilterVisible);
    supportedCB->setChecked(filters & Transaction::FilterSupported);

    newestCB->setChecked(filters & Transaction::FilterNewest);
}

#include "BackendDetails.moc"
