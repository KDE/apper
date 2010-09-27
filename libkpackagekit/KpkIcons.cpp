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

#include "config.h"

#include <KIconLoader>
#include <KStandardDirs>

#include <KDebug>

bool KpkIcons::init = false;
QHash<QString, KIcon> KpkIcons::cache = QHash<QString, KIcon>();

void KpkIcons::configure()
{
    KGlobal::dirs()->addResourceDir("pixmap", "/usr/share/app-install/icons/");
    KIconLoader::global()->addAppDir("kpackagekit");
    KIconLoader::global()->reconfigure("kpackagekit", 0);
    KpkIcons::init = true;
}

KIcon KpkIcons::getIcon(const QString &name)
{
//     kDebug() << 1 << name;
    if (!KpkIcons::init) {
        KpkIcons::configure();
    }

    if (!KpkIcons::cache.contains(name)) {
        KpkIcons::cache[name] = KIcon(name);
    }
    return KpkIcons::cache[name];
}

KIcon KpkIcons::getIcon(const QString &name, const QString &defaultName)
{
//     kDebug() << 2 << name << defaultName;
    if (!KpkIcons::init) {
        KpkIcons::configure();
    }

    if (name.isEmpty()) {
        return KIcon();
    }

    if (!KpkIcons::cache.contains(name)) {
//         kDebug() << KIconLoader::global()->iconPath(name, KIconLoader::NoGroup) << name;
        QPixmap icon;
        icon = KIconLoader::global()->loadIcon(name,
                                            KIconLoader::NoGroup,
                                            KIconLoader::SizeLarge,
                                            KIconLoader::DefaultState,
                                            QStringList(),
                                            NULL,
                                            true);
        if (icon.isNull() && !defaultName.isNull()) {
            KpkIcons::cache[name] = KIcon(defaultName);
        } else if (icon.isNull()) {
            return KIcon();
        } else {
            KpkIcons::cache[name] = KIcon(name);
        }
    }
    return KpkIcons::cache[name];
}

QString KpkIcons::statusIconName(Enum::Status status)
{
    switch (status) {
    case Enum::LastStatus                 :
    case Enum::UnknownStatus              : return "help-browser";
    case Enum::StatusCancel               :
    case Enum::StatusCleanup              : return "kpk-clean-up";
    case Enum::StatusCommit               : return "package-setup";//TODO needs a better icon
    case Enum::StatusDepResolve           : return "package-info";
    case Enum::StatusDownloadChangelog    :
    case Enum::StatusDownloadFilelist     :
    case Enum::StatusDownloadGroup        :
    case Enum::StatusDownloadPackagelist  : return "kpk-refresh-cache";
    case Enum::StatusDownload             : return "package-download";
    case Enum::StatusDownloadRepository   :
    case Enum::StatusDownloadUpdateinfo   : return "kpk-refresh-cache";
    case Enum::StatusFinished             : return "kpk-clean-up";
    case Enum::StatusGeneratePackageList  : return "kpk-refresh-cache";
    case Enum::StatusWaitingForLock       : return "dialog-password";
    case Enum::StatusWaitingForAuth       : return "dialog-password";//IMPROVE ME
    case Enum::StatusInfo                 : return "package-info";
    case Enum::StatusInstall              : return "kpk-package-add";
    case Enum::StatusLoadingCache         : return "kpk-refresh-cache";
    case Enum::StatusObsolete             : return "kpk-clean-up";
    case Enum::StatusQuery                : return "package-search";
    case Enum::StatusRefreshCache         : return "kpk-refresh-cache";
    case Enum::StatusRemove               : return "package-removed";
    case Enum::StatusRepackaging          : return "kpk-clean-up";
    case Enum::StatusRequest              : return "package-search";
    case Enum::StatusRollback             : return "package-rollback";
    case Enum::StatusRunning              : return "package-setup";
    case Enum::StatusScanApplications     : return "package-search";
    case Enum::StatusSetup                : return "package-setup";
    case Enum::StatusSigCheck             :
    case Enum::StatusTestCommit           : return "package-info";//TODO needs a better icon
    case Enum::StatusUpdate               : return "package-update";
    case Enum::StatusWait                 : return "package-wait";
    case Enum::StatusScanProcessList      : return "package-info";
    case Enum::StatusCheckExecutableFiles : return "package-info";
    case Enum::StatusCheckLibraries       : return "package-info";
    case Enum::StatusCopyFiles            : return "package-info";
    }
    kDebug() << "status icon unrecognised: " << status;
    return "help-browser";
}

