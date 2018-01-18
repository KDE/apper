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

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(APPER_LIB, "apper.lib")

//#include <KGlobal>

using namespace PackageKit;

bool PkIcons::init = false;
QHash<QString, QIcon> PkIcons::cache = QHash<QString, QIcon>();

void PkIcons::configure()
{
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() +
                               QStringList{QLatin1String("xdgdata-pixmap"),
                                           QLatin1String("/usr/share/app-info/icons/"),
                                           QLatin1String("/usr/share/app-install/icons/")});
//    KStandardDirs:: KGlobal::dirs()->addResourceDir("xdgdata-pixmap", "/usr/share/app-info/icons/", "/usr/share/app-install/icons/");
//    KIconLoader::global()->reconfigure("apper");
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
    case Transaction::StatusUnknown              : return QLatin1String("help-browser");
    case Transaction::StatusCancel               :
    case Transaction::StatusCleanup              : return QLatin1String("package-clean-up");
    case Transaction::StatusCommit               : return QLatin1String("package-working");//TODO needs a better icon
    case Transaction::StatusDepResolve           : return QLatin1String("package-info");
    case Transaction::StatusDownloadChangelog    :
    case Transaction::StatusDownloadFilelist     :
    case Transaction::StatusDownloadGroup        :
    case Transaction::StatusDownloadPackagelist  : return QLatin1String("refresh-cache");
    case Transaction::StatusDownload             : return QLatin1String("package-download");
    case Transaction::StatusDownloadRepository   :
    case Transaction::StatusDownloadUpdateinfo   : return QLatin1String("refresh-cache");
    case Transaction::StatusFinished             : return QLatin1String("package-clean-up");
    case Transaction::StatusGeneratePackageList  : return QLatin1String("refresh-cache");
    case Transaction::StatusWaitingForLock       : return QLatin1String("dialog-password");
    case Transaction::StatusWaitingForAuth       : return QLatin1String("dialog-password");//IMPROVE ME
    case Transaction::StatusInfo                 : return QLatin1String("package-info");
    case Transaction::StatusInstall              : return QLatin1String("kpk-package-add");
    case Transaction::StatusLoadingCache         : return QLatin1String("refresh-cache");
    case Transaction::StatusObsolete             : return QLatin1String("package-clean-up");
    case Transaction::StatusQuery                : return QLatin1String("search-package");
    case Transaction::StatusRefreshCache         : return QLatin1String("refresh-cache");
    case Transaction::StatusRemove               : return QLatin1String("package-removed");
    case Transaction::StatusRepackaging          : return QLatin1String("package-clean-up");
    case Transaction::StatusRequest              : return QLatin1String("search-package");
    case Transaction::StatusRunning              : return QLatin1String("package-working");
    case Transaction::StatusScanApplications     : return QLatin1String("search-package");
    case Transaction::StatusSetup                : return QLatin1String("package-working");
    case Transaction::StatusSigCheck             :
    case Transaction::StatusTestCommit           : return QLatin1String("package-info");//TODO needs a better icon
    case Transaction::StatusUpdate               : return QLatin1String("package-update");
    case Transaction::StatusWait                 : return QLatin1String("package-wait");
    case Transaction::StatusScanProcessList      : return QLatin1String("package-info");
    case Transaction::StatusCheckExecutableFiles : return QLatin1String("package-info");
    case Transaction::StatusCheckLibraries       : return QLatin1String("package-info");
    case Transaction::StatusCopyFiles            : return QLatin1String("package-info");
    case Transaction::StatusRunHook              : return QLatin1String("package-info");
    }
    qCDebug(APPER_LIB) << "status icon unrecognised: " << status;
    return QLatin1String("help-browser");
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
    case Transaction::StatusUnknown             : return QLatin1String("help-browser");
    case Transaction::StatusCancel              :
    case Transaction::StatusCleanup             : return QLatin1String("pk-cleaning-up");
    case Transaction::StatusCommit              :
    case Transaction::StatusDepResolve          : return QLatin1String("pk-testing");
    case Transaction::StatusDownloadChangelog   :
    case Transaction::StatusDownloadFilelist    :
    case Transaction::StatusDownloadGroup       :
    case Transaction::StatusDownloadPackagelist : return QLatin1String("pk-refresh-cache");
    case Transaction::StatusDownload            : return QLatin1String("pk-downloading");
    case Transaction::StatusDownloadRepository  :
    case Transaction::StatusDownloadUpdateinfo  : return QLatin1String("pk-refresh-cache");
    case Transaction::StatusFinished            : return QLatin1String("pk-cleaning-up");
    case Transaction::StatusGeneratePackageList : return QLatin1String("pk-searching");
    case Transaction::StatusWaitingForLock      : return QLatin1String("pk-waiting");
    case Transaction::StatusInfo                : return QLatin1String("package-working");
    case Transaction::StatusInstall             : return QLatin1String("pk-installing");
    case Transaction::StatusLoadingCache        : return QLatin1String("pk-refresh-cache");
    case Transaction::StatusObsolete            : return QLatin1String("pk-cleaning-up");
    case Transaction::StatusQuery               : return QLatin1String("pk-searching");
    case Transaction::StatusRefreshCache        : return QLatin1String("pk-refresh-cache");
    case Transaction::StatusRemove              : return QLatin1String("package-removed");//TODO do the animation
    case Transaction::StatusRepackaging         : return QLatin1String("pk-searching");
    case Transaction::StatusRequest             : return QLatin1String("process-working");
    case Transaction::StatusRunning             : return QLatin1String("pk-testing");
    case Transaction::StatusScanApplications    : return QLatin1String("pk-searching");
    case Transaction::StatusSetup               : return QLatin1String("pk-searching");
    case Transaction::StatusSigCheck            : return QLatin1String("package-info");
    case Transaction::StatusTestCommit          : return QLatin1String("pk-testing");
    case Transaction::StatusUpdate              : return QLatin1String("pk-installing");
    case Transaction::StatusWait                : return QLatin1String("pk-waiting");
    case Transaction::StatusWaitingForAuth      : return QLatin1String("dialog-password");
    case Transaction::StatusScanProcessList     : return QLatin1String("utilities-system-monitor");
    default                              : qCDebug(APPER_LIB) << "status icon unrecognised: " << status;
        return QLatin1String("help-browser");
    }
}

