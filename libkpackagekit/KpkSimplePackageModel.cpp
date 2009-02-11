/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "KpkSimplePackageModel.h"

#include <KDebug>
#include <KpkIcons.h>

using namespace PackageKit;

KpkSimplePackageModel::KpkSimplePackageModel(QObject *parent)
: QStandardItemModel(parent)
{
    setSortRole(Qt::DisplayRole);
}

void KpkSimplePackageModel::addPackage(PackageKit::Package *p)
{
    QStandardItem *item = new QStandardItem;
    item->setText(p->name() + " - " + p->version() + (p->arch().isNull() ? "" : " (" + p->arch() + ")"));
    item->setIcon(KpkIcons::packageIcon(p->state()));
    item->setSelectable(false);
    item->setEditable(false);
//     item->setDragEnabled(false);
//     item->setDropEnabled(false);
//     item->setIcon((p->state() == Package::Installed) ?
//                                                        KpkIcons::getIcon("package-remove")
//                                                      : KpkIcons::getIcon("package-download"));
    item->setToolTip(p->summary());
    appendRow(item);
    //                     case NameRole:
//                         return p->name() + " - " + p->version() + (p->arch().isNull() ? "" : " (" + p->arch() + ")");
//                     case IconRole:
//                         for (int i = 0; i < m_checkedPackages.size(); ++i) {
//                             if ( m_checkedPackages.at(i)->id() == p->id() )
//                                 return (p->state() == Package::Installed) ?
//                                         KpkIcons::getIcon("package-remove")
//                                       : KpkIcons::getIcon("package-download");
//                         }
//                         return KpkIcons::packageIcon(p->state());;
//                     case SummaryRole:
//                         return p->summary();
//                     case InstalledRole:
//                         return p->state() == Package::Installed;
//                     case IdRole:
//                         return p->id();

}
