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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "KpkPackageModel.h"
#include <KpkStrings.h>
#include <KIconLoader>
#include <KDebug>

#define UNIVERSAL_PADDING 6
#define FAV_ICON_SIZE 24

using namespace PackageKit;

KpkPackageModel::KpkPackageModel(QObject *parent, QAbstractItemView *packageView)
: QAbstractItemModel(parent),
  m_packageView(packageView),
  m_grouped(false),
  m_iconBugFix("script-error"),
  m_iconLow("security-high"),
  m_iconImportant("security-low"),
  m_iconEnhancement("ktip"),
  m_iconSecurity("emblem-important"),
  m_iconNormal("security-medium"),
  m_iconBlocked("edit-delete")
{
    KIconLoader *ic = KIconLoader::global();
    ic->addAppDir("kpackagekit");
    m_iconGeneric = KIcon("package", ic);
    m_iconDownload = KIcon("package-download", ic);
    m_iconInstalled = KIcon("package-installed", ic);
    m_iconRemove = KIcon("package-remove", ic);
}

KpkPackageModel::KpkPackageModel(const QList<Package*> &packages, QObject *parent, QAbstractItemView *packageView)
: QAbstractItemModel(parent),
  m_packageView(packageView),
  m_grouped(false),
  m_iconBugFix("script-error"),
  m_iconLow("security-high"),
  m_iconImportant("security-low"),
  m_iconEnhancement("ktip"),
  m_iconSecurity("emblem-important"),
  m_iconNormal("security-medium"),
  m_iconBlocked("edit-delete")
{
    KIconLoader *ic = KIconLoader::global();
    ic->addAppDir("kpackagekit");
    m_iconGeneric = KIcon("package", ic);
    m_iconDownload = KIcon("package-download", ic);
    m_iconRemove = KIcon("package-remove", ic);
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
                return QVariant("Action");
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
        if (parent.isValid())
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
        if (index.row() >= m_groups.size())
            return QVariant();
        //Grouped, and the parent is invalid means this is a group
        Package::State group = m_groups.keys().at(index.row());
        int count = m_groups.value(group).size();
        //TODO: Group descriptions
        switch(index.column()) {
            case 0:
                switch(role) {
                    case Qt::DisplayRole:
                        return KpkStrings::infoUpdate(group, count);
                    case Qt::DecorationRole:
                        return icon(group);
		    case GroupRole:
			return true;
		    case Qt::SizeHintRole:
			if (m_packageView) {
			    int width = m_packageView->viewport()->width() - (FAV_ICON_SIZE + 2 * UNIVERSAL_PADDING);
			    // Here we are discouting the plus sign
			    // of the tree 20 is a value get with kruller
			    // not sure but this might change... 
			    return QSize(width - 20, 50);
			}
                    default:
                        return QVariant();
                }
            case 1:
                switch(role) {
		    case Qt::CheckStateRole:
		    {
		    	// we do this here cause it's the same code for column 1 and 2		    
		        int nChecked = 0;
                        foreach(Package* p, m_groups[group]) {
			    for (int i = 0; i < m_checkedPackages.size(); ++i) {
				if ( m_checkedPackages.at(i)->id() == p->id() )
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
                        return group == Package::Installed;
		    case Qt::SizeHintRole:
			return QSize(FAV_ICON_SIZE + 2 * UNIVERSAL_PADDING, 50);
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

	// we do this here cause it's the same code for column 1 and 2
	if (role == Qt::CheckStateRole) {
	    for (int i = 0; i < m_checkedPackages.size(); ++i) {
		if ( m_checkedPackages.at(i)->id() == p->id() )
		    return Qt::Checked;
	    }
	    return Qt::Unchecked;
	}

        //TODO: Change the background color depending on a package's state
        switch (index.column()) {
            case 0: //Package name column
                switch (role) {			    
                    case Qt::DisplayRole:
                        return p->name();
                    case Qt::DecorationRole:
			if ( m_checkedPackages.contains(p) ){
			    if (p->state() == Package::Installed)
			        return m_iconRemove;
			    else
				return m_iconDownload;
			}
			else
			    return m_iconGeneric;
                    case SummaryRole:
                        return p->summary();
                    case InstalledRole:
                        return p->state() == Package::Installed;
                    case IdRole:
                        return p->id();
		    case GroupRole:
			return false;
		    case Qt::SizeHintRole:
			if (m_packageView) {
			    int width = m_packageView->viewport()->width() - (FAV_ICON_SIZE + 2 * UNIVERSAL_PADDING);
			    // Here we are discouting the plus sign
			    // of the tree 20 is a value get with kruller
			    // not sure but this might change... 
			    if (m_grouped)
				return QSize(width - 40, 50);
			    else
				//if not grouped we SHOULD not show the decorated root so
				// we have nothing to discount
				return QSize(width, 50);
			}
                    default:
                        return QVariant();
                }
            case 1: //Checkbox column
                switch(role) {
                    case InstalledRole:
                        return p->state() == Package::Installed;
		    case Qt::SizeHintRole:
			return QSize(FAV_ICON_SIZE + 2 * UNIVERSAL_PADDING, 50);
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
                    emit dataChanged(index.parent(), index.parent().sibling(index.parent().row(), index.parent().column() + 1) );
		    // emit this so the packageIcon can also change
		    emit dataChanged(index.parent(), index.parent().sibling(index.parent().row(), index.parent().column()) );
            } else {
                Package::State group = m_groups.keys().at(index.row());
                foreach(Package* package, m_groups[group]) {
		    int nChecked = 0;
		    for (int i = 0; i < m_checkedPackages.size(); ++i) {
			if ( m_checkedPackages.at(i)->id() == package->id() )
			    nChecked++;
		    }
		    if (!nChecked)
			m_checkedPackages.append(package);
                }
                emit dataChanged(this->index(0, 1, index), this->index(m_groups[group].size(), 1, index));
            }
        } else {
            if (p || !m_grouped) {
                if (p)
		    for (int i = 0; i < m_checkedPackages.size(); ++i) {
			if ( m_checkedPackages.at(i)->id() == p->id() )
			    m_checkedPackages.removeAt(i);
		    }
                else
		    for (int i = 0; i < m_checkedPackages.size(); ++i) {
			if ( m_checkedPackages.at(i)->id() == m_packages.at( index.row() )->id() )
			    m_checkedPackages.removeAt(i);
		    }
                emit dataChanged(index, index);
                if (m_grouped)
                    emit dataChanged(index.parent(), index.parent().sibling(index.parent().row(), index.parent().column() + 1) );
		    // emit this so the packageIcon can also change
		    emit dataChanged(index.parent(), index.parent().sibling(index.parent().row(), index.parent().column()) );
            } else {
                Package::State group = m_groups.keys().at(index.row());
                foreach(Package* package, m_groups[group]) {
		    for (int i = 0; i < m_checkedPackages.size(); ++i) {
			if ( m_checkedPackages.at(i)->id() == package->id() )
			    m_checkedPackages.removeAt(i);
		    }
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
        if ( package(index) ) {
	    if ( package(index)->state() == Package::Blocked )
		return QAbstractItemModel::flags(index);
	    else
		return Qt::ItemIsUserCheckable | QAbstractItemModel::flags(index);
	}
	else if ( m_groups.keys().at(index.row()) == Package::Blocked )
	    return QAbstractItemModel::flags(index);
	else
	    return Qt::ItemIsUserCheckable | Qt::ItemIsTristate | QAbstractItemModel::flags(index);;
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
    if (m_grouped && !index.parent().isValid() ) {
	return 0;
    }
    else {
	if (m_grouped)
            return packagesWithState( m_groups.keys().at( index.parent().row() ) ).at( index.row() );
        else
            return m_packages.at( index.row() );
    }
}

//TODO: Sometimes duplicate packages are added. Not sure who's fault, but add some defenses.
// THIS IS DBUS Session fault(it also happens in gnome version..)
// please don't add unessary defences dbus is the one
// who needs fixing ;)
void KpkPackageModel::addPackage(PackageKit::Package *package)
{
    // YOU MUST check if the item has a parent so you don't break
    // QT rules
    // check to see if the list of info has any package
    if (!m_grouped) {
	beginInsertRows(QModelIndex(), m_packages.size(), m_packages.size());
	m_packages.append(package);
	m_groups[package->state()].append(package);
	endInsertRows();
    }
    else if ( !m_groups.contains( package->state() ) ) {
	// insert the group item
	beginInsertRows( QModelIndex(), m_groups.size(), m_groups.size() );
	m_groups[ package->state() ].append(package);
	endInsertRows();
	// now insert the package
	beginInsertRows( createIndex( m_groups.keys().indexOf( package->state() ), 0), m_groups[ package->state() ].size(), m_groups[ package->state() ].size() );
	m_packages.append(package);
	endInsertRows();
	// the displayed data of the parent MUST be updated to show the right number of packages
	emit dataChanged( createIndex( m_groups.keys().indexOf( package->state() ), 0), createIndex( m_groups.keys().indexOf( package->state() ), 0) );
    }
    else {
	beginInsertRows( createIndex( m_groups.keys().indexOf( package->state() ), 0), m_groups[ package->state() ].size(), m_groups[ package->state() ].size() );
	m_packages.append(package);
	m_groups[package->state()].append(package);
	endInsertRows();
	// the displayed data of the parent MUST be updated to show the right number of packages
	emit dataChanged( createIndex( m_groups.keys().indexOf( package->state() ), 0), createIndex( m_groups.keys().indexOf( package->state() ), 0) );
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
}

void KpkPackageModel::checkAll()
{
    m_checkedPackages.clear();
    foreach(Package *package, m_packages) {
        if ( package->state() != Package::Blocked )
            m_checkedPackages << package;
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
    foreach(Package *package, m_packages) {
        if ( package->state() != Package::Blocked && !m_checkedPackages.contains(package) )
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
        case Package::Available:
            return m_iconDownload;
	case Package::Installed:
            return m_iconInstalled;
        default :
            return m_iconGeneric;
    }
}
