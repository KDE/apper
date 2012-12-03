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
#ifndef APPLICATIONSORTFILTERMODEL_H
#define APPLICATIONSORTFILTERMODEL_H

#include <QSortFilterProxyModel>

#include <QDeclarativeItem>

#include <Transaction>

#include <kdemacros.h>

using namespace PackageKit;

class KDE_EXPORT ApplicationSortFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QDeclarativeItem* sourceModel READ sourceModelQML WRITE setSourceModelQML NOTIFY changed)
public:
    explicit ApplicationSortFilterModel(QObject *parent = 0);

    QDeclarativeItem* sourceModelQML() const;
    void setSourceModelQML(QDeclarativeItem *sourceModel);

public slots:
    void filterByInfo(Transaction::Info info);
    void filterApplications(bool enable);

signals:
    void changed();

private:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

    Transaction::Info m_info;
    bool m_applicationsOnly;
};

#endif // APPLICATIONSORTFILTERMODEL_H
