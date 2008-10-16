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

#ifndef KPKADDRMMODEL_H
#define KPKADDRMMODEL_H

#include <QAbstractTableModel>
#include <KIcon>

#include <QPackageKit>

using namespace PackageKit;

class KpkAddRmModel : public QAbstractTableModel
{
Q_OBJECT

public:
    KpkAddRmModel(QObject *parent = 0);
    KpkAddRmModel(const QList<Package*> &packages, QObject *parent = 0);

    int rowCount(const QModelIndex &/*parent = QModelIndex()*/) const;
    int columnCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void removePackage(Package *package);
    Package * package(const QModelIndex &index) const;
    void clearPkg();
    void clearPkgChanges();

    QList<Package*> packagesChanges() { return m_packagesChanges; };

    enum {
        SummaryRole = 32,
        InstalledRole,
        IdRole
        };

public slots:
    void addPackage(PackageKit::Package *package);
    void checkChanges();

signals:
    void changed(bool state);

private:
    QList<Package*> m_packages;
    QList<Package*> m_packagesChanges;
    KIcon m_iconDeb;
    KIcon m_iconRpm;
    KIcon m_iconTgz;
    KIcon m_iconGeneric;
};

#endif