QString PkIcons::actionIconName(Transaction::Role role)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }
    switch (role) {
    case Transaction::RoleUnknown                 : return QLatin1String("applications-other");
    case Transaction::RoleAcceptEula              : return QLatin1String("package-info");
    case Transaction::RoleCancel                  : return QLatin1String("process-stop");
    case Transaction::RoleDownloadPackages        : return QLatin1String("package-download");
    case Transaction::RoleGetCategories           : return QLatin1String("package-info");
    case Transaction::RoleDependsOn               : return QLatin1String("package-info");
    case Transaction::RoleGetDetails              : return QLatin1String("package-info");
    case Transaction::RoleGetDetailsLocal         : return QLatin1String("package-info");
    case Transaction::RoleGetDistroUpgrades       : return QLatin1String("distro-upgrade");
    case Transaction::RoleGetFiles                : return QLatin1String("search-package");
    case Transaction::RoleGetFilesLocal           : return QLatin1String("search-package");
    case Transaction::RoleGetOldTransactions      : return QLatin1String("package-info");
    case Transaction::RoleGetPackages             : return QLatin1String("package-packages");
    case Transaction::RoleGetRepoList             : return QLatin1String("package-orign");
    case Transaction::RoleRequiredBy              : return QLatin1String("package-info");
    case Transaction::RoleGetUpdateDetail         : return QLatin1String("package-info");
    case Transaction::RoleGetUpdates              : return QLatin1String("package-info");
    case Transaction::RoleInstallFiles            : return QLatin1String("package-installed");
    case Transaction::RoleInstallPackages         : return QLatin1String("package-installed");
    case Transaction::RoleInstallSignature        : return QLatin1String("package-installed");
    case Transaction::RoleRefreshCache            : return QLatin1String("refresh-cache");
    case Transaction::RoleRemovePackages          : return QLatin1String("package-removed");
    case Transaction::RoleRepoEnable              : return QLatin1String("package-orign");
    case Transaction::RoleRepoSetData             : return QLatin1String("package-orign");
    case Transaction::RoleRepoRemove              : return QLatin1String("package-orign");
    case Transaction::RoleResolve                 : return QLatin1String("search-package");
    case Transaction::RoleSearchDetails           : return QLatin1String("search-package");
    case Transaction::RoleSearchFile              : return QLatin1String("search-package");
    case Transaction::RoleSearchGroup             : return QLatin1String("search-package");
    case Transaction::RoleSearchName              : return QLatin1String("search-package");
    case Transaction::RoleUpdatePackages          : return QLatin1String("package-update");
    case Transaction::RoleWhatProvides            : return QLatin1String("search-package");
    case Transaction::RoleRepairSystem            : return QLatin1String("package-rollback");
    case Transaction::RoleUpgradeSystem           : return QLatin1String("package-update");
    }
    qCDebug(APPER_LIB) << "action unrecognised: " << role;
    return QLatin1String("applications-other");
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
    case Transaction::GroupUnknown         : return QIcon::fromTheme(QLatin1String("unknown"));
    case Transaction::GroupAccessibility   : return QIcon::fromTheme(QLatin1String("preferences-desktop-accessibility"));
    case Transaction::GroupAccessories     : return QIcon::fromTheme(QLatin1String("applications-accessories"));
    case Transaction::GroupAdminTools      : return QIcon::fromTheme(QLatin1String("dialog-password"));
    case Transaction::GroupCommunication   : return QIcon::fromTheme(QLatin1String("network-workgroup"));//FIXME
    case Transaction::GroupDesktopGnome    : return QIcon::fromTheme(QLatin1String("kpk-desktop-gnome"));
    case Transaction::GroupDesktopKde      : return QIcon::fromTheme(QLatin1String("kde"));
    case Transaction::GroupDesktopOther    : return QIcon::fromTheme(QLatin1String("user-desktop"));
    case Transaction::GroupDesktopXfce     : return QIcon::fromTheme(QLatin1String("kpk-desktop-xfce"));
    case Transaction::GroupDocumentation   : return QIcon::fromTheme(QLatin1String("accessories-dictionary"));//FIXME
    case Transaction::GroupEducation       : return QIcon::fromTheme(QLatin1String("applications-education"));
    case Transaction::GroupElectronics     : return QIcon::fromTheme(QLatin1String("media-flash"));
    case Transaction::GroupFonts           : return QIcon::fromTheme(QLatin1String("preferences-desktop-font"));
    case Transaction::GroupGames           : return QIcon::fromTheme(QLatin1String("applications-games"));
    case Transaction::GroupGraphics        : return QIcon::fromTheme(QLatin1String("applications-graphics"));
    case Transaction::GroupInternet        : return QIcon::fromTheme(QLatin1String("applications-internet"));
    case Transaction::GroupLegacy          : return QIcon::fromTheme(QLatin1String("media-floppy"));
    case Transaction::GroupLocalization    : return QIcon::fromTheme(QLatin1String("applications-education-language"));
    case Transaction::GroupMaps            : return QIcon::fromTheme(QLatin1String("Maps"));//FIXME
    case Transaction::GroupCollections     : return QIcon::fromTheme(QLatin1String("package-orign"));
    case Transaction::GroupMultimedia      : return QIcon::fromTheme(QLatin1String("applications-multimedia"));
    case Transaction::GroupNetwork         : return QIcon::fromTheme(QLatin1String("network-wired"));
    case Transaction::GroupOffice          : return QIcon::fromTheme(QLatin1String("applications-office"));
    case Transaction::GroupOther           : return QIcon::fromTheme(QLatin1String("applications-other"));
    case Transaction::GroupPowerManagement : return QIcon::fromTheme(QLatin1String("battery"));
    case Transaction::GroupProgramming     : return QIcon::fromTheme(QLatin1String("applications-development"));
    case Transaction::GroupPublishing      : return QIcon::fromTheme(QLatin1String("accessories-text-editor"));
    case Transaction::GroupRepos           : return QIcon::fromTheme(QLatin1String("application-x-compressed-tar"));
    case Transaction::GroupScience         : return QIcon::fromTheme(QLatin1String("applications-science"));
    case Transaction::GroupSecurity        : return QIcon::fromTheme(QLatin1String("security-high"));
    case Transaction::GroupServers         : return QIcon::fromTheme(QLatin1String("network-server"));
    case Transaction::GroupSystem          : return QIcon::fromTheme(QLatin1String("applications-system"));
    case Transaction::GroupVirtualization  : return QIcon::fromTheme(QLatin1String("cpu"));
    case Transaction::GroupVendor          : return QIcon::fromTheme(QLatin1String("application-certificate"));
    case Transaction::GroupNewest          : return QIcon::fromTheme(QLatin1String("dialog-information"));
    }
    qCDebug(APPER_LIB) << "group unrecognised: " << group;
    return QIcon::fromTheme(QLatin1String("unknown"));
}

