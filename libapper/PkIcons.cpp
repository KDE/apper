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
#include <KGlobal>

using namespace PackageKit;

bool PkIcons::init = false;
QHash<QString, QIcon> PkIcons::cache = QHash<QString, QIcon>();

void PkIcons::configure()
{
    KGlobal::dirs()->addResourceDir("xdgdata-pixmap", "/usr/share/app-info/icons/", "/usr/share/app-install/icons/");
    KIconLoader::global()->reconfigure("apper");
    PkIcons::init = true;
}

QIcon PkIcons::getIcon(const QString &name)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }

    return QIcon::fromTheme(name);
}

QIcon PkIcons::getIcon(const QString &name, const QString &defaultName)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }

    if (name.isEmpty()) {
        return QIcon();
    }

    bool isNull;
    isNull = KIconLoader::global()->iconPath(name, KIconLoader::NoGroup, true).isEmpty();

    if (isNull && !defaultName.isNull()) {
        return QIcon::fromTheme(defaultName);
    } else if (isNull) {
        return QIcon();
    }
    return QIcon::fromTheme(name);
}

QString PkIcons::statusIconName(Transaction::Status status)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }
    switch (status) {
    case Transaction::StatusUnknown              : return "help-browser";
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

QIcon PkIcons::statusIcon(Transaction::Status status)
{
    return QIcon::fromTheme(PkIcons::statusIconName(status));
}

QString PkIcons::statusAnimation(Transaction::Status status)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }
    switch (status) {
    case Transaction::StatusUnknown             : return "help-browser";
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
    case Transaction::RoleUnknown                 : return "applications-other";
    case Transaction::RoleAcceptEula              : return "package-info";
    case Transaction::RoleCancel                  : return "process-stop";
    case Transaction::RoleDownloadPackages        : return "package-download";
    case Transaction::RoleGetCategories           : return "package-info";
    case Transaction::RoleDependsOn               : return "package-info";
    case Transaction::RoleGetDetails              : return "package-info";
    case Transaction::RoleGetDetailsLocal         : return "package-info";
    case Transaction::RoleGetDistroUpgrades       : return "distro-upgrade";
    case Transaction::RoleGetFiles                : return "search-package";
    case Transaction::RoleGetFilesLocal           : return "search-package";
    case Transaction::RoleGetOldTransactions      : return "package-info";
    case Transaction::RoleGetPackages             : return "package-packages";
    case Transaction::RoleGetRepoList             : return "package-orign";
    case Transaction::RoleRequiredBy              : return "package-info";
    case Transaction::RoleGetUpdateDetail         : return "package-info";
    case Transaction::RoleGetUpdates              : return "package-info";
    case Transaction::RoleInstallFiles            : return "package-installed";
    case Transaction::RoleInstallPackages         : return "package-installed";
    case Transaction::RoleInstallSignature        : return "package-installed";
    case Transaction::RoleRefreshCache            : return "refresh-cache";
    case Transaction::RoleRemovePackages          : return "package-removed";
    case Transaction::RoleRepoEnable              : return "package-orign";
    case Transaction::RoleRepoSetData             : return "package-orign";
    case Transaction::RoleRepoRemove              : return "package-orign";
    case Transaction::RoleResolve                 : return "search-package";
    case Transaction::RoleSearchDetails           : return "search-package";
    case Transaction::RoleSearchFile              : return "search-package";
    case Transaction::RoleSearchGroup             : return "search-package";
    case Transaction::RoleSearchName              : return "search-package";
    case Transaction::RoleUpdatePackages          : return "package-update";
    case Transaction::RoleWhatProvides            : return "search-package";
    case Transaction::RoleRepairSystem            : return "package-rollback";
    }
    kDebug() << "action unrecognised: " << role;
    return "applications-other";
}

QIcon PkIcons::actionIcon(Transaction::Role role)
{
    return QIcon::fromTheme(actionIconName(role));
}

