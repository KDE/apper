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

#ifndef KPKICONS_H
#define KPKICONS_H

#include <QPackageKit>
#include <KIcon>

using namespace PackageKit;

class KDE_EXPORT KpkIcons {
    public:
        static KIcon   groupsIcon(Client::Group group);
        static KIcon   statusIcon(PackageKit::Transaction::Status status);
        static QString statusAnimation(PackageKit::Transaction::Status status);
        static KIcon   actionIcon(Client::Action action);
        static KIcon   packageIcon(Package::State state);
        static KIcon   restartIcon(Client::RestartType type);
        static KIcon   getIcon(const QString &name);

    private:
        static QHash<QString, KIcon> cache;
        static bool init;
};

#endif
