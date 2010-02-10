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

#include "KpkBackendDetails.h"
#include <KIcon>

using namespace PackageKit;

KpkBackendDetails::KpkBackendDetails(QWidget *parent)
  : KDialog(parent)
{
    setupUi(mainWidget());
    setButtons(KDialog::Close);
    setWindowIcon(KIcon("help-about"));

    // GENERAL - Setup backend name and author
    nameL->setText(Client::instance()->backendName());
    descriptionL->setText(Client::instance()->backendDescription());
    authorL->setText(Client::instance()->backendAuthor());

    // METHODS - Setup backend supported methods
    Enum::Roles actions = Client::instance()->actions();
    getUpdatesCB->setChecked(actions & Enum::RoleGetUpdates);
    refreshCacheCB->setChecked(actions & Enum::RoleRefreshCache);
    updateSystemCB->setChecked(actions & Enum::RoleUpdateSystem);
    searchNameCB->setChecked(actions & Enum::RoleSearchName);
    searchDetailsCB->setChecked(actions & Enum::RoleSearchDetails);
    searchGroupCB->setChecked(actions & Enum::RoleSearchGroup);
    searchFileCB->setChecked(actions & Enum::RoleSearchFile);
    cancelCB->setChecked(actions & Enum::RoleCancel);
    resolveCB->setChecked(actions & Enum::RoleResolve);

    updatePackageCB->setChecked(actions & Enum::RoleUpdatePackages);
    installPackageCB->setChecked(actions & Enum::RoleInstallPackages);
    removePackageCB->setChecked(actions & Enum::RoleRemovePackages);
    getDependsCB->setChecked(actions & Enum::RoleGetDepends);
    getRequiresCB->setChecked(actions & Enum::RoleGetRequires);
    getUpdateDetailCB->setChecked(actions & Enum::RoleGetUpdateDetail);
    getDescriptionCB->setChecked(actions & Enum::RoleGetDetails);
    getFilesCB->setChecked(actions & Enum::RoleRefreshCache);
    installFileCB->setChecked(actions & Enum::RoleInstallFiles);

    getRepositoryListCB->setChecked(actions & Enum::RoleGetRepoList);
    repositoryEnableCB->setChecked(actions & Enum::RoleRepoEnable);
    repositorySetEnableCB->setChecked(actions & Enum::RoleRepoSetData);
    whatProvidesCB->setChecked(actions & Enum::RoleWhatProvides);
    getPackagesCB->setChecked(actions & Enum::RoleGetPackages);
    simulateInstallFilesCB->setChecked(actions & Enum::RoleSimulateInstallFiles);
    simulateInstallPackagesCB->setChecked(actions & Enum::RoleSimulateInstallPackages);
    simulateRemovePackagesCB->setChecked(actions & Enum::RoleSimulateRemovePackages);
    simulateUpdatePackagesCB->setChecked(actions & Enum::RoleSimulateUpdatePackages);

    // FILTERS - Setup filters
    Enum::Filters filters = Client::instance()->filters();
    installedCB->setChecked(filters & Enum::FilterInstalled);
    guiCB->setChecked(filters & Enum::FilterGui);

    developmentCB->setChecked(filters & Enum::FilterDevelopment);
    freeCB->setChecked(filters & Enum::FilterFree);

    visibleCB->setChecked(filters & Enum::FilterVisible);
    supportedCB->setChecked(filters & Enum::FilterSupported);

    newestCB->setChecked(filters & Enum::FilterNewest);
}

#include "KpkBackendDetails.moc"
