/***************************************************************************
 *   Copyright (C) 2008 by Trever Fischer                                  *
 *   wm161@wm161.net                                                       *
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

#include "KpkIcons.h"
#include <KIconLoader>
#include <KDebug>

bool KpkIcons::init = false;
QHash<QString, KIcon> KpkIcons::cache = QHash<QString, KIcon>();

KIcon KpkIcons::getIcon(const QString &name)
{
    if (!KpkIcons::init) {
        KIconLoader::global()->addAppDir("kpackagekit");
        KpkIcons::init = true;
    }
    if (!KpkIcons::cache.contains(name)) {
        KpkIcons::cache[name] = KIcon(name);
    }
    return KpkIcons::cache[name];
}

KIcon KpkIcons::getIcon(const QString &name, const QString &defaultName)
{
    if (!KpkIcons::init) {
        KIconLoader::global()->addAppDir("kpackagekit");
        KpkIcons::init = true;
    }
    if (!KpkIcons::cache.contains(name)) {
        if (KIconLoader::global()->loadIcon(name,
                                            KIconLoader::NoGroup,
                                            KIconLoader::SizeLarge,
                                            KIconLoader::DefaultState,
                                            QStringList(),
                                            NULL,
                                            true).isNull()) {
            KpkIcons::cache[name] = KIcon(defaultName);
        } else {
            KpkIcons::cache[name] = KIcon(name);
        }
    }
    return KpkIcons::cache[name];
}

KIcon KpkIcons::statusIcon(Enum::Status status)
{
    switch (status) {
    case Enum::LastStatus                 :
    case Enum::UnknownStatus              : return KpkIcons::getIcon("help-browser");
    case Enum::StatusCancel               :
    case Enum::StatusCleanup              : return KpkIcons::getIcon("kpk-clean-up");
    case Enum::StatusCommit               : return KpkIcons::getIcon("package-setup");//TODO needs a better icon
    case Enum::StatusDepResolve           : return KpkIcons::getIcon("package-info");
    case Enum::StatusDownloadChangelog    :
    case Enum::StatusDownloadFilelist     :
    case Enum::StatusDownloadGroup        :
    case Enum::StatusDownloadPackagelist  : return KpkIcons::getIcon("kpk-refresh-cache");
    case Enum::StatusDownload             : return KpkIcons::getIcon("package-download");
    case Enum::StatusDownloadRepository   :
    case Enum::StatusDownloadUpdateinfo   : return KpkIcons::getIcon("kpk-refresh-cache");
    case Enum::StatusFinished             : return KpkIcons::getIcon("kpk-clean-up");
    case Enum::StatusGeneratePackageList  : return KpkIcons::getIcon("kpk-refresh-cache");
    case Enum::StatusWaitingForLock       : return KpkIcons::getIcon("dialog-password");
    case Enum::StatusWaitingForAuth       : return KpkIcons::getIcon("dialog-password");//IMPROVE ME
    case Enum::StatusInfo                 : return KpkIcons::getIcon("package-info");
    case Enum::StatusInstall              : return KpkIcons::getIcon("kpk-package-add");
    case Enum::StatusLoadingCache         : return KpkIcons::getIcon("kpk-refresh-cache");
    case Enum::StatusObsolete             : return KpkIcons::getIcon("kpk-clean-up");
    case Enum::StatusQuery                : return KpkIcons::getIcon("package-search");
    case Enum::StatusRefreshCache         : return KpkIcons::getIcon("kpk-refresh-cache");
    case Enum::StatusRemove               : return KpkIcons::getIcon("package-removed");
    case Enum::StatusRepackaging          : return KpkIcons::getIcon("kpk-clean-up");
    case Enum::StatusRequest              : return KpkIcons::getIcon("package-search");
    case Enum::StatusRollback             : return KpkIcons::getIcon("package-rollback");
    case Enum::StatusRunning              : return KpkIcons::getIcon("package-setup");
    case Enum::StatusScanApplications     : return KpkIcons::getIcon("package-search");
    case Enum::StatusSetup                : return KpkIcons::getIcon("package-setup");
    case Enum::StatusSigCheck             :
    case Enum::StatusTestCommit           : return KpkIcons::getIcon("package-info");//TODO needs a better icon
    case Enum::StatusUpdate               : return KpkIcons::getIcon("package-update");
    case Enum::StatusWait                 : return KpkIcons::getIcon("package-wait");
    case Enum::StatusScanProcessList      : return KpkIcons::getIcon("package-info");
    case Enum::StatusCheckExecutableFiles : return KpkIcons::getIcon("package-info");
    case Enum::StatusCheckLibraries       : return KpkIcons::getIcon("package-info");
    case Enum::StatusCopyFiles            : return KpkIcons::getIcon("package-info");
    }
    kDebug() << "status icon unrecognised: " << status;
    return KpkIcons::getIcon("help-browser");
}

QString KpkIcons::statusAnimation(PackageKit::Enum::Status status)
{
    switch (status) {
    case Enum::UnknownStatus             : return "help-browser";
    case Enum::StatusCancel              :
    case Enum::StatusCleanup             : return "pk-cleaning-up";
    case Enum::StatusCommit              :
    case Enum::StatusDepResolve          : return "pk-testing";
    case Enum::StatusDownloadChangelog   :
    case Enum::StatusDownloadFilelist    :
    case Enum::StatusDownloadGroup       :
    case Enum::StatusDownloadPackagelist : return "pk-refresh-cache";
    case Enum::StatusDownload            : return "pk-downloading";
    case Enum::StatusDownloadRepository  :
    case Enum::StatusDownloadUpdateinfo  : return "pk-refresh-cache";
    case Enum::StatusFinished            : return "pk-cleaning-up";
    case Enum::StatusGeneratePackageList : return "pk-searching";
    case Enum::StatusWaitingForLock      : return "pk-waiting";
    case Enum::StatusInfo                : return "package-working";
    case Enum::StatusInstall             : return "pk-installing";
    case Enum::StatusLoadingCache        : return "pk-refresh-cache";
    case Enum::StatusObsolete            : return "pk-cleaning-up";
    case Enum::StatusQuery               : return "pk-searching";
    case Enum::StatusRefreshCache        : return "pk-refresh-cache";
    case Enum::StatusRemove              : return "package-removed";//TODO do the animation
    case Enum::StatusRepackaging         : return "pk-searching";
    case Enum::StatusRequest             : return "process-working";
    case Enum::StatusRollback            : return "package-removed";
    case Enum::StatusRunning             : return "pk-testing";
    case Enum::StatusScanApplications    : return "pk-searching";
    case Enum::StatusSetup               : return "pk-searching";
    case Enum::StatusSigCheck            : return "package-info";
    case Enum::StatusTestCommit          : return "pk-testing";
    case Enum::StatusUpdate              : return "pk-installing";
    case Enum::StatusWait                : return "pk-waiting";
    default                                     : kDebug() << "status icon unrecognised: " << status;
                                                  return "help-browser";
    }
}

KIcon KpkIcons::actionIcon(Enum::Role role)
{
    switch (role) {
    case Enum::LastRole                    :
    case Enum::UnknownRole                 : return KpkIcons::getIcon("applications-other");
    case Enum::RoleAcceptEula              : return KpkIcons::getIcon("package-info");
    case Enum::RoleCancel                  : return KpkIcons::getIcon("process-stop");
    case Enum::RoleDownloadPackages        : return KpkIcons::getIcon("package-download");
    case Enum::RoleGetCategories           : return KpkIcons::getIcon("package-info");
    case Enum::RoleGetDepends              : return KpkIcons::getIcon("package-info");
    case Enum::RoleGetDetails              : return KpkIcons::getIcon("package-info");
    case Enum::RoleGetDistroUpgrades       : return KpkIcons::getIcon("distro-upgrade");
    case Enum::RoleGetFiles                : return KpkIcons::getIcon("package-search");
    case Enum::RoleGetOldTransactions      : return KpkIcons::getIcon("package-info");
    case Enum::RoleGetPackages             : return KpkIcons::getIcon("package-packages");
    case Enum::RoleGetRepoList             : return KpkIcons::getIcon("package-orign");
    case Enum::RoleGetRequires             : return KpkIcons::getIcon("package-info");
    case Enum::RoleGetUpdateDetail         : return KpkIcons::getIcon("package-info");
    case Enum::RoleGetUpdates              : return KpkIcons::getIcon("package-info");
    case Enum::RoleInstallFiles            : return KpkIcons::getIcon("package-installed");
    case Enum::RoleInstallPackages         : return KpkIcons::getIcon("package-installed");
    case Enum::RoleInstallSignature        : return KpkIcons::getIcon("package-installed");
    case Enum::RoleRefreshCache            : return KpkIcons::getIcon("kpk-refresh-cache");
    case Enum::RoleRemovePackages          : return KpkIcons::getIcon("package-removed");
    case Enum::RoleRepoEnable              : return KpkIcons::getIcon("package-orign");
    case Enum::RoleRepoSetData             : return KpkIcons::getIcon("package-orign");
    case Enum::RoleResolve                 : return KpkIcons::getIcon("package-search");
    case Enum::RoleRollback                : return KpkIcons::getIcon("package-rollback");
    case Enum::RoleSearchDetails           : return KpkIcons::getIcon("package-search");
    case Enum::RoleSearchFile              : return KpkIcons::getIcon("package-search");
    case Enum::RoleSearchGroup             : return KpkIcons::getIcon("package-search");
    case Enum::RoleSearchName              : return KpkIcons::getIcon("package-search");
    case Enum::RoleUpdatePackages          : return KpkIcons::getIcon("package-update");
    case Enum::RoleUpdateSystem            : return KpkIcons::getIcon("distro-upgrade");//TODO
    case Enum::RoleWhatProvides            : return KpkIcons::getIcon("package-search");
    case Enum::RoleSimulateInstallFiles    : return KpkIcons::getIcon("package-installed");
    case Enum::RoleSimulateInstallPackages : return KpkIcons::getIcon("package-installed");
    case Enum::RoleSimulateRemovePackages  : return KpkIcons::getIcon("package-removed");
    case Enum::RoleSimulateUpdatePackages  : return KpkIcons::getIcon("package-update'");
    }
    kDebug() << "action unrecognised: " << role;
    return KpkIcons::getIcon("applications-other");
}

KIcon KpkIcons::groupsIcon(Enum::Group group)
{
    switch (group) {
    case Enum::LastGroup            :
    case Enum::UnknownGroup         : return KpkIcons::getIcon("unknown");
    case Enum::GroupAccessibility   : return KpkIcons::getIcon("preferences-desktop-accessibility");
    case Enum::GroupAccessories     : return KpkIcons::getIcon("applications-accessories");
    case Enum::GroupAdminTools      : return KpkIcons::getIcon("dialog-password");
    case Enum::GroupCommunication   : return KpkIcons::getIcon("network-workgroup");//FIXME
    case Enum::GroupDesktopGnome    : return KpkIcons::getIcon("user-desktop");//FIXME
    case Enum::GroupDesktopKde      : return KpkIcons::getIcon("kde");
    case Enum::GroupDesktopOther    : return KpkIcons::getIcon("user-desktop");
    case Enum::GroupDesktopXfce     : return KpkIcons::getIcon("user-desktop");//FIXME
    case Enum::GroupDocumentation   : return KpkIcons::getIcon("accessories-dictionary");//FIXME
    case Enum::GroupEducation       : return KpkIcons::getIcon("applications-education");
    case Enum::GroupElectronics     : return KpkIcons::getIcon("media-flash");
    case Enum::GroupFonts           : return KpkIcons::getIcon("preferences-desktop-font");
    case Enum::GroupGames           : return KpkIcons::getIcon("applications-games");
    case Enum::GroupGraphics        : return KpkIcons::getIcon("applications-graphics");
    case Enum::GroupInternet        : return KpkIcons::getIcon("applications-internet");
    case Enum::GroupLegacy          : return KpkIcons::getIcon("media-floppy");
    case Enum::GroupLocalization    : return KpkIcons::getIcon("applications-education-language");
    case Enum::GroupMaps            : return KpkIcons::getIcon("Maps");//FIXME
    case Enum::GroupCollections     : return KpkIcons::getIcon("unknown");//FIXME
    case Enum::GroupMultimedia      : return KpkIcons::getIcon("applications-multimedia");
    case Enum::GroupNetwork         : return KpkIcons::getIcon("network-wired");
    case Enum::GroupOffice          : return KpkIcons::getIcon("applications-office");
    case Enum::GroupOther           : return KpkIcons::getIcon("applications-other");
    case Enum::GroupPowerManagement : return KpkIcons::getIcon("battery");
    case Enum::GroupProgramming     : return KpkIcons::getIcon("applications-development");
    case Enum::GroupPublishing      : return KpkIcons::getIcon("accessories-text-editor");
    case Enum::GroupRepos           : return KpkIcons::getIcon("application-x-compressed-tar");
    case Enum::GroupScience         : return KpkIcons::getIcon("applications-science");
    case Enum::GroupSecurity        : return KpkIcons::getIcon("security-high");
    case Enum::GroupServers         : return KpkIcons::getIcon("network-server");
    case Enum::GroupSystem          : return KpkIcons::getIcon("applications-system");
    case Enum::GroupVirtualization  : return KpkIcons::getIcon("cpu");
    case Enum::GroupVendor          : return KpkIcons::getIcon("application-certificate");
    case Enum::GroupNewest          : return KpkIcons::getIcon("dialog-information");
    }
    kDebug() << "group unrecognised: " << group;
    return KpkIcons::getIcon("unknown");
}

KIcon KpkIcons::packageIcon(Enum::Info info)
{
    switch (info) {
    case Enum::InfoBugfix      : return KpkIcons::getIcon("script-error");
    case Enum::InfoImportant   : return KpkIcons::getIcon("security-low");
    case Enum::InfoLow         : return KpkIcons::getIcon("security-high");
    case Enum::InfoEnhancement : return KpkIcons::getIcon("ktip");
    case Enum::InfoSecurity    : return KpkIcons::getIcon("emblem-important");
    case Enum::InfoNormal      : return KpkIcons::getIcon("security-medium");
    case Enum::InfoBlocked     : return KpkIcons::getIcon("dialog-cancel");
    case Enum::InfoAvailable   : return KpkIcons::getIcon("package-download");
    case Enum::InfoInstalled   : return KpkIcons::getIcon("package-installed");
    default                    : return KpkIcons::getIcon("package");
    }
}

KIcon KpkIcons::restartIcon(Enum::Restart type)
{
    switch (type) {
    case Enum::RestartSecuritySystem  :
    case Enum::RestartSystem          : return KpkIcons::getIcon("system-reboot");
    case Enum::RestartSecuritySession :
    case Enum::RestartSession         : return KpkIcons::getIcon("system-log-out");
    case Enum::RestartApplication     : return KpkIcons::getIcon("process-stop");
    case Enum::RestartNone            :
    case Enum::LastRestart            :
    case Enum::UnknownRestart         : KpkIcons::getIcon("");
    }
    return KpkIcons::getIcon("");
}