QIcon PkIcons::packageIcon(Transaction::Info info)
{
    if (!PkIcons::init) {
        PkIcons::configure();
    }
    switch (info) {
    case Transaction::InfoBugfix      : return QIcon::fromTheme(QLatin1String("script-error"));
    case Transaction::InfoEnhancement : return QIcon::fromTheme(QLatin1String("ktip"));
    case Transaction::InfoImportant   : return QIcon::fromTheme(QLatin1String("security-medium"));
    case Transaction::InfoLow         : return QIcon::fromTheme(QLatin1String("security-high"));
    case Transaction::InfoSecurity    : return QIcon::fromTheme(QLatin1String("security-low"));
    case Transaction::InfoNormal      : return QIcon::fromTheme(QLatin1String("emblem-new"));
    case Transaction::InfoBlocked     : return QIcon::fromTheme(QLatin1String("dialog-cancel"));
    case Transaction::InfoAvailable   : return QIcon::fromTheme(QLatin1String("package-download"));
    case Transaction::InfoInstalled   : return QIcon::fromTheme(QLatin1String("package-installed"));
    default                    : return QIcon::fromTheme(QLatin1String("package"));
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
    case Transaction::RestartSystem          : return QLatin1String("system-reboot");
    case Transaction::RestartSecuritySession :
    case Transaction::RestartSession         : return QLatin1String("system-log-out");
    case Transaction::RestartApplication     : return QLatin1String("process-stop");
    case Transaction::RestartNone            :
    case Transaction::RestartUnknown         : break;
    }
    return QLatin1String("");
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

    qCDebug(APPER_LIB) << KIconLoader::global()->iconPath(name, KIconLoader::NoGroup, true);
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

#include "moc_PkIcons.cpp"
