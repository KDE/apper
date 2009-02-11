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

#include "KpkModelOrigin.h"
#include <QVariant>
#include <QDebug>

KpkModelOrigin::KpkModelOrigin(QObject *parent) :
 QAbstractListModel(parent), m_finished(false)
{
    m_client = Client::instance();
}


KpkModelOrigin::~KpkModelOrigin()
{
}

QVariant KpkModelOrigin::data(const QModelIndex &index, int role) const
{
     if ( role == Qt::DisplayRole )
         return m_items.at( index.row() ).value(Qt::DisplayRole);

     if (role == Qt::CheckStateRole)
         return m_actualState[ m_items.at( index.row() ).value(Qt::UserRole).toString() ];

     return  QVariant();
}

Qt::ItemFlags KpkModelOrigin::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    else
        return Qt::ItemIsUserCheckable | QAbstractItemModel::flags(index);
}

void KpkModelOrigin::addOriginItem(const QString &repo_id, const QString &details, bool enabled)
{
    if (m_finished) {
	m_items.clear();
	m_finished = false;
	layoutChanged();
    }
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    QHash<Qt::ItemDataRole, QVariant> hash;
    hash[Qt::UserRole] = repo_id;
    hash[Qt::DisplayRole] = details;
    if (enabled) {
	hash[Qt::CheckStateRole] = Qt::Checked;
	if ( !m_actualState.contains(repo_id) )
	    m_actualState[repo_id] = Qt::Checked;
    }
    else {
	hash[Qt::CheckStateRole] = Qt::Unchecked;
	if ( !m_actualState.contains(repo_id) )
	    m_actualState[repo_id] = Qt::Unchecked;
    }
    m_items << hash;
    emit stateChanged();
    endInsertRows();
}

void KpkModelOrigin::finished()
{
    m_finished = true;
}

void KpkModelOrigin::clearChanges()
{
    for (int i = 0; i < m_items.size(); i++)
	m_actualState[ m_items.at(i).value(Qt::UserRole).toString() ] = (Qt::CheckState) m_items.at(i).value(Qt::CheckStateRole).toInt();
    emit layoutChanged();
}

bool KpkModelOrigin::changed() const
{
    for (int i = 0; i < m_items.size(); i++) {
        if ( m_items.at(i).value(Qt::CheckStateRole) != m_actualState.value( m_items.at(i).value(Qt::UserRole).toString() ) ) {
            return true;
        }
    }
    return false;
}

bool KpkModelOrigin::save()
{
    QString repoId;
    for (int i = 0; i < m_items.size(); i++) {
        if ( m_items.at(i).value(Qt::CheckStateRole) != m_actualState.value( m_items.at(i).value(Qt::UserRole).toString() ) ) {
            repoId = m_items.at(i).value(Qt::UserRole).toString();
            if ( m_client->repoEnable(repoId, (bool) m_actualState.value( repoId ) ) == 0 )
                return false;
        }
    }
    return true;
}

bool KpkModelOrigin::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::CheckStateRole ) {
	m_actualState[ m_items.at( index.row() ).value(Qt::UserRole).toString() ] = (Qt::CheckState) value.toUInt();
	emit stateChanged();
        return true;
    }
    else
        return false;
}

int KpkModelOrigin::rowCount(const QModelIndex &index) const {
    Q_UNUSED(index);
    return m_items.size();
}
