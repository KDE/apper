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

#include <Transaction>

using namespace PackageKit;

class KDE_EXPORT SimulateModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SimulateModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    void setSkipPackages(const QStringList &skipPackages);
    QStringList newPackages() const;
    Transaction::Info currentInfo() const;
    void setCurrentInfo(Transaction::Info currentInfo);
    int countInfo(Transaction::Info info);
    void clear();

public slots:
    void addPackage(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary);

private:
    QStringList m_skipPackages;
    QStringList m_newPackages;
    Transaction::Info m_currentInfo;
};

#endif
