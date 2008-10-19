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

#include "KpkUpdateModel.h"
#include <KpkStrings.h>

#define UNIVERSAL_PADDING 6
#define FAV_ICON_SIZE 24

KpkUpdateModel::KpkUpdateModel(QObject *parent, QAbstractItemView *packageView)
: QAbstractTableModel(parent),
  m_packageView(packageView),
  selectablePackagesCount(0),
  m_iconDeb("application-x-deb"),
  m_iconRpm("application-x-rpm"),
  m_iconTgz("application-x-compressed-tar"),
  m_iconGeneric("utilities-file-archiver"),
  m_iconBugFix("script-error"),
  m_iconLow("security-high"),
  m_iconImportant("security-low"),
  m_iconEnhancement("ktip"),
  m_iconSecurity("emblem-important"),
  m_iconNormal("security-medium"),
  m_iconBlocked("edit-delete")
{
}

int KpkUpdateModel::rowCount(const QModelIndex &parent) const
{
    if ( parent.isValid() ){
	// packages index has an internal pointer
	// so if the parent has an internal poiter it is
	// a package and must not have rows...
	if ( parent.internalPointer() )
	    return 0;
	else
	    return m_packages[ order.at( parent.row() ) ].count();
    }
    else
	return order.count();
}

int KpkUpdateModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

Qt::ItemFlags KpkUpdateModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

    if ( index.isValid() && index.column() == 1 )
	if ( index.parent().isValid() )
	    if ( static_cast<Package*>( index.internalPointer() )->state() != Package::Blocked )
		return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
	    else
		return QAbstractTableModel::flags(index);
        else
	    if ( order.at( index.row() ) != Package::Blocked )
		return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
	    else
		return QAbstractTableModel::flags(index);
    else
        return QAbstractTableModel::flags(index);
}

QVariant KpkUpdateModel::data(const QModelIndex &index, int role) const
{
    if ( !index.isValid() )
	return QVariant();

    switch ( index.column() ) {
	case 0:
	    switch (role) {
		case Qt::DisplayRole:
		    if ( index.parent().isValid() ){
			Package *p = m_packages[ static_cast<Package*>( index.internalPointer() )->state() ].at( index.row() );
			return p->name() + " - " + p->version() + " (" + p->arch() + ")";
		    }
		    else {
			return KpkStrings::infoUpdate( order.at( index.row() ), m_packages[ order.at( index.row() ) ].count() );
		    }

		case Qt::DecorationRole:
		    if ( index.parent().isValid() ){
			return icon( order.at( index.parent().row() ) );
		    }
		    else {
			return icon( order.at( index.row() ) );
		    }

		case SummaryRole:
		    if ( index.parent().isValid() ){
			return m_packages[ order.at( index.parent().row() ) ].at( index.row() )->summary();
		    }
		    else
		        return QVariant();

		case IdRole:
		    if ( index.parent().isValid() ){
			return m_packages[ order.at( index.parent().row() ) ].at( index.row() )->id();
		    }
		    else
			return QVariant();
		case Qt::SizeHintRole:
		    {
			int width = m_packageView->viewport()->width() - (FAV_ICON_SIZE + 2 * UNIVERSAL_PADDING);
			// Here we are discouting the plus sign
			// of the tree 20 is a value get with kruller
			// not sure but this might change... 
			if ( index.parent().isValid() ){
			    return QSize(width - 40, 50);
			}
			else {
			    return QSize(width - 20, 50);
			}
		    }
		default:
		    return QVariant();
	    }
	    break;
	case 1:
	    switch (role) {
		case Qt::CheckStateRole:
		    if ( index.parent().isValid() ){
			if ( m_selectedPackages[ static_cast<Package*>( index.internalPointer() )->state() ].contains( static_cast<Package*>( index.internalPointer() ) ) )
			    return Qt::Checked;
			else
			    return Qt::Unchecked;
		    }
		    else {
		        int p = m_packages[ order.at( index.row() ) ].count();
			int sp = m_selectedPackages[ order.at( index.row() ) ].count();
			if ( p == sp)
			    return Qt::Checked;
			else if ( sp > 0 )
			    return Qt::PartiallyChecked;
			else
			    return Qt::Unchecked;
		    }
		case Qt::SizeHintRole:
		    return QSize(FAV_ICON_SIZE + 2 * UNIVERSAL_PADDING, 50);

		default:
		    return QVariant();
	    }
	default:
	    return QVariant();
    }
}

