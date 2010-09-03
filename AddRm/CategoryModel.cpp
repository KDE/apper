/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
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

#include "CategoryModel.h"

#include <KpkStrings.h>
#include <KpkIcons.h>

#include <KCategorizedSortFilterProxyModel>
#include <KLocale>

#include <KDebug>

#include <QPackageKit>

using namespace PackageKit;

CategoryModel::CategoryModel(QObject *parent)
 : QStandardItemModel(parent)
{
    QStandardItem *item;

    item = new QStandardItem(i18n("Installed Software"));
    item->setData(Enum::RoleGetPackages, SearchRole);
    item->setData(i18n("Lists"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    item->setData(0, KCategorizedSortFilterProxyModel::CategorySortRole);
    item->setIcon(KIcon("dialog-ok"));
    appendRow(item);

    // Get the groups
    Enum::Groups groups = Client::instance()->groups();
    Enum::Roles  roles  = Client::instance()->actions();

    foreach (const Enum::Group &group, groups) {
        if (group != Enum::UnknownGroup) {
            item = new QStandardItem(KpkStrings::groups(group));
            item->setData(Enum::RoleSearchGroup, SearchRole);
            item->setData(group, GroupRole);
            item->setData(i18n("Groups"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
            item->setData(1, KCategorizedSortFilterProxyModel::CategorySortRole);
            item->setIcon(KpkIcons::groupsIcon(group));
            if (!(roles & Enum::RoleSearchGroup)) {
                item->setSelectable(false);
            }
            appendRow(item);
        }
    }
}


CategoryModel::~CategoryModel()
{
}

#include "CategoryModel.moc"
