/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
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

#include "KpkPackageModel.h"
#include <KpkStrings.h>
#include <KIconLoader>
#include <KDebug>
#include <KpkIcons.h>

using namespace PackageKit;

KpkPackageModel::KpkPackageModel(QObject *parent, QAbstractItemView *packageView)
: QAbstractItemModel(parent),
m_packageView(packageView),
m_grouped(false)
{
}

KpkPackageModel::KpkPackageModel(const QList<Package*> &packages, QObject *parent, QAbstractItemView *packageView)
: QAbstractItemModel(parent),
m_packageView(packageView),
m_grouped(false)
{
    foreach(Package* p, packages) {
        addPackage(p);
    }
}

//Sort helpers
bool packageNameSortLessThan(const Package* p1, const Package* p2)
{
    return p1->name().toLower() < p2->name().toLower();
}

bool packageNameSortGreaterThan(const Package* p1, const Package* p2)
{
    return p1->name().toLower() > p2->name().toLower();
}

//A fancy function object to allow the use of the checklist
class ascendingSelectionSorter {
    public:
        ascendingSelectionSorter(QList<Package*> l) : m_list(l) {}
        bool operator()(const Package* p1, const Package*p2) {
            if (m_list.contains(const_cast<Package*>(p1)) && m_list.contains(const_cast<Package*>(p2))) {
                return false;
            }
            return m_list.contains(const_cast<Package*>(p2));
        }
        QList<Package*> m_list;
};

class descendingSelectionSorter {
    public:
        descendingSelectionSorter(QList<Package*> l) : m_list(l) {}
        bool operator()(const Package* p1, const Package* p2) {
            if (m_list.contains(const_cast<Package*>(p1)) && m_list.contains(const_cast<Package*>(p2))) {
                return false;
            }
            return m_list.contains(const_cast<Package*>(p1));
        }
        QList<Package*> m_list;
};

void KpkPackageModel::sort(int column, Qt::SortOrder order)
{
    if (column == 0) {
        if (order == Qt::DescendingOrder) {
            qSort(m_packages.begin(), m_packages.end(), packageNameSortGreaterThan);
            foreach(const Package::State &group, m_groups.keys()) {
                qSort(m_groups[group].begin(), m_groups[group].end(), packageNameSortGreaterThan);
            }
        } else {
            qSort(m_packages.begin(), m_packages.end(), packageNameSortLessThan);
            foreach(const Package::State &group, m_groups.keys()) {
                qSort(m_groups[group].begin(), m_groups[group].end(), packageNameSortLessThan);
            }
        }
    } else if (column == 1) {
        if (order == Qt::DescendingOrder){
            descendingSelectionSorter sort(m_checkedPackages);
            qSort(m_packages.begin(), m_packages.end(), sort);
            foreach(const Package::State &group, m_groups.keys()) {
                qSort(m_groups[group].begin(), m_groups[group].end(), sort);
            }
        } else {
            ascendingSelectionSorter sort(m_checkedPackages);
            qSort(m_packages.begin(), m_packages.end(), sort);
            foreach(const Package::State &group, m_groups.keys()) {
                qSort(m_groups[group].begin(), m_groups[group].end(), sort);
            }
        }
    }
    if (m_grouped) {
        for (int i = 0;i<rowCount(QModelIndex());i++) {
            QModelIndex group = index(i, 0);
            emit dataChanged(index(0, 0, group), index(0, rowCount(group), group));
        }
    } else {
        reset();
    }
}

QVariant KpkPackageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);
    if (role == Qt::DisplayRole) {
        switch(section) {
            case 0:
                return QVariant("Package");
            case 1:
                return QVariant("Action");
        }
    }
    return QVariant();
}

int KpkPackageModel::rowCount(const QModelIndex &parent) const
{
    if (m_grouped) {
        if (parent.internalPointer()) {
            return 0;
        }
        if (!parent.isValid()) {
            return m_groups.size();
        }
        Package::State group = m_groups.keys().at(parent.row());
        return m_groups.value(group).size();
    } else {
        if (parent.isValid()) {
            return 0;
        }
        return m_packages.size();
    }
}

