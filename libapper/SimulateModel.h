/***************************************************************************
 *   Copyright (C) 2009-2011 by Daniel Nicoletti                           *
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

#ifndef SIMULATE_MODEL_H
#define SIMULATE_MODEL_H

#include <QAbstractTableModel>
#include <KIcon>

#include <Package>

using namespace PackageKit;

class KDE_EXPORT SimulateModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SimulateModel(QObject *parent = 0,
                              QList<Package> skipPackages = QList<Package>());

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QStringList newPackages() const;
    Package::Info currentInfo() const;
    void setCurrentInfo(Package::Info currentInfo);
    int countInfo(Package::Info info);
    void clear();

public slots:
    void addPackage(const PackageKit::Package &package);

private:
    QHash<Package::Info, QList<Package> > m_packages;
    QList<Package> m_skipPackages;
    QStringList m_newPackages;
    Package::Info m_currentInfo;
};

#endif