bool KpkUpdateModel::setData( const QModelIndex &index, const QVariant &value, int role)
{
    if ( role == Qt::CheckStateRole && index.column() == 1 ) {
        // now we check if we need to add or remove from the list
        if ( value.toBool() ) {
            // Add
	    // its good to check if the package isn't already in
	    // this might happen for example if the user installed
	    // 
	    if ( index.parent().isValid() ){
		m_selectedPackages[ static_cast<Package*>( index.internalPointer() )->state() ] << static_cast<Package*>( index.internalPointer() );
		emit dataChanged( createIndex( order.indexOf( static_cast<Package*>( index.internalPointer() )->state() ), 1), createIndex( order.indexOf( static_cast<Package*>( index.internalPointer() )->state() ), 1) );
	    }
	    else {
		m_selectedPackages[ order.at( index.row() ) ] = m_packages[ order.at( index.row() ) ];
		Package *p = m_packages[ order.at( index.row() ) ].at(0);
		emit dataChanged( createIndex( 0, 1, p), createIndex( m_packages[ order.at( index.row() ) ].size(), 1, p) );
	    }
	    emit dataChanged(index, index);
	    emit updatesSelected( true );
	    return true;
        }
        else {
            //remove
	    if ( index.parent().isValid() ){
		m_selectedPackages[ static_cast<Package*>( index.internalPointer() )->state() ].removeAll( static_cast<Package*>( index.internalPointer() ) );
		emit dataChanged( createIndex( order.indexOf( static_cast<Package*>( index.internalPointer() )->state() ), 1), createIndex( order.indexOf( static_cast<Package*>( index.internalPointer() )->state() ), 1) );
	    }
	    else {
		m_selectedPackages[ order.at( index.row() ) ].clear();
		Package *p = m_packages[ order.at( index.row() ) ].at(0);
		emit dataChanged( createIndex( 0, 1, p), createIndex( m_packages[ order.at( index.row() ) ].size(), 1, p) );
	    }
	    emit dataChanged(index, index);
	    emit updatesSelected( !selectedPackages().isEmpty() );
	    return true;
        }
    }
    else
        return false;
}

QModelIndex KpkUpdateModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if ( parent.isValid() )
	return createIndex(row, column, m_packages[ order.at( parent.row() ) ].at(row) );
    else
        return createIndex(row, column, 0);
}

QModelIndex KpkUpdateModel::parent(const QModelIndex &index) const
{
    if ( !index.isValid() )
        return QModelIndex();

    if ( static_cast<Package*>( index.internalPointer() ) ) {
	return createIndex( order.indexOf( static_cast<Package*>( index.internalPointer() )->state() ), 0);
    }
    else {
	return QModelIndex();
    }
}

Package * KpkUpdateModel::package(const QModelIndex &index)
{
    if ( static_cast<Package*>( index.internalPointer() ) )
      return static_cast<Package*>( index.internalPointer() );
    else
      return 0;
}

void KpkUpdateModel::addPackage(PackageKit::Package *package)
{
    // check to see if the list of info has any package
    if ( ! order.contains( package->state() ) ) {
	beginInsertRows( QModelIndex(), order.size(), order.size() );
	order << package->state();
	endInsertRows();
    }
    beginInsertRows( createIndex( order.indexOf( package->state() ), 0), m_packages[ package->state() ].size(), m_packages[ package->state() ].size() );
    m_packages[ package->state() ] << package;
    if ( package->state() != Package::Blocked ) {
	m_selectedPackages[ package->state() ] << package;
	selectablePackagesCount++;
	emit updatesSelected( true );
    }
    endInsertRows();
    // the displayed data of the parent must be updated to show how many packages are there
    emit dataChanged( createIndex( order.indexOf( package->state() ), 0), createIndex( order.indexOf( package->state() ), 0) );
}

void KpkUpdateModel::clear()
{
    layoutAboutToBeChanged();
    m_packages.clear();
    m_selectedPackages.clear();
    order.clear();
    selectablePackagesCount = 0;
    layoutChanged();
}

QList<Package*> KpkUpdateModel::selectedPackages() const
{
    QList<Package*> returnList;
    QHash< Package::State, QList<Package*> >::const_iterator i = m_selectedPackages.constBegin();
    while (i != m_selectedPackages.constEnd()) {
	returnList << i.value();
	++i;
    }
    return returnList;
}

QVariant KpkUpdateModel::icon(Package::State state) const
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