QModelIndex KpkPackageModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!m_grouped && !parent.isValid()) {
        return createIndex(row, column, 0);
    } else if (!m_grouped) {
        return QModelIndex();
    }

    if (!hasIndex(row, column, parent)) {
        return QModelIndex();
    }

    if (parent.isValid()) {
        Package::State group = m_groups.keys().at(parent.row());
        Package* p = m_groups[group].at(row);
        return createIndex(row, column, p);
    } else {
        return createIndex(row, column, 0);
    }
}

QModelIndex KpkPackageModel::parent(const QModelIndex &index) const
{
    // If we're not grouping anything, everyone lies at the root
    if (!m_grouped) {
        return QModelIndex();
    }

    /*if (!index.isValid())
        return QModelIndex();*/

    Package* p = static_cast<Package*>(index.internalPointer());
    if (p) {
        return createIndex(m_groups.keys().indexOf(p->state()), 0);
    } else {
        return QModelIndex();
    }
}

void KpkPackageModel::setGrouped(bool g)
{
    m_grouped = g;
    reset();
}

bool KpkPackageModel::isGrouped() const
{
    return m_grouped;
}

//TODO: Make this not hideous.
QVariant KpkPackageModel::data(const QModelIndex &index, int role) const
{
    if (m_grouped && !index.parent().isValid()) {
        if (index.row() >= m_groups.size()) {
            return QVariant();
        }
        //Grouped, and the parent is invalid means this is a group
        Package::State group = m_groups.keys().at(index.row());
        int count = m_groups.value(group).size();
        //TODO: Group descriptions
        switch(index.column()) {
            case 0:
                switch(role) {
                    case NameRole:
                        return KpkStrings::infoUpdate(group, count);
                    case IconRole:
                        return KpkIcons::packageIcon(group);
                    case GroupRole:
                        return true;
                    default:
                        return QVariant();
                }
            case 1:
                switch(role) {
                    case CheckedRole:
                    {
                        // we do this here cause it's the same code for column 1 and 2
                        int nChecked = 0;
                        foreach(Package *p, m_groups[group]) {
                            if (containsChecked(p)) {
                                nChecked++;
                            }
                        }
                        if (m_groups[group].size() == nChecked)
                            return Qt::Checked;
                        else if (nChecked == 0)
                            return Qt::Unchecked;
                        else
                            return Qt::PartiallyChecked;
                    }
                    case InstalledRole:
                        return group == Package::StateInstalled;
                    default:
                        return QVariant();
                }
            default:
                return QVariant();
        }
    } else {
        //Otherwise, we're either not grouped and a root, or we are grouped and not a root.
        //Either way, we're a package.
        if (index.row() >= m_packages.size()) {
            return QVariant();
        }
        Package *p;
        if (m_grouped) {
            p = packagesWithState(m_groups.keys().at(index.parent().row())).at(index.row());
        } else {
            p = m_packages.at(index.row());
        }

        // we do this here cause it's the same code for column 1 and 2
        if (role == CheckedRole) {
            if (containsChecked(p)) {
                return Qt::Checked;
            }
            return Qt::Unchecked;
        }

        //TODO: Change the background color depending on a package's state
        switch (index.column()) {
            case 0: //Package name column
                switch (role) {
                    case NameRole:
                        return p->name() + " - " + p->version() + (p->arch().isNull() ? NULL : " (" + p->arch() + ')');
                    case IconRole:
                        if (containsChecked(p)) {
                            return (p->state() == Package::StateInstalled) ?
                                KpkIcons::getIcon("package-removed")
                                : KpkIcons::getIcon("package-download");
                        }
                        return KpkIcons::packageIcon(p->state());;
                    case SummaryRole:
                        return p->summary();
                    case InstalledRole:
                        return p->state() == Package::StateInstalled;
                    case IdRole:
                        return p->id();
                    case GroupRole:
                        return false;
                    default:
                        return QVariant();
                }
            case 1: //Checkbox column
                switch(role) {
                    case InstalledRole:
                        return p->state() == Package::StateInstalled;
                    default:
                        return QVariant();
                }
            default:
                return QVariant();
        }
    }
}

