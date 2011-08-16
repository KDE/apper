/***************************************************************************
 *   Copyright (C) 2008 by Trever Fischer                                  *
 *   wm161@wm161.net                                                       *
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

#include <config.h>

#include "PkIcons.h"

#include <KIconLoader>
#include <KStandardDirs>

#include <KDebug>

bool PkIcons::init = false;
QHash<QString, KIcon> PkIcons::cache = QHash<QString, KIcon>();

void PkIcons::configure()
{
    KGlobal::dirs()->addResourceDir("xdgdata-pixmap", "/usr/share/app-install/icons/");
    KIconLoader::global()->reconfigure("apper", 0);
    PkIcons::init = true;
}

KIcon PkIcons::getIcon(const QString &name)
{
//     kDebug() << 1 << name;
    if (!PkIcons::init) {
        PkIcons::configure();
    }

    return KIcon(name);
}

KIcon PkIcons::getIcon(const QString &name, const QString &defaultName)
{
//     kDebug() << 2 << name << defaultName;
    if (!PkIcons::init) {
        PkIcons::configure();
    }

    if (name.isEmpty()) {
        return KIcon();
    }

    bool isNull;
    isNull = KIconLoader::global()->iconPath(name, KIconLoader::NoGroup, true).isEmpty();

    if (isNull && !defaultName.isNull()) {
        return KIcon(defaultName);
    } else if (isNull) {
        return KIcon();
    }
    return KIcon(name);
}

QString PkIcons::statusIconName(Transaction::Status status)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }
    switch (status) {
    case Transaction::UnknownStatus              : return "help-browser";
    case Transaction::StatusCancel               :
    case Transaction::StatusCleanup              : return "package-clean-up";
    case Transaction::StatusCommit               : return "package-working";//TODO needs a better icon
    case Transaction::StatusDepResolve           : return "package-info";
    case Transaction::StatusDownloadChangelog    :
    case Transaction::StatusDownloadFilelist     :
    case Transaction::StatusDownloadGroup        :
    case Transaction::StatusDownloadPackagelist  : return "refresh-cache";
    case Transaction::StatusDownload             : return "package-download";
    case Transaction::StatusDownloadRepository   :
    case Transaction::StatusDownloadUpdateinfo   : return "refresh-cache";
    case Transaction::StatusFinished             : return "package-clean-up";
    case Transaction::StatusGeneratePackageList  : return "refresh-cache";
    case Transaction::StatusWaitingForLock       : return "dialog-password";
    case Transaction::StatusWaitingForAuth       : return "dialog-password";//IMPROVE ME
    case Transaction::StatusInfo                 : return "package-info";
    case Transaction::StatusInstall              : return "kpk-package-add";
    case Transaction::StatusLoadingCache         : return "refresh-cache";
    case Transaction::StatusObsolete             : return "package-clean-up";
    case Transaction::StatusQuery                : return "search-package";
    case Transaction::StatusRefreshCache         : return "refresh-cache";
    case Transaction::StatusRemove               : return "package-removed";
    case Transaction::StatusRepackaging          : return "package-clean-up";
    case Transaction::StatusRequest              : return "search-package";
    case Transaction::StatusRollback             : return "package-rollback";
    case Transaction::StatusRunning              : return "package-working";
    case Transaction::StatusScanApplications     : return "search-package";
    case Transaction::StatusSetup                : return "package-working";
    case Transaction::StatusSigCheck             :
    case Transaction::StatusTestCommit           : return "package-info";//TODO needs a better icon
    case Transaction::StatusUpdate               : return "package-update";
    case Transaction::StatusWait                 : return "package-wait";
    case Transaction::StatusScanProcessList      : return "package-info";
    case Transaction::StatusCheckExecutableFiles : return "package-info";
    case Transaction::StatusCheckLibraries       : return "package-info";
    case Transaction::StatusCopyFiles            : return "package-info";
    }
    kDebug() << "status icon unrecognised: " << status;
    return "help-browser";
}

KIcon PkIcons::statusIcon(Transaction::Status status)
{
    return KIcon(PkIcons::statusIconName(status));
}

QString PkIcons::statusAnimation(Transaction::Status status)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }
    switch (status) {
    case Transaction::UnknownStatus             : return "help-browser";
    case Transaction::StatusCancel              :
    case Transaction::StatusCleanup             : return "pk-cleaning-up";
    case Transaction::StatusCommit              :
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
    case Transaction::StatusRemove              : return "package-removed";//TODO do the animation
    case Transaction::StatusRepackaging         : return "pk-searching";
    case Transaction::StatusRequest             : return "process-working";
    case Transaction::StatusRollback            : return "package-removed";
    case Transaction::StatusRunning             : return "pk-testing";
    case Transaction::StatusScanApplications    : return "pk-searching";
    case Transaction::StatusSetup               : return "pk-searching";
    case Transaction::StatusSigCheck            : return "package-info";
    case Transaction::StatusTestCommit          : return "pk-testing";
    case Transaction::StatusUpdate              : return "pk-installing";
    case Transaction::StatusWait                : return "pk-waiting";
    case Transaction::StatusWaitingForAuth      : return "dialog-password";
    case Transaction::StatusScanProcessList     : return "utilities-system-monitor";
    default                              : kDebug() << "status icon unrecognised: " << status;
        return "help-browser";
    }
}

QString PkIcons::actionIconName(Transaction::Role role)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }
    switch (role) {
    case Transaction::UnknownRole                 : return "applications-other";
    case Transaction::RoleAcceptEula              : return "package-info";
    case Transaction::RoleCancel                  : return "process-stop";
    case Transaction::RoleDownloadPackages        : return "package-download";
    case Transaction::RoleGetCategories           : return "package-info";
    case Transaction::RoleGetDepends              : return "package-info";
    case Transaction::RoleGetDetails              : return "package-info";
    case Transaction::RoleGetDistroUpgrades       : return "distro-upgrade";
    case Transaction::RoleGetFiles                : return "search-package";
    case Transaction::RoleGetOldTransactions      : return "package-info";
    case Transaction::RoleGetPackages             : return "package-packages";
    case Transaction::RoleGetRepoList             : return "package-orign";
    case Transaction::RoleGetRequires             : return "package-info";
    case Transaction::RoleGetUpdateDetail         : return "package-info";
    case Transaction::RoleGetUpdates              : return "package-info";
    case Transaction::RoleInstallFiles            : return "package-installed";
    case Transaction::RoleInstallPackages         : return "package-installed";
    case Transaction::RoleInstallSignature        : return "package-installed";
    case Transaction::RoleRefreshCache            : return "refresh-cache";
    case Transaction::RoleRemovePackages          : return "package-removed";
    case Transaction::RoleRepoEnable              : return "package-orign";
    case Transaction::RoleRepoSetData             : return "package-orign";
    case Transaction::RoleResolve                 : return "search-package";
    case Transaction::RoleRollback                : return "package-rollback";
    case Transaction::RoleSearchDetails           : return "search-package";
    case Transaction::RoleSearchFile              : return "search-package";
    case Transaction::RoleSearchGroup             : return "search-package";
    case Transaction::RoleSearchName              : return "search-package";
    case Transaction::RoleUpdatePackages          : return "package-update";
    case Transaction::RoleUpdateSystem            : return "distro-upgrade";//TODO
    case Transaction::RoleUpgradeSystem           : return "distro-upgrade";//TODO
    case Transaction::RoleWhatProvides            : return "search-package";
    case Transaction::RoleSimulateInstallFiles    : return "package-installed";
    case Transaction::RoleSimulateInstallPackages : return "package-installed";
    case Transaction::RoleSimulateRemovePackages  : return "package-removed";
    case Transaction::RoleSimulateUpdatePackages  : return "package-update'";
    }
    kDebug() << "action unrecognised: " << role;
    return "applications-other";
}

KIcon PkIcons::actionIcon(Transaction::Role role)
{
    return KIcon(actionIconName(role));
}

KIcon PkIcons::groupsIcon(Package::Group group)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }
    switch (group) {
    case Package::UnknownGroup         : return KIcon("unknown");
    case Package::GroupAccessibility   : return KIcon("preferences-desktop-accessibility");
    case Package::GroupAccessories     : return KIcon("applications-accessories");
    case Package::GroupAdminTools      : return KIcon("dialog-password");
    case Package::GroupCommunication   : return KIcon("network-workgroup");//FIXME
    case Package::GroupDesktopGnome    : return KIcon("kpk-desktop-gnome");
    case Package::GroupDesktopKde      : return KIcon("kde");
    case Package::GroupDesktopOther    : return KIcon("user-desktop");
    case Package::GroupDesktopXfce     : return KIcon("kpk-desktop-xfce");
    case Package::GroupDocumentation   : return KIcon("accessories-dictionary");//FIXME
    case Package::GroupEducation       : return KIcon("applications-education");
    case Package::GroupElectronics     : return KIcon("media-flash");
    case Package::GroupFonts           : return KIcon("preferences-desktop-font");
    case Package::GroupGames           : return KIcon("applications-games");
    case Package::GroupGraphics        : return KIcon("applications-graphics");
    case Package::GroupInternet        : return KIcon("applications-internet");
    case Package::GroupLegacy          : return KIcon("media-floppy");
    case Package::GroupLocalization    : return KIcon("applications-education-language");
    case Package::GroupMaps            : return KIcon("Maps");//FIXME
    case Package::GroupCollections     : return KIcon("package-orign");
    case Package::GroupMultimedia      : return KIcon("applications-multimedia");
    case Package::GroupNetwork         : return KIcon("network-wired");
    case Package::GroupOffice          : return KIcon("applications-office");
    case Package::GroupOther           : return KIcon("applications-other");
    case Package::GroupPowerManagement : return KIcon("battery");
    case Package::GroupProgramming     : return KIcon("applications-development");
    case Package::GroupPublishing      : return KIcon("accessories-text-editor");
    case Package::GroupRepos           : return KIcon("application-x-compressed-tar");
    case Package::GroupScience         : return KIcon("applications-science");
    case Package::GroupSecurity        : return KIcon("security-high");
    case Package::GroupServers         : return KIcon("network-server");
    case Package::GroupSystem          : return KIcon("applications-system");
    case Package::GroupVirtualization  : return KIcon("cpu");
    case Package::GroupVendor          : return KIcon("application-certificate");
    case Package::GroupNewest          : return KIcon("dialog-information");
    }
    kDebug() << "group unrecognised: " << group;
    return KIcon("unknown");
}

KIcon PkIcons::packageIcon(Package::Info info)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }
    switch (info) {
    case Package::InfoBugfix      : return KIcon("script-error");
    case Package::InfoEnhancement : return KIcon("ktip");
    case Package::InfoImportant   : return KIcon("security-low");
    case Package::InfoLow         : return KIcon("security-high");
    case Package::InfoSecurity    : return KIcon("security-medium");
    case Package::InfoNormal      : return KIcon("emblem-new");
    case Package::InfoBlocked     : return KIcon("dialog-cancel");
    case Package::InfoAvailable   : return KIcon("package-download");
    case Package::InfoInstalled   : return KIcon("package-installed");
    default                    : return KIcon("package");
    }
}

QString PkIcons::restartIconName(Package::Restart type)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }
    // These names MUST be standard icons, otherwise KStatusNotifierItem
    // will not be able to load them
    switch (type) {
    case Package::RestartSecuritySystem  :
    case Package::RestartSystem          : return "system-reboot";
    case Package::RestartSecuritySession :
    case Package::RestartSession         : return "system-log-out";
    case Package::RestartApplication     : return "process-stop";
    case Package::RestartNone            :
    case Package::UnknownRestart         : break;
    }
    return "";
}

KIcon PkIcons::restartIcon(Package::Restart type)
{
    return PkIcons::getIcon(restartIconName(type));
}

QIcon PkIcons::getPreloadedIcon(const QString &name)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }

    kDebug() << KIconLoader::global()->iconPath(name, KIconLoader::NoGroup, true);
    QIcon icon;
    icon.addPixmap(KIcon(name).pixmap(48, 48));
    return icon;
}