KIcon KpkIcons::statusIcon(Enum::Status status)
{
    return KIcon(KpkIcons::statusIconName(status));
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
    default                              : kDebug() << "status icon unrecognised: " << status;
        return "help-browser";
    }
}

QString KpkIcons::actionIconName(Enum::Role role)
{
    switch (role) {
    case Enum::LastRole                    :
    case Enum::UnknownRole                 : return "applications-other";
    case Enum::RoleAcceptEula              : return "package-info";
    case Enum::RoleCancel                  : return "process-stop";
    case Enum::RoleDownloadPackages        : return "package-download";
    case Enum::RoleGetCategories           : return "package-info";
    case Enum::RoleGetDepends              : return "package-info";
    case Enum::RoleGetDetails              : return "package-info";
    case Enum::RoleGetDistroUpgrades       : return "distro-upgrade";
    case Enum::RoleGetFiles                : return "package-search";
    case Enum::RoleGetOldTransactions      : return "package-info";
    case Enum::RoleGetPackages             : return "package-packages";
    case Enum::RoleGetRepoList             : return "package-orign";
    case Enum::RoleGetRequires             : return "package-info";
    case Enum::RoleGetUpdateDetail         : return "package-info";
    case Enum::RoleGetUpdates              : return "package-info";
    case Enum::RoleInstallFiles            : return "package-installed";
    case Enum::RoleInstallPackages         : return "package-installed";
    case Enum::RoleInstallSignature        : return "package-installed";
    case Enum::RoleRefreshCache            : return "kpk-refresh-cache";
    case Enum::RoleRemovePackages          : return "package-removed";
    case Enum::RoleRepoEnable              : return "package-orign";
    case Enum::RoleRepoSetData             : return "package-orign";
    case Enum::RoleResolve                 : return "package-search";
    case Enum::RoleRollback                : return "package-rollback";
    case Enum::RoleSearchDetails           : return "package-search";
    case Enum::RoleSearchFile              : return "package-search";
    case Enum::RoleSearchGroup             : return "package-search";
    case Enum::RoleSearchName              : return "package-search";
    case Enum::RoleUpdatePackages          : return "package-update";
    case Enum::RoleUpdateSystem            : return "distro-upgrade";//TODO
    case Enum::RoleWhatProvides            : return "package-search";
    case Enum::RoleSimulateInstallFiles    : return "package-installed";
    case Enum::RoleSimulateInstallPackages : return "package-installed";
    case Enum::RoleSimulateRemovePackages  : return "package-removed";
    case Enum::RoleSimulateUpdatePackages  : return "package-update'";
    }
    kDebug() << "action unrecognised: " << role;
    return "applications-other";
}

KIcon KpkIcons::actionIcon(Enum::Role role)
{
    return KIcon(actionIconName(role));
}

