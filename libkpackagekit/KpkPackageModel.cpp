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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "KpkPackageModel.h"
#include <KpkStrings.h>
#include <KDebug>

using namespace PackageKit;

KpkPackageModel::KpkPackageModel(QObject *parent)
    : QAbstractItemModel(parent), m_iconGeneric("utilities-file-archiver"), m_grouped(false),
      m_iconBugFix("script-error"),
      m_iconLow("security-high"),
      m_iconImportant("security-low"),
      m_iconEnhancement("ktip"),
      m_iconSecurity("emblem-important"),
      m_iconNormal("security-medium"),
      m_iconBlocked("edit-delete")
{
}

KpkPackageModel::KpkPackageModel(const QList<Package*> &packages, QObject *parent)
    : QAbstractItemModel(parent), m_iconGeneric("utilities-file-archiver"), m_grouped(false),
      m_iconBugFix("script-error"),
      m_iconLow("security-high"),
      m_iconImportant("security-low"),
      m_iconEnhancement("ktip"),
      m_iconSecurity("emblem-important"),
      m_iconNormal("security-medium"),
      m_iconBlocked("edit-delete")
{
    foreach(Package* p, packages) {
        addPackage(p);
    }
}

//Sort helpers
//Untill QPackageKit2 makes its methods const, we need to mess with it this way.
bool packageNameSortLessThan(const Package* p1, const Package* p2)
{
    return const_cast<Package*>(p1)->name().toLower() < const_cast<Package*>(p2)->name().toLower();
}

bool packageNameSortGreaterThan(const Package* p1, const Package* p2)
{
    return const_cast<Package*>(p1)->name().toLower() > const_cast<Package*>(p2)->name().toLower();
}

//A fancy function object to allow the use of the checklist
class ascendingSelectionSorter {
    public:
        ascendingSelectionSorter(QList<Package*> l) : m_list(l) {}
        bool operator()(const Package* p1, const Package*p2) {
            if (m_list.contains(const_cast<Package*>(p1)) && m_list.contains(const_cast<Package*>(p2)))
                return false;
            return m_list.contains(const_cast<Package*>(p2));
        }
        QList<Package*> m_list;
};

class descendingSelectionSorter {
    public:
        descendingSelectionSorter(QList<Package*> l) : m_list(l) {}
        bool operator()(const Package* p1, const Package* p2) {
            if (m_list.contains(const_cast<Package*>(p1)) && m_list.contains(const_cast<Package*>(p2)))
                return false;
            return m_list.contains(const_cast<Package*>(p1));
        }
        QList<Package*> m_list;
};

void KpkPackageModel::sort(int column, Qt::SortOrder order)
{
    if (column == 0) {
        if (order == Qt::DescendingOrder) {
            qSort(m_packages.begin(), m_packages.end(), packageNameSortGreaterThan);
            foreach(Package::State group, m_groups.keys()) {
                qSort(m_groups[group].begin(), m_groups[group].end(), packageNameSortGreaterThan);
            }
        } else {
            qSort(m_packages.begin(), m_packages.end(), packageNameSortLessThan);
            foreach(Package::State group, m_groups.keys()) {
                qSort(m_groups[group].begin(), m_groups[group].end(), packageNameSortLessThan);
            }
        }
    } else if (column == 1) {
        if (order == Qt::DescendingOrder){
            descendingSelectionSorter sort(m_checkedPackages);
            qSort(m_packages.begin(), m_packages.end(), sort);
            foreach(Package::State group, m_groups.keys()) {
                qSort(m_groups[group].begin(), m_groups[group].end(), sort);
            }
        } else {
            ascendingSelectionSorter sort(m_checkedPackages);
            qSort(m_packages.begin(), m_packages.end(), sort);
            foreach(Package::State group, m_groups.keys()) {
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
                return QVariant("Install/Remove");
        }
    }
    return QVariant();
}

int KpkPackageModel::rowCount(const QModelIndex &parent) const
{
    if (m_grouped) {
        if (parent.internalPointer())
            return 0;
        if (!parent.isValid())
            return m_groups.size();
        Package::State group = m_groups.keys().at(parent.row());
        return m_groups.value(group).size();
    } else {
        if (parent.internalPointer())
            return 0;
        return m_packages.size();
    }
}

QModelIndex KpkPackageModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!m_grouped && !parent.isValid())
        return createIndex(row, column, 0);
    else if (!m_grouped)
        return QModelIndex();

    if (!hasIndex(row, column, parent))
        return QModelIndex();

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
  //If we're not grouping anything, everyone lies at the root
    if (!m_grouped)
        return QModelIndex();

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
}

bool KpkPackageModel::isGrouped() const
{
    return m_grouped;
}

