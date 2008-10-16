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

#ifndef KPK_PACKAGEMODEL_H
#define KPK_PACKAGEMODEL_H

#include <QAbstractTableModel>
#include <QAbstractItemView>
#include <KIcon>

#include <QPackageKit>

using namespace PackageKit;

class KpkUpdateModel : public QAbstractTableModel
{
Q_OBJECT

public:
    KpkUpdateModel(QObject *parent = 0, QAbstractItemView *packageView = 0);

    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    QList<Package*> selectedPackages() const;
    Package * package(const QModelIndex &index);
    int selectablePackages() const { return selectablePackagesCount; };
    void clear();

    enum {
        SummaryRole = 32,
        InstalledRole,
        IdRole
        };

public slots:
    void addPackage(PackageKit::Package *package);

signals:
    void updatesSelected(bool state);

private:
    QAbstractItemView *m_packageView;
    int selectablePackagesCount;
    QVariant icon(Package::State state) const;
    QList<Package::State> order;
    QHash< Package::State, QList<Package*> > m_packages;
    QHash< Package::State, QList<Package*> > m_selectedPackages;
    KIcon m_iconDeb; 
    KIcon m_iconRpm;
    KIcon m_iconTgz;
    KIcon m_iconGeneric;
    KIcon m_iconBugFix;
    KIcon m_iconLow;
    KIcon m_iconImportant;
    KIcon m_iconEnhancement;
    KIcon m_iconSecurity;
    KIcon m_iconNormal;
    KIcon m_iconBlocked;
};

#endif
