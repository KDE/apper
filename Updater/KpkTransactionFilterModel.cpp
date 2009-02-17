/***************************************************************************
 *   Copyright (C) 2009 by Daniel Nicoletti                                *
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

#include "KpkTransactionFilterModel.h"

#include <QDateTime>
#include <KDebug>

KpkTransactionFilterModel::KpkTransactionFilterModel(QObject *parent)
  : QSortFilterProxyModel(parent)
{
}

KpkTransactionFilterModel::~KpkTransactionFilterModel()
{
}

bool KpkTransactionFilterModel::lessThan(const QModelIndex &left,
                                         const QModelIndex &right) const
{
    QVariant leftData = sourceModel()->data(left, Qt::UserRole);
    QVariant rightData = sourceModel()->data(right, Qt::UserRole);

    if (leftData.type() == QVariant::DateTime) {
        return leftData.toDateTime() < rightData.toDateTime();
    } else {
        return QSortFilterProxyModel::lessThan(left, right);
    }
}

#include "KpkTransactionFilterModel.moc"