KIcon KpkIcons::groupsIcon(Enum::Group group)
{
    switch (group) {
    case Enum::LastGroup            :
    case Enum::UnknownGroup         : return KIcon("unknown");
    case Enum::GroupAccessibility   : return KIcon("preferences-desktop-accessibility");
    case Enum::GroupAccessories     : return KIcon("applications-accessories");
    case Enum::GroupAdminTools      : return KIcon("dialog-password");
    case Enum::GroupCommunication   : return KIcon("network-workgroup");//FIXME
    case Enum::GroupDesktopGnome    : return KIcon("kpk-desktop-gnome");
    case Enum::GroupDesktopKde      : return KIcon("kde");
    case Enum::GroupDesktopOther    : return KIcon("user-desktop");
    case Enum::GroupDesktopXfce     : return KIcon("kpk-desktop-xfce");
    case Enum::GroupDocumentation   : return KIcon("accessories-dictionary");//FIXME
    case Enum::GroupEducation       : return KIcon("applications-education");
    case Enum::GroupElectronics     : return KIcon("media-flash");
    case Enum::GroupFonts           : return KIcon("preferences-desktop-font");
    case Enum::GroupGames           : return KIcon("applications-games");
    case Enum::GroupGraphics        : return KIcon("applications-graphics");
    case Enum::GroupInternet        : return KIcon("applications-internet");
    case Enum::GroupLegacy          : return KIcon("media-floppy");
    case Enum::GroupLocalization    : return KIcon("applications-education-language");
    case Enum::GroupMaps            : return KIcon("Maps");//FIXME
    case Enum::GroupCollections     : return KIcon("package-orign");
    case Enum::GroupMultimedia      : return KIcon("applications-multimedia");
    case Enum::GroupNetwork         : return KIcon("network-wired");
    case Enum::GroupOffice          : return KIcon("applications-office");
    case Enum::GroupOther           : return KIcon("applications-other");
    case Enum::GroupPowerManagement : return KIcon("battery");
    case Enum::GroupProgramming     : return KIcon("applications-development");
    case Enum::GroupPublishing      : return KIcon("accessories-text-editor");
    case Enum::GroupRepos           : return KIcon("application-x-compressed-tar");
    case Enum::GroupScience         : return KIcon("applications-science");
    case Enum::GroupSecurity        : return KIcon("security-high");
    case Enum::GroupServers         : return KIcon("network-server");
    case Enum::GroupSystem          : return KIcon("applications-system");
    case Enum::GroupVirtualization  : return KIcon("cpu");
    case Enum::GroupVendor          : return KIcon("application-certificate");
    case Enum::GroupNewest          : return KIcon("dialog-information");
    }
    kDebug() << "group unrecognised: " << group;
    return KIcon("unknown");
}

KIcon KpkIcons::packageIcon(Enum::Info info)
{
    switch (info) {
    case Enum::InfoBugfix      : return KIcon("script-error");
    case Enum::InfoImportant   : return KIcon("security-low");
    case Enum::InfoLow         : return KIcon("security-high");
    case Enum::InfoEnhancement : return KIcon("ktip");
    case Enum::InfoSecurity    : return KIcon("emblem-important");
    case Enum::InfoNormal      : return KIcon("security-medium");
    case Enum::InfoBlocked     : return KIcon("dialog-cancel");
    case Enum::InfoAvailable   : return KIcon("package-download");
    case Enum::InfoInstalled   : return KIcon("package-installed");
    default                    : return KIcon("package");
    }
}

QString KpkIcons::restartIconName(Enum::Restart type)
{
    // These names MUST be standard icons, otherwise KStatusNotifierItem
    // will not be able to load them
    switch (type) {
    case Enum::RestartSecuritySystem  :
    case Enum::RestartSystem          : return "system-reboot";
    case Enum::RestartSecuritySession :
    case Enum::RestartSession         : return "system-log-out";
    case Enum::RestartApplication     : return "process-stop";
    case Enum::RestartNone            :
    case Enum::LastRestart            :
    case Enum::UnknownRestart         : break;
    }
    return "";
}

KIcon KpkIcons::restartIcon(Enum::Restart type)
{
    return KpkIcons::getIcon(restartIconName(type));
}

QIcon KpkIcons::getPreloadedIcon(const QString &name)
{
    if (!KpkIcons::init) {
        KpkIcons::configure();
    }

kDebug() << KIconLoader::global()->iconPath(name, KIconLoader::NoGroup);
    QIcon icon;
    icon.addPixmap(KIcon(name).pixmap(48, 48));
    return icon;
}

