/***************************************************************************
 *   Copyright (C) 2009 by Daniel Nicoletti                                *
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
    setWindowIcon(KIcon::KIcon("help-about"));

    // GENERAL - Setup backend name and author
    Client::BackendDetail bkDetail = Client::instance()->getBackendDetail();
    nameL->setText(bkDetail.name);
    authorL->setText(bkDetail.author);

    // METHODS - Setup backend supported methods
    Client::Actions actions = Client::instance()->getActions();
    getUpdatesCB->setChecked(actions & Client::ActionGetUpdates);
    refreshCacheCB->setChecked(actions & Client::ActionRefreshCache);
    updateSystemCB->setChecked(actions & Client::ActionUpdateSystem);
    searchNameCB->setChecked(actions & Client::ActionSearchName);
    searchDetailsCB->setChecked(actions & Client::ActionSearchDetails);
    searchGroupCB->setChecked(actions & Client::ActionSearchGroup);
    searchFileCB->setChecked(actions & Client::ActionSearchFile);
    cancelCB->setChecked(actions & Client::ActionCancel);
    resolveCB->setChecked(actions & Client::ActionResolve);

    updatePackageCB->setChecked(actions & Client::ActionUpdatePackages);
    installPackageCB->setChecked(actions & Client::ActionInstallPackages);
    removePackageCB->setChecked(actions & Client::ActionRemovePackages);
    getDependsCB->setChecked(actions & Client::ActionGetDepends);
    getRequiresCB->setChecked(actions & Client::ActionGetRequires);
    getUpdateDetailCB->setChecked(actions & Client::ActionGetUpdateDetail);
    getDescriptionCB->setChecked(actions & Client::ActionGetDetails);
    getFilesCB->setChecked(actions & Client::ActionRefreshCache);
    installFileCB->setChecked(actions & Client::ActionInstallFiles);

    getRepositoryListCB->setChecked(actions & Client::ActionGetRepoList);
    repositoryEnableCB->setChecked(actions & Client::ActionRepoEnable);
    repositorySetEnableCB->setChecked(actions & Client::ActionRepoSetData);
    whatProvidesCB->setChecked(actions & Client::ActionWhatProvides);
    getPackagesCB->setChecked(actions & Client::ActionGetPackages);

    // FILTERS - Setup filters
    Client::Filters filters = Client::instance()->getFilters();
    installedCB->setChecked(filters & Client::FilterInstalled);
    guiCB->setChecked(filters & Client::FilterGui);

    developmentCB->setChecked(filters & Client::FilterDevelopment);
    freeCB->setChecked(filters & Client::FilterFree);

    visibleCB->setChecked(filters & Client::FilterVisible);
    supportedCB->setChecked(filters & Client::FilterSupported);

    newestCB->setChecked(filters & Client::FilterNewest);
}

#include "KpkBackendDetails.moc"