bool KpkPackageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == CheckedRole) {
        Package *p = package(index);
        if (value.toBool()) {
            if (p || !m_grouped) {
                if (p) {
                    m_checkedPackages.append(p);
                } else {
                    m_checkedPackages.append(m_packages.at(index.row()));
                }
                emit dataChanged(index, index);
                // emit this so the package icon is also updated
                emit dataChanged(index,
                                 index.sibling(index.row(),
                                 index.column() - 1));
                if (m_grouped) {
                    emit dataChanged(index.parent(),
                                     index.parent().sibling(index.parent().row(),
                                     index.parent().column() + 1));
                    // emit this so the packageIcon can also change
                    emit dataChanged(index.parent(),
                                     index.parent().sibling(index.parent().row(),
                                     index.parent().column()));
                }
            } else {
                Package::State group = m_groups.keys().at(index.row());
                foreach(Package *p, m_groups[group]) {
                    if (!containsChecked(p)) {
                        m_checkedPackages.append(p);
                    }
                }
                emit dataChanged(this->index(0, 1, index),
                                 this->index(m_groups[group].size(),
                                 1,
                                 index));
            }
        } else {
            if (p || !m_grouped) {
                if (!p) {
                    p = m_packages.at(index.row());
                }
                removeChecked(p);
                emit dataChanged(index, index);
                // emit this so the package icon is also updated
                emit dataChanged(index,
                                 index.sibling(index.row(),
                                 index.column() - 1));
                if (m_grouped) {
                    emit dataChanged(index.parent(),
                                     index.parent().sibling(index.parent().row(),
                                     index.parent().column() + 1));
                    // emit this so the packageIcon can also change
                    emit dataChanged(index.parent(),
                                     index.parent().sibling(index.parent().row(),
                                     index.parent().column()));
                }
            } else {
                Package::State group = m_groups.keys().at(index.row());
                foreach(Package *p, m_groups[group]) {
                    removeChecked(p);
                }
                emit dataChanged(this->index(0, 1, index),
                                 this->index(m_groups[group].size(),
                                 1,
                                 index));
            }
        }
        return true;
    }
    return false;
}

// This class remove package is
// in the checked list.
// removeOne/removeAll can't be used since our
// Packages are pointers
void KpkPackageModel::removeChecked(Package *package)
{
    for (int i = 0; i < m_checkedPackages.size(); ++i) {
        if (*m_checkedPackages.at(i) == package) {
            m_checkedPackages.removeAt(i);
            break;
        }
    }
}

// This class checks if the package is
// in the checked list.
// contains() can't be used since our
// Packages are pointers
bool KpkPackageModel::containsChecked(Package *package) const
{
    for (int i = 0; i < m_checkedPackages.size(); ++i) {
        if (*m_checkedPackages.at(i) == package) {
            return true;
        }
    }
    return false;
}

Qt::ItemFlags KpkPackageModel::flags(const QModelIndex &index) const
{
    if (index.column() == 1) {
        if (package(index)) {
            if (package(index)->state() == Package::StateBlocked) {
                return QAbstractItemModel::flags(index);
            } else {
                return Qt::ItemIsUserCheckable | QAbstractItemModel::flags(index);
            }
        } else if (m_groups.keys().at(index.row()) == Package::StateBlocked) {
            return QAbstractItemModel::flags(index);
        } else {
            return Qt::ItemIsUserCheckable | Qt::ItemIsTristate | QAbstractItemModel::flags(index);
        }
    }
    return QAbstractItemModel::flags(index);
}

int KpkPackageModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

Package* KpkPackageModel::package(const QModelIndex &index) const
{
    if (m_grouped && !index.parent().isValid()) {
        return 0;
    } else {
        if (m_grouped) {
            return packagesWithState(m_groups.keys().at(index.parent().row())).at(index.row());
        } else {
            return m_packages.at(index.row());
        }
    }
}

void KpkPackageModel::addSelectedPackage(PackageKit::Package *package)
{
    if (package->state() != Package::StateBlocked) {
        m_checkedPackages << package;
    }
    addPackage(package);
}

void KpkPackageModel::addPackage(PackageKit::Package *package)
{
    // check to see if the list of info has any package
    if (!m_grouped) {
        beginInsertRows(QModelIndex(), m_packages.size(), m_packages.size());
        m_packages.append(package);
        m_groups[package->state()].append(package);
        endInsertRows();
    } else if (!m_groups.contains(package->state())) {
        // insert the group item
        beginInsertRows(QModelIndex(), m_groups.size(), m_groups.size());
        m_groups[package->state()].append(package);
        endInsertRows();
        // now insert the package
        QModelIndex index(createIndex(m_groups.keys().indexOf(package->state()), 0));
        beginInsertRows(index,
                        m_groups[package->state()].size(),
                        m_groups[package->state()].size());
        m_packages.append(package);
        endInsertRows();
        // the displayed data of the parent MUST be updated to show the right number of packages
        emit dataChanged(index, index);
    }
    else {
        QModelIndex index(createIndex(m_groups.keys().indexOf(package->state()), 0));
        beginInsertRows(index,
                        m_groups[package->state()].size(),
                        m_groups[package->state()].size());
        m_packages.append(package);
        m_groups[package->state()].append(package);
        endInsertRows();
        // the displayed data of the parent MUST be updated to show the right number of packages
        emit dataChanged(index, index);
    }
}

void KpkPackageModel::removePackage(Package *package)
{
    beginRemoveRows(QModelIndex(), m_packages.size() - 1, m_packages.size() - 1);
    m_packages.removeOne(package);
    m_groups[package->state()].removeOne(package);
    endRemoveRows();
}

void KpkPackageModel::clear()
{
    m_packages.clear();
    m_groups.clear();
    reset();
}

void KpkPackageModel::uncheckAll()
{
    m_checkedPackages.clear();
    emit dataChanged(createIndex(0, 1),
                     createIndex(m_groups.size(), 1));
    if (m_grouped) {
        foreach(const Package::State &group, m_groups.keys()) {
            QModelIndex groupIndex = index(m_groups.keys().indexOf(group), 0, QModelIndex());
            emit dataChanged(index(0, 1, groupIndex),
                             index(m_groups[group].size(),
                             1,
                             groupIndex));
        }
    }
}

void KpkPackageModel::checkAll()
{
    m_checkedPackages.clear();
    foreach(Package *package, m_packages) {
        if (package->state() != Package::StateBlocked) {
            m_checkedPackages << package;
        }
    }
    emit dataChanged(createIndex(0, 1),
                     createIndex(m_groups.size(), 1));
    if (m_grouped) {
        foreach(const Package::State &group, m_groups.keys()) {
            QModelIndex groupIndex = index(m_groups.keys().indexOf(group), 0, QModelIndex());
            emit dataChanged(index(0, 1, groupIndex),
                             index(m_groups[group].size(),
                             1,
                             groupIndex));
        }
    }
}

QList<Package*> KpkPackageModel::selectedPackages() const
{
    return QList<Package*>(m_checkedPackages);
}

QList<Package*> KpkPackageModel::packagesWithState(Package::State state) const
{
    return m_groups[state];
}

bool KpkPackageModel::allSelected() const
{
    foreach(Package *p, m_packages) {
        if (p->state() != Package::StateBlocked && !containsChecked(p)) {
            return false;
        }
    }
    return true;
}
