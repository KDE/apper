/***************************************************************************
 *   Copyright (C) 2008 by Trever Fischer                                  *
 *   wm161@wm161.net                                                       *
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
    case Transaction::Setup :
        return KpkIcons::getIcon("package-setup");
    case Transaction::Wait :
        return KpkIcons::getIcon("package-wait");
    case Transaction::Running :
        return KpkIcons::getIcon("package-setup");
    case Transaction::Query :
        return KpkIcons::getIcon("package-search");
    case Transaction::Info :
        return KpkIcons::getIcon("package-info");
    case Transaction::Remove :
        return KpkIcons::getIcon("package-removed");
    case Transaction::Download :
        return KpkIcons::getIcon("package-download");
    case Transaction::Update :
        return KpkIcons::getIcon("package-update");
    case Transaction::DepResolve :
        return KpkIcons::getIcon("package-info");
    case Transaction::SigCheck :
        return KpkIcons::getIcon("package-info");//TODO needs a better icon
    case Transaction::Rollback :
        return KpkIcons::getIcon("package-rollback");
    case Transaction::TestCommit :
        return KpkIcons::getIcon("package-info");//TODO needs a better icon
    case Transaction::Commit :
        return KpkIcons::getIcon("package-setup");//TODO needs a better icon
    case Transaction::Request :
        return KpkIcons::getIcon("package-search");
    default :
        kDebug() << "status icon unrecognised: " << status;
        return KpkIcons::getIcon("applications-other");
    }
}

KIcon KpkIcons::actionIcon(Client::Action action)
{
    switch (action) {
    case Client::ActionGetDepends :
        return KpkIcons::getIcon("package-info");
    case Client::ActionGetDetails :
        return KpkIcons::getIcon("package-info");
    case Client::ActionGetFiles :
        return KpkIcons::getIcon("package-search");
    case Client::ActionGetPackages :
        return KpkIcons::getIcon("package-packages");
    case Client::ActionGetRepoList :
        return KpkIcons::getIcon("package-orign");
    case Client::ActionGetRequires :
        return KpkIcons::getIcon("package-info");
    case Client::ActionGetUpdateDetail :
        return KpkIcons::getIcon("package-info");
    case Client::ActionGetUpdates :
        return KpkIcons::getIcon("package-info");
    case Client::ActionRepoEnable :
        return KpkIcons::getIcon("package-orign");
    case Client::ActionRepoSetData :
        return KpkIcons::getIcon("package-orign");
    case Client::ActionResolve :
        return KpkIcons::getIcon("package-search");
    case Client::ActionRollback :
        return KpkIcons::getIcon("package-rollback");
    case Client::ActionSearchDetails :
        return KpkIcons::getIcon("package-search");
    case Client::ActionSearchFile :
        return KpkIcons::getIcon("package-search");
    case Client::ActionSearchGroup :
        return KpkIcons::getIcon("package-search");
    case Client::ActionSearchName :
        return KpkIcons::getIcon("package-search");
    case Client::ActionUpdatePackages :
        return KpkIcons::getIcon("package-update");
    case Client::ActionUpdateSystem :
        return KpkIcons::getIcon("distro-upgrade");//TODO
    case Client::ActionWhatProvides :
        return KpkIcons::getIcon("package-search");
    case Client::ActionAcceptEula :
        return KpkIcons::getIcon("package-info");
    case Client::ActionDownloadPackages :
        return KpkIcons::getIcon("package-download");
    case Client::ActionGetDistroUpgrades :
        return KpkIcons::getIcon("distro-upgrade");
    case Client::ActionInstallPackages :
        return KpkIcons::getIcon("package-installed");
    case Client::ActionRemovePackages :
        return KpkIcons::getIcon("package-removed");
    default :
        kDebug() << "action unrecognised: " << action;
        return KpkIcons::getIcon("applications-other");
    }
}

KIcon KpkIcons::groupsIcon(Client::Group group)
{
    switch (group) {
    case Client::Accessibility :
        return KpkIcons::getIcon("preferences-desktop-accessibility");
    case Client::Accessories :
        return KpkIcons::getIcon("applications-accessories");
    case Client::AdminTools :
        return KpkIcons::getIcon("dialog-password");
    case Client::Communication :
        return KpkIcons::getIcon("network-workgroup");//FIXME
    case Client::DesktopGnome :
        return KpkIcons::getIcon("user-desktop");//FIXME
    case Client::DesktopKde :
        return KpkIcons::getIcon("kde");
    case Client::DesktopOther :
        return KpkIcons::getIcon("user-desktop");
    case Client::DesktopXfce :
        return KpkIcons::getIcon("user-desktop");//FIXME
    case Client::Documentation :
        return KpkIcons::getIcon("accessories-dictionary");//FIXME
    case Client::Education :
        return KpkIcons::getIcon("applications-education");
    case Client::Electronics :
        return KpkIcons::getIcon("media-flash");
    case Client::Fonts :
        return KpkIcons::getIcon("preferences-desktop-font");
    case Client::Games :
        return KpkIcons::getIcon("applications-games");
    case Client::Graphics :
        return KpkIcons::getIcon("applications-graphics");
    case Client::Internet :
        return KpkIcons::getIcon("applications-internet");
    case Client::Legacy :
        return KpkIcons::getIcon("media-floppy");
    case Client::Localization :
        return KpkIcons::getIcon("applications-education-language");
    case Client::Maps :
        return KpkIcons::getIcon("Maps");//FIXME
    case Client::Collections :
        return KpkIcons::getIcon("unknown");//FIXME
    case Client::Multimedia :
        return KpkIcons::getIcon("applications-multimedia");
    case Client::Network :
        return KpkIcons::getIcon("network-wired");
    case Client::Office :
        return KpkIcons::getIcon("applications-office");
    case Client::Other :
        return KpkIcons::getIcon("applications-other");
    case Client::PowerManagement :
        return KpkIcons::getIcon("battery");
    case Client::Programming :
        return KpkIcons::getIcon("applications-development");
    case Client::Publishing :
        return KpkIcons::getIcon("accessories-text-editor");
    case Client::Repos :
        return KpkIcons::getIcon("application-x-compressed-tar");
    case Client::Science :
        return KpkIcons::getIcon("applications-science");
    case Client::Security :
        return KpkIcons::getIcon("security-high");
    case Client::Servers :
        return KpkIcons::getIcon("network-server");
    case Client::System :
        return KpkIcons::getIcon("applications-system");
    case Client::Virtualization :
        return KpkIcons::getIcon("cpu");
    case Client::UnknownGroup :
        return KpkIcons::getIcon("unknown");
    default :
        kDebug() << "group unrecognised: " << group;
        return KpkIcons::getIcon("unknown");
    }
}

KIcon KpkIcons::packageIcon(Package::State state)
{
    switch (state) {
    case Package::Bugfix :
        return KpkIcons::getIcon("script-error");
    case Package::Important :
        return KpkIcons::getIcon("security-low");
    case Package::Low :
        return KpkIcons::getIcon("security-high");
    case Package::Enhancement :
        return KpkIcons::getIcon("ktip");
    case Package::Security :
        return KpkIcons::getIcon("emblem-important");
    case Package::Normal :
        return KpkIcons::getIcon("security-medium");
    case Package::Blocked :
        return KpkIcons::getIcon("dialog-cancel");
    case Package::Available:
        return KpkIcons::getIcon("package-download");
    case Package::Installed:
        return KpkIcons::getIcon("package-installed");
    default :
        return KpkIcons::getIcon("package");
    }
}