QIcon PkIcons::groupsIcon(Transaction::Group group)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }
    switch (group) {
    case Transaction::GroupUnknown         : return QIcon::fromTheme("unknown");
    case Transaction::GroupAccessibility   : return QIcon::fromTheme("preferences-desktop-accessibility");
    case Transaction::GroupAccessories     : return QIcon::fromTheme("applications-accessories");
    case Transaction::GroupAdminTools      : return QIcon::fromTheme("dialog-password");
    case Transaction::GroupCommunication   : return QIcon::fromTheme("network-workgroup");//FIXME
    case Transaction::GroupDesktopGnome    : return QIcon::fromTheme("kpk-desktop-gnome");
    case Transaction::GroupDesktopKde      : return QIcon::fromTheme("kde");
    case Transaction::GroupDesktopOther    : return QIcon::fromTheme("user-desktop");
    case Transaction::GroupDesktopXfce     : return QIcon::fromTheme("kpk-desktop-xfce");
    case Transaction::GroupDocumentation   : return QIcon::fromTheme("accessories-dictionary");//FIXME
    case Transaction::GroupEducation       : return QIcon::fromTheme("applications-education");
    case Transaction::GroupElectronics     : return QIcon::fromTheme("media-flash");
    case Transaction::GroupFonts           : return QIcon::fromTheme("preferences-desktop-font");
    case Transaction::GroupGames           : return QIcon::fromTheme("applications-games");
    case Transaction::GroupGraphics        : return QIcon::fromTheme("applications-graphics");
    case Transaction::GroupInternet        : return QIcon::fromTheme("applications-internet");
    case Transaction::GroupLegacy          : return QIcon::fromTheme("media-floppy");
    case Transaction::GroupLocalization    : return QIcon::fromTheme("applications-education-language");
    case Transaction::GroupMaps            : return QIcon::fromTheme("Maps");//FIXME
    case Transaction::GroupCollections     : return QIcon::fromTheme("package-orign");
    case Transaction::GroupMultimedia      : return QIcon::fromTheme("applications-multimedia");
    case Transaction::GroupNetwork         : return QIcon::fromTheme("network-wired");
    case Transaction::GroupOffice          : return QIcon::fromTheme("applications-office");
    case Transaction::GroupOther           : return QIcon::fromTheme("applications-other");
    case Transaction::GroupPowerManagement : return QIcon::fromTheme("battery");
    case Transaction::GroupProgramming     : return QIcon::fromTheme("applications-development");
    case Transaction::GroupPublishing      : return QIcon::fromTheme("accessories-text-editor");
    case Transaction::GroupRepos           : return QIcon::fromTheme("application-x-compressed-tar");
    case Transaction::GroupScience         : return QIcon::fromTheme("applications-science");
    case Transaction::GroupSecurity        : return QIcon::fromTheme("security-high");
    case Transaction::GroupServers         : return QIcon::fromTheme("network-server");
    case Transaction::GroupSystem          : return QIcon::fromTheme("applications-system");
    case Transaction::GroupVirtualization  : return QIcon::fromTheme("cpu");
    case Transaction::GroupVendor          : return QIcon::fromTheme("application-certificate");
    case Transaction::GroupNewest          : return QIcon::fromTheme("dialog-information");
    }
    kDebug() << "group unrecognised: " << group;
    return QIcon::fromTheme("unknown");
}

QIcon PkIcons::packageIcon(Transaction::Info info)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }
    switch (info) {
    case Transaction::InfoBugfix      : return QIcon::fromTheme("script-error");
    case Transaction::InfoEnhancement : return QIcon::fromTheme("ktip");
    case Transaction::InfoImportant   : return QIcon::fromTheme("security-medium");
    case Transaction::InfoLow         : return QIcon::fromTheme("security-high");
    case Transaction::InfoSecurity    : return QIcon::fromTheme("security-low");
    case Transaction::InfoNormal      : return QIcon::fromTheme("emblem-new");
    case Transaction::InfoBlocked     : return QIcon::fromTheme("dialog-cancel");
    case Transaction::InfoAvailable   : return QIcon::fromTheme("package-download");
    case Transaction::InfoInstalled   : return QIcon::fromTheme("package-installed");
    default                    : return QIcon::fromTheme("package");
    }
}

QString PkIcons::restartIconName(Transaction::Restart type)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }
    // These names MUST be standard icons, otherwise KStatusNotifierItem
    // will not be able to load them
    switch (type) {
    case Transaction::RestartSecuritySystem  :
    case Transaction::RestartSystem          : return "system-reboot";
    case Transaction::RestartSecuritySession :
    case Transaction::RestartSession         : return "system-log-out";
    case Transaction::RestartApplication     : return "process-stop";
    case Transaction::RestartNone            :
    case Transaction::RestartUnknown         : break;
    }
    return "";
}

QIcon PkIcons::restartIcon(Transaction::Restart type)
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
    icon.addPixmap(QIcon::fromTheme(name).pixmap(48, 48));
    return icon;
}

QString PkIcons::lastCacheRefreshIconName(uint lastTime)
{
    unsigned long fifteen = 60 * 60 * 24 * 15;
    unsigned long tirty = 60 * 60 * 24 * 30;

    if (lastTime != UINT_MAX && lastTime < fifteen) {
        return QLatin1String("security-high");
    } else if (lastTime != UINT_MAX && lastTime > fifteen && lastTime < tirty) {
        return QLatin1String("security-medium");
    }
    return QLatin1String("security-low");
}

