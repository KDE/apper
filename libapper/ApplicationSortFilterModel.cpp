/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti                                *
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
#include "ApplicationSortFilterModel.h"

#include "PackageModel.h"

//#include <KDebug>

ApplicationSortFilterModel::ApplicationSortFilterModel(QObject *parent) :
    QSortFilterProxyModel(parent),
    m_info(Transaction::InfoUnknown),
    m_applicationsOnly(false)
{
    setDynamicSortFilter(true);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setSortRole(PackageModel::SortRole);
}

PackageModel *ApplicationSortFilterModel::sourcePkgModel() const
{
    return qobject_cast<PackageModel*>(sourceModel());
}

void ApplicationSortFilterModel::setSourcePkgModel(PackageModel *packageModel)
{
    setSourceModel(packageModel);
}

Transaction::Info ApplicationSortFilterModel::infoFilter() const
{
    return m_info;
}

bool ApplicationSortFilterModel::applicationFilter() const
{
    return m_applicationsOnly;
}

void ApplicationSortFilterModel::setInfoFilter(Transaction::Info info)
{
    m_info = info;
    invalidate();
}

void ApplicationSortFilterModel::setApplicationFilter(bool enable)
{
    m_applicationsOnly = enable;
    invalidate();
}

bool ApplicationSortFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    // If we are filtering by Info check if the info matches
    if (m_info != Transaction::InfoUnknown &&
            m_info != index.data(PackageModel::InfoRole).value<Transaction::Info>()) {
        return false;
    }

    // If we are filtering by Applications check if it is an application
    if (m_applicationsOnly && index.data(PackageModel::IsPackageRole).toBool()) {
        return false;
    }

    // Return true if no filter was applied
    return true;
}

bool ApplicationSortFilterModel::lessThan(const QModelIndex &left,
                                          const QModelIndex &right) const
{
    bool leftIsPackage = left.data(PackageModel::IsPackageRole).toBool();
    bool rightIsPackage = left.data(PackageModel::IsPackageRole).toBool();

    if (leftIsPackage != rightIsPackage) {
        // If the right item is a package the left should move right
        return rightIsPackage;
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

void ApplicationSortFilterModel::sortNow()
{
    sort(0);
}

#include "moc_ApplicationSortFilterModel.cpp"
