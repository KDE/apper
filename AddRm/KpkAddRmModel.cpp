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

#include "KpkAddRmModel.h"

using namespace PackageKit;

KpkAddRmModel::KpkAddRmModel(QObject *parent)
 : QAbstractTableModel(parent),
m_iconDeb("application-x-deb"), m_iconRpm("application-x-rpm"),
m_iconTgz("application-x-compressed-tar")
{

}

KpkAddRmModel::KpkAddRmModel(const QList<Package*> &packages, QObject *parent)
 : QAbstractTableModel(parent),
m_packages(packages), m_packagesChanges(packages),
m_iconDeb("application-x-deb"), m_iconRpm("application-x-rpm"),
m_iconTgz("application-x-compressed-tar"), m_iconGeneric("utilities-file-archiver")
{

}

int KpkAddRmModel::rowCount(const QModelIndex &) const
{
    return m_packages.size();
}

int KpkAddRmModel::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant KpkAddRmModel::data(const QModelIndex &index, int role) const
{
    if( index.row() > m_packages.size() )
	return QVariant();

    Package *p = m_packages.at(index.row());

    switch(role) {
        case Qt::DisplayRole:
            return p->name() + " - " + p->version() + " (" + p->arch() + ")";

        case Qt::DecorationRole:
            if ( p->data() == "Debian" )
	        return m_iconDeb;
	    else if ( p->data() == "fedora" )
	        return m_iconRpm;
	    else
	        return m_iconTgz;

        case SummaryRole:
            return p->summary();

        case InstalledRole:
	    if ( p->state() == Package::Installed )
	        return true;
            else
	        return false;

        case IdRole:
            return p->id();

	case Qt::CheckStateRole:
	    if ( index.column() == 1 ) {
	        for (int i = 0; i < m_packagesChanges.size(); ++i) {
                    if ( m_packagesChanges.at(i)->id() == package(index)->id() )
                        return Qt::Checked;
                }
	        return Qt::Unchecked;
	    }
	    else
	        return QVariant();

        default:
            return QVariant();
    }
}

bool KpkAddRmModel::setData( const QModelIndex &index, const QVariant &value, int role)
{
    if ( role == Qt::CheckStateRole && index.column() == 1 ) {
        // now we check if we need to add or remove from the list
        if ( value.toBool() ) {
            // Add
	    // its good to check if the package isn't already in
	    // this might happen for example if the user installed
	    // 
	    m_packagesChanges.append( package(index) );
	    emit dataChanged(index, index);
	    emit changed( !m_packagesChanges.isEmpty() );
	    return true;
        }
        else {
            //remove
	    for (int i = 0; i < m_packagesChanges.size(); ++i) {
                 if ( m_packagesChanges.at(i)->id() == package(index)->id() )
                     m_packagesChanges.removeAt(i);
            }
	    emit changed( !m_packagesChanges.isEmpty() );
	    emit dataChanged(index, index);
	    return true;
        }
    }
    else
        return false;
}

void KpkAddRmModel::checkChanges()
{
    emit changed( !m_packagesChanges.isEmpty() );
}

Qt::ItemFlags KpkAddRmModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

    if ( index.isValid() && index.column() == 1 )
        return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
    else
        return QAbstractTableModel::flags(index);
}

Package * KpkAddRmModel::package(const QModelIndex &index) const
{
    return m_packages.at(index.row());
}

void KpkAddRmModel::addPackage(PackageKit::Package *package)
{
//     qDebug() << package->name() << package->version() << package->arch() << package->data() << package->info() <<  package->summary() ;
    beginInsertRows(QModelIndex(), m_packages.size(), m_packages.size());
    m_packages.append(package);
    endInsertRows();
}

void KpkAddRmModel::removePackage(Package *package)
{
    beginRemoveRows(QModelIndex(), 1, 1);
    m_packages.removeAt(m_packages.indexOf(package));
    endRemoveRows();
}

void KpkAddRmModel::clearPkg()
{
    beginRemoveRows(QModelIndex(), 0, m_packages.size());
    m_packages.clear();
    endRemoveRows();
}

void KpkAddRmModel::clearPkgChanges()
{
    qDebug() << "Changes cleared";
    m_packagesChanges.clear();
    emit changed( !m_packagesChanges.isEmpty() );
}
