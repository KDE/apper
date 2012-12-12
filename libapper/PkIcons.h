/***************************************************************************
 *   Copyright (C) 2008 by Trever Fischer                                  *
 *   wm161@wm161.net                                                       *
 *   Copyright (C) 2010-2012 by Daniel Nicoletti                           *
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

#ifndef PK_ICONS_H
#define PK_ICONS_H

#include <QObject>

#include <Transaction>
#include <KIcon>

#define KPK_ICON_SIZE 64

class KDE_EXPORT PkIcons : public QObject
{
    Q_OBJECT
public slots:
    static KIcon   groupsIcon(PackageKit::Transaction::Group group);
    static QString statusIconName(PackageKit::Transaction::Status status);
    static KIcon   statusIcon(PackageKit::Transaction::Status status);
    static QString statusAnimation(PackageKit::Transaction::Status status);
    static QString actionIconName(PackageKit::Transaction::Role role);
    static KIcon   actionIcon(PackageKit::Transaction::Role role);
    static KIcon   packageIcon(PackageKit::Transaction::Info state);
    static QString restartIconName(PackageKit::Transaction::Restart type);
    static KIcon   restartIcon(PackageKit::Transaction::Restart type);
    static KIcon   getIcon(const QString &name);
    static KIcon   getIcon(const QString &name, const QString &defaultName);
    static QIcon   getPreloadedIcon(const QString &name);
    static QString lastCacheRefreshIconName(uint lastTime);

private:
    static void configure();

    static QHash<QString, KIcon> cache;
    static bool init;
};

#endif
