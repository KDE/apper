/***************************************************************************
 *   Copyright (C) 2008 by Trever Fischer                                  *
 *   wm161@wm161.net                                                       *
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

#include "KpkIcons.h"
#include <KIconLoader>
#include <KDebug>

bool KpkIcons::init = false;
QHash<QString, KIcon> KpkIcons::cache = QHash<QString, KIcon>();
//KpkIcons::cache = QHash<QString, KIcon>();

KIcon KpkIcons::getIcon(const QString &name)
{
    if (!KpkIcons::init) {
        KIconLoader::global()->addAppDir("kpackagekit");
        KpkIcons::init = true;
    }
    if (!KpkIcons::cache.contains(name))
        KpkIcons::cache[name] = KIcon(name);
    return KpkIcons::cache[name];
}

KIcon KpkIcons::statusIcon(PackageKit::Transaction::Status status)
{
    switch (status) {
    case Transaction::UnknownStatus              : return KpkIcons::getIcon("help-browser");
    case Transaction::StatusCancel               : 
    case Transaction::StatusCleanup              : return KpkIcons::getIcon("kpk-clean-up");
    case Transaction::StatusCommit               : return KpkIcons::getIcon("package-setup");//TODO needs a better icon
    case Transaction::StatusDepResolve           : return KpkIcons::getIcon("package-info");
    case Transaction::StatusDownloadChangelog    :
    case Transaction::StatusDownloadFilelist     :
    case Transaction::StatusDownloadGroup        : 
    case Transaction::StatusDownloadPackagelist  : return KpkIcons::getIcon("kpk-refresh-cache");
    case Transaction::StatusDownload             : return KpkIcons::getIcon("package-download");
    case Transaction::StatusDownloadRepository   :
    case Transaction::StatusDownloadUpdateinfo   : return KpkIcons::getIcon("kpk-refresh-cache");
    case Transaction::StatusFinished             : return KpkIcons::getIcon("kpk-clean-up");
    case Transaction::StatusGeneratePackageList  : return KpkIcons::getIcon("kpk-refresh-cache");
    case Transaction::StatusWaitingForLock       : return KpkIcons::getIcon("dialog-password");
    case Transaction::StatusWaitingForAuth       : return KpkIcons::getIcon("dialog-password");//IMPROVE ME
    case Transaction::StatusInfo                 : return KpkIcons::getIcon("package-info");
    case Transaction::StatusInstall              : return KpkIcons::getIcon("kpk-package-add");
    case Transaction::StatusLoadingCache         : return KpkIcons::getIcon("kpk-refresh-cache");
    case Transaction::StatusObsolete             : return KpkIcons::getIcon("kpk-clean-up");
    case Transaction::StatusQuery                : return KpkIcons::getIcon("package-search");
    case Transaction::StatusRefreshCache         : return KpkIcons::getIcon("kpk-refresh-cache");
    case Transaction::StatusRemove               : return KpkIcons::getIcon("package-removed");
    case Transaction::StatusRepackaging          : return KpkIcons::getIcon("kpk-clean-up");
    case Transaction::StatusRequest              : return KpkIcons::getIcon("package-search");
    case Transaction::StatusRollback             : return KpkIcons::getIcon("package-rollback");
    case Transaction::StatusRunning              : return KpkIcons::getIcon("package-setup");
    case Transaction::StatusScanApplications     : return KpkIcons::getIcon("package-search");
    case Transaction::StatusSetup                : return KpkIcons::getIcon("package-setup");
    case Transaction::StatusSigCheck             :
    case Transaction::StatusTestCommit           : return KpkIcons::getIcon("package-info");//TODO needs a better icon
    case Transaction::StatusUpdate               : return KpkIcons::getIcon("package-update");
    case Transaction::StatusWait                 : return KpkIcons::getIcon("package-wait");
    case Transaction::StatusScanProcessList      : return KpkIcons::getIcon("package-info");
    case Transaction::StatusCheckExecutableFiles : return KpkIcons::getIcon("package-info");
    case Transaction::StatusCheckLibraries       : return KpkIcons::getIcon("package-info");
    }
    kDebug() << "status icon unrecognised: " << status;
    return KpkIcons::getIcon("help-browser");
}

QString KpkIcons::statusAnimation(PackageKit::Transaction::Status status)
{
    switch (status) {
    case Transaction::UnknownStatus             : return "help-browser";
    case Transaction::StatusCancel              :
    case Transaction::StatusCleanup             : return "pk-cleaning-up";
    case Transaction::StatusCommit              : return "package-setup";
    case Transaction::StatusDepResolve          : return "pk-testing";
    case Transaction::StatusDownloadChangelog   :
    case Transaction::StatusDownloadFilelist    :
    case Transaction::StatusDownloadGroup       :
    case Transaction::StatusDownloadPackagelist : return "pk-refresh-cache";
    case Transaction::StatusDownload            : return "pk-downloading";
    case Transaction::StatusDownloadRepository  :
    case Transaction::StatusDownloadUpdateinfo  : return "pk-refresh-cache";
    case Transaction::StatusFinished            : return "pk-cleaning-up";
    case Transaction::StatusGeneratePackageList : return "pk-searching";
    case Transaction::StatusWaitingForLock      : return "pk-waiting";
    case Transaction::StatusInfo                : return "package-working";
    case Transaction::StatusInstall             : return "pk-installing";
    case Transaction::StatusLoadingCache        : return "pk-refresh-cache";
    case Transaction::StatusObsolete            : return "pk-cleaning-up";
    case Transaction::StatusQuery               : return "pk-searching";
    case Transaction::StatusRefreshCache        : return "pk-refresh-cache";
    case Transaction::StatusRemove              : return "pk-removing";
    case Transaction::StatusRepackaging         : return "package-info";
    case Transaction::StatusRequest             : return "process-working";
    case Transaction::StatusRollback            : return "pk-removing";
    case Transaction::StatusRunning             : return "pk-setup";
    case Transaction::StatusScanApplications    : return "pk-searching";
    case Transaction::StatusSetup               : return "package-info";
    case Transaction::StatusSigCheck            : return "package-info";
    case Transaction::StatusTestCommit          : return "pk-testing";
    case Transaction::StatusUpdate              : return "pk-installing";
    case Transaction::StatusWait                : return "pk-waiting";
    default                                     : kDebug() << "status icon unrecognised: " << status;
                                                  return "help-browser";
    }
}

KIcon KpkIcons::actionIcon(Client::Action action)
{
    switch (action) {
    case Client::UnknownAction                 : return KpkIcons::getIcon("applications-other");
    case Client::ActionAcceptEula              : return KpkIcons::getIcon("package-info");
    case Client::ActionCancel                  : return KpkIcons::getIcon("process-stop");
    case Client::ActionDownloadPackages        : return KpkIcons::getIcon("package-download");
    case Client::ActionGetCategories           : return KpkIcons::getIcon("package-info");
    case Client::ActionGetDepends              : return KpkIcons::getIcon("package-info");
    case Client::ActionGetDetails              : return KpkIcons::getIcon("package-info");
    case Client::ActionGetDistroUpgrades       : return KpkIcons::getIcon("distro-upgrade");
    case Client::ActionGetFiles                : return KpkIcons::getIcon("package-search");
    case Client::ActionGetOldTransactions      : return KpkIcons::getIcon("package-info");
    case Client::ActionGetPackages             : return KpkIcons::getIcon("package-packages");
    case Client::ActionGetRepoList             : return KpkIcons::getIcon("package-orign");
    case Client::ActionGetRequires             : return KpkIcons::getIcon("package-info");
    case Client::ActionGetUpdateDetail         : return KpkIcons::getIcon("package-info");
    case Client::ActionGetUpdates              : return KpkIcons::getIcon("package-info");
    case Client::ActionInstallFiles            : return KpkIcons::getIcon("package-installed");
    case Client::ActionInstallPackages         : return KpkIcons::getIcon("package-installed");
    case Client::ActionInstallSignature        : return KpkIcons::getIcon("package-installed");
    case Client::ActionRefreshCache            : return KpkIcons::getIcon("pk-refresh-cache");
    case Client::ActionRemovePackages          : return KpkIcons::getIcon("package-removed");
    case Client::ActionRepoEnable              : return KpkIcons::getIcon("package-orign");
    case Client::ActionRepoSetData             : return KpkIcons::getIcon("package-orign");
    case Client::ActionResolve                 : return KpkIcons::getIcon("package-search");
    case Client::ActionRollback                : return KpkIcons::getIcon("package-rollback");
    case Client::ActionSearchDetails           : return KpkIcons::getIcon("package-search");
    case Client::ActionSearchFile              : return KpkIcons::getIcon("package-search");
    case Client::ActionSearchGroup             : return KpkIcons::getIcon("package-search");
    case Client::ActionSearchName              : return KpkIcons::getIcon("package-search");
    case Client::ActionUpdatePackages          : return KpkIcons::getIcon("package-update");
    case Client::ActionUpdateSystem            : return KpkIcons::getIcon("distro-upgrade");//TODO
    case Client::ActionWhatProvides            : return KpkIcons::getIcon("package-search");
    case Client::ActionSimulateInstallFiles    : return KpkIcons::getIcon("package-installed");
    case Client::ActionSimulateInstallPackages : return KpkIcons::getIcon("package-installed");
    case Client::ActionSimulateRemovePackages  : return KpkIcons::getIcon("package-removed");
    case Client::ActionSimulateUpdatePackages  : return KpkIcons::getIcon("package-update'");
    }
    kDebug() << "action unrecognised: " << action;
    return KpkIcons::getIcon("applications-other");
}

KIcon KpkIcons::groupsIcon(Client::Group group)
{
    switch (group) {
    case Client::UnknownGroup         : return KpkIcons::getIcon("unknown");
    case Client::GroupAccessibility   : return KpkIcons::getIcon("preferences-desktop-accessibility");
    case Client::GroupAccessories     : return KpkIcons::getIcon("applications-accessories");
    case Client::GroupAdminTools      : return KpkIcons::getIcon("dialog-password");
    case Client::GroupCommunication   : return KpkIcons::getIcon("network-workgroup");//FIXME
    case Client::GroupDesktopGnome    : return KpkIcons::getIcon("user-desktop");//FIXME
    case Client::GroupDesktopKde      : return KpkIcons::getIcon("kde");
    case Client::GroupDesktopOther    : return KpkIcons::getIcon("user-desktop");
    case Client::GroupDesktopXfce     : return KpkIcons::getIcon("user-desktop");//FIXME
    case Client::GroupDocumentation   : return KpkIcons::getIcon("accessories-dictionary");//FIXME
    case Client::GroupEducation       : return KpkIcons::getIcon("applications-education");
    case Client::GroupElectronics     : return KpkIcons::getIcon("media-flash");
    case Client::GroupFonts           : return KpkIcons::getIcon("preferences-desktop-font");
    case Client::GroupGames           : return KpkIcons::getIcon("applications-games");
    case Client::GroupGraphics        : return KpkIcons::getIcon("applications-graphics");
    case Client::GroupInternet        : return KpkIcons::getIcon("applications-internet");
    case Client::GroupLegacy          : return KpkIcons::getIcon("media-floppy");
    case Client::GroupLocalization    : return KpkIcons::getIcon("applications-education-language");
    case Client::GroupMaps            : return KpkIcons::getIcon("Maps");//FIXME
    case Client::GroupCollections     : return KpkIcons::getIcon("unknown");//FIXME
    case Client::GroupMultimedia      : return KpkIcons::getIcon("applications-multimedia");
    case Client::GroupNetwork         : return KpkIcons::getIcon("network-wired");
    case Client::GroupOffice          : return KpkIcons::getIcon("applications-office");
    case Client::GroupOther           : return KpkIcons::getIcon("applications-other");
    case Client::GroupPowerManagement : return KpkIcons::getIcon("battery");
    case Client::GroupProgramming     : return KpkIcons::getIcon("applications-development");
    case Client::GroupPublishing      : return KpkIcons::getIcon("accessories-text-editor");
    case Client::GroupRepos           : return KpkIcons::getIcon("application-x-compressed-tar");
    case Client::GroupScience         : return KpkIcons::getIcon("applications-science");
    case Client::GroupSecurity        : return KpkIcons::getIcon("security-high");
    case Client::GroupServers         : return KpkIcons::getIcon("network-server");
    case Client::GroupSystem          : return KpkIcons::getIcon("applications-system");
    case Client::GroupVirtualization  : return KpkIcons::getIcon("cpu");
    case Client::GroupVendor          : return KpkIcons::getIcon("application-certificate");
    case Client::GroupNewest          : return KpkIcons::getIcon("dialog-information");
    }
    kDebug() << "group unrecognised: " << group;
    return KpkIcons::getIcon("unknown");
}

KIcon KpkIcons::packageIcon(Package::State state)
{
    switch (state) {
    case Package::StateBugfix      : return KpkIcons::getIcon("script-error");
    case Package::StateImportant   : return KpkIcons::getIcon("security-low");
    case Package::StateLow         : return KpkIcons::getIcon("security-high");
    case Package::StateEnhancement : return KpkIcons::getIcon("ktip");
    case Package::StateSecurity    : return KpkIcons::getIcon("emblem-important");
    case Package::StateNormal      : return KpkIcons::getIcon("security-medium");
    case Package::StateBlocked     : return KpkIcons::getIcon("dialog-cancel");
    case Package::StateAvailable   : return KpkIcons::getIcon("package-download");
    case Package::StateInstalled   : return KpkIcons::getIcon("package-installed");
    default                        : return KpkIcons::getIcon("package");
    }
}

KIcon KpkIcons::restartIcon(Client::RestartType type)
{
    switch (type) {
    case Client::RestartSecuritySystem  :
    case Client::RestartSystem          : return KpkIcons::getIcon("system-reboot");
    case Client::RestartSecuritySession :
    case Client::RestartSession         : return KpkIcons::getIcon("system-log-out");
    case Client::RestartApplication     : return KpkIcons::getIcon("process-stop");
    case Client::RestartNone            :
    case Client::UnknownRestartType     : KpkIcons::getIcon("");
    }
    return KpkIcons::getIcon("");
}

