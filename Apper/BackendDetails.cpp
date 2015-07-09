/***************************************************************************
 *   Copyright (C) 2009-2013 by Daniel Nicoletti                           *
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
#include "ui_BackendDetails.h"

#include <QStringBuilder>
#include <KLocalizedString>

#include <Daemon>

using namespace PackageKit;

BackendDetails::BackendDetails(QWidget *parent) :
    KDialog(parent),
    ui(new Ui::BackendDetails)
{
    setWindowTitle(i18n("Backend Details"));

    ui->setupUi(mainWidget());
    setButtons(KDialog::Close);
    setWindowIcon(QIcon::fromTheme("help-about"));

    // update information about PackageKit backend
    connect(Daemon::global(), SIGNAL(changed()), this, SLOT(daemonChanged()));

    if (Daemon::global()->isRunning()) {
        daemonChanged();
    }
}

BackendDetails::~BackendDetails()
{
    delete ui;
}

void BackendDetails::daemonChanged()
{
    // PackageKit
    QString versionMajor = QString::number(Daemon::global()->versionMajor());
    QString versionMinor = QString::number(Daemon::global()->versionMinor());
    QString versionMicro = QString::number(Daemon::global()->versionMicro());
    ui->pkVersionL->setText(versionMajor % QLatin1Char('.') % versionMinor % QLatin1Char('.') % versionMicro);

    // GENERAL - Setup backend name and author
    ui->nameL->setText(Daemon::global()->backendName());
    ui->descriptionL->setText(Daemon::global()->backendDescription());
    ui->authorL->setText(Daemon::global()->backendAuthor());
    ui->distroL->setText(Daemon::global()->distroID());

    // METHODS - Setup backend supported methods
    Transaction::Roles actions = Daemon::global()->roles();// TODO this is async now
    ui->getUpdatesCB->setChecked(actions & Transaction::RoleGetUpdates);
    ui->getDistroUpgradesCB->setChecked(actions & Transaction::RoleGetDistroUpgrades);
    ui->refreshCacheCB->setChecked(actions & Transaction::RoleRefreshCache);
    ui->searchNameCB->setChecked(actions & Transaction::RoleSearchName);
    ui->searchDetailsCB->setChecked(actions & Transaction::RoleSearchDetails);
    ui->searchGroupCB->setChecked(actions & Transaction::RoleSearchGroup);
    ui->searchFileCB->setChecked(actions & Transaction::RoleSearchFile);
    ui->cancelCB->setChecked(actions & Transaction::RoleCancel);
    ui->resolveCB->setChecked(actions & Transaction::RoleResolve);

    ui->updatePackageCB->setChecked(actions & Transaction::RoleUpdatePackages);
    ui->installPackageCB->setChecked(actions & Transaction::RoleInstallPackages);
    ui->removePackageCB->setChecked(actions & Transaction::RoleRemovePackages);
    ui->getDependsCB->setChecked(actions & Transaction::RoleDependsOn);
    ui->getRequiresCB->setChecked(actions & Transaction::RoleRequiredBy);
    ui->getUpdateDetailCB->setChecked(actions & Transaction::RoleGetUpdateDetail);
    ui->getDescriptionCB->setChecked(actions & Transaction::RoleGetDetails);
    ui->getFilesCB->setChecked(actions & Transaction::RoleRefreshCache);
    ui->installFileCB->setChecked(actions & Transaction::RoleInstallFiles);

    ui->getRepositoryListCB->setChecked(actions & Transaction::RoleGetRepoList);
    ui->repositoryEnableCB->setChecked(actions & Transaction::RoleRepoEnable);
    ui->repositorySetEnableCB->setChecked(actions & Transaction::RoleRepoSetData);
    ui->whatProvidesCB->setChecked(actions & Transaction::RoleWhatProvides);
    ui->getPackagesCB->setChecked(actions & Transaction::RoleGetPackages);
    ui->repairSystemCB->setChecked(actions & Transaction::RoleRepairSystem);

    // FILTERS - Setup filters
    Transaction::Filters filters = Daemon::global()->filters();
    ui->installedCB->setChecked(filters & Transaction::FilterInstalled);
    ui->guiCB->setChecked(filters & Transaction::FilterGui);

    ui->developmentCB->setChecked(filters & Transaction::FilterDevel);
    ui->freeCB->setChecked(filters & Transaction::FilterFree);

    ui->visibleCB->setChecked(filters & Transaction::FilterVisible);
    ui->supportedCB->setChecked(filters & Transaction::FilterSupported);

    ui->newestCB->setChecked(filters & Transaction::FilterNewest);
    ui->archCB->setChecked(filters & Transaction::FilterNotArch);
}

#include "BackendDetails.moc"