//TODO: Make this not hideous.
QVariant KpkPackageModel::data(const QModelIndex &index, int role) const
{
    if (m_grouped && !index.parent().isValid()) {
        if (index.row() >= m_groups.size())
            return QVariant();
        //Grouped, and the parent is invalid means this is a group
        Package::State group = m_groups.keys().at(index.row());
        int count = m_groups.value(group).size();
        //TODO: Group descriptions
        bool unchecked = false;
        bool maybechecked = false;
        switch(index.column()) {
            case 0:
                switch(role) {
                    case Qt::DisplayRole:
                        return KpkStrings::infoUpdate(group, count);
                    case Qt::DecorationRole:
                        return icon(group);
                    default:
                        return QVariant();
                }
            case 1:
                switch(role) {
                    case Qt::CheckStateRole:
                        foreach(Package* p, m_groups[group]) {
                            if (!m_checkedPackages.contains(p)) {
                                unchecked = true;
                            } else {
                                maybechecked = true;
                            }
                        }
                        if (unchecked)
                            return Qt::Unchecked;
                        if (maybechecked)
                            return Qt::PartiallyChecked;
                        return Qt::Checked;
                    default:
                        return QVariant();
                }
            default:
                return QVariant();
        }
    } else {
        //Otherwise, we're either not grouped and a root, or we are grouped and not a root.
        //Either way, we're a package.
        if (index.row() >= m_packages.size())
            return QVariant();
        Package* p;
        if (m_grouped)
            p = packagesWithState(m_groups.keys().at(index.parent().row())).at(index.row());
        else
            p = m_packages.at(index.row());
        //TODO: Change the background color depending on a package's state
        switch (index.column()) {
            case 0: //Package name column
                switch (role) {
                    case Qt::DisplayRole:
                        return p->name();
                    case Qt::DecorationRole:
                        return m_iconGeneric;
                    case SummaryRole:
                        return p->summary();
                    case InstalledRole:
                        if (p->state() == Package::Installed)
                            return true;
                        return false;
                    case IdRole:
                        return p->id();
                    default:
                        return QVariant();
                }
            case 1: //Checkbox column
                switch(role) {
                    case Qt::CheckStateRole:
                        if (m_checkedPackages.contains(p))
                            return Qt::Checked;
                        return Qt::Unchecked;
                    case InstalledRole:
                        if (p->state() == Package::Installed)
                            return true;
                        return false;
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
    if (role == Qt::CheckStateRole) {
        Package* p = package(index);
        if (value.toBool()) {
            if (p || !m_grouped) {
                if (p)
                    m_checkedPackages.append(p);
                else
                    m_checkedPackages.append(m_packages.at(index.row()));
                emit dataChanged(index, index);
                if (m_grouped)
                    emit dataChanged(index.parent(), index.parent().sibling(index.parent().row(),index.parent().column()+1));
            } else {
                Package::State group = m_groups.keys().at(index.row());
                foreach(Package* package, m_groups[group]) {
                    if (!m_checkedPackages.contains(package))
                        m_checkedPackages.append(package);
                }
                emit dataChanged(this->index(0, 1, index), this->index(m_groups[group].size(), 1, index));
            }
        } else {
            if (p || !m_grouped) {
                if (p)
                    m_checkedPackages.removeAll(p);
                else
                    m_checkedPackages.removeAll(m_packages.at(index.row()));
                emit dataChanged(index, index);
                if (m_grouped)
                    emit dataChanged(index.parent(), index.parent().sibling(index.parent().row(),index.parent().column()+1));
            } else {
                Package::State group = m_groups.keys().at(index.row());
                foreach(Package* package, m_groups[group]) {
                    m_checkedPackages.removeAll(package);
                }
                emit dataChanged(this->index(0, 1, index), this->index(m_groups[group].size(), 1, index));
            }
        }
        return true;
    }
    return false;
}

Qt::ItemFlags KpkPackageModel::flags(const QModelIndex &index) const
{
    if (index.column() == 1) {
        if (package(index))
            return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsTristate;
    }
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

int KpkPackageModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 2;
}

Package* KpkPackageModel::package(const QModelIndex &index) const
{
    Package* p = static_cast<Package*>(index.internalPointer());
    if (p)
        return p;
    return 0;
}

//TODO: Sometimes duplicate packages are added. Not sure who's fault, but add some defenses.
void KpkPackageModel::addPackage(PackageKit::Package *package)
{
    beginInsertRows(QModelIndex(), m_packages.size(), m_packages.size());
    m_packages.append(package);
    m_groups[package->state()].append(package);
    endInsertRows();
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
    m_checkedPackages.clear();
    m_groups.clear();
    reset();
}

void KpkPackageModel::uncheckAll()
{
    m_checkedPackages.clear();
}

void KpkPackageModel::checkAll()
{
    m_checkedPackages = m_packages;
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
    foreach(Package* package, m_packages) {
        if (package->state() != Package::Blocked && !m_checkedPackages.contains(package))
            return false;
    }
    return true;
}

QVariant KpkPackageModel::icon(Package::State state) const
{
    switch (state) {
        case Package::Bugfix :
            return m_iconBugFix;
        case Package::Important :
            return m_iconImportant;
        case Package::Low :
            return m_iconLow;
        case Package::Enhancement :
            return m_iconEnhancement;
        case Package::Security :
            return m_iconSecurity;
        case Package::Normal :
            return m_iconNormal;
        case Package::Blocked :
            return m_iconBlocked;
        default :
            return m_iconGeneric;
    }
}
