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


#ifndef KPKPACKAGEMODEL_H
#define KPKPACKAGEMODEL_H

#include <QAbstractTableModel>
#include <KIcon>

#include <QPackageKit>

using namespace PackageKit;

class KDE_EXPORT KpkPackageModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(bool groupPackages READ isGrouped WRITE setGrouped)

public:
    KpkPackageModel(QObject *parent = 0);
    KpkPackageModel(const QList<Package*> &packages, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool allSelected() const;
    QList<Package*> selectedPackages() const;
    QList<Package*> packagesWithState(Package::State) const;
    void removePackage(Package *package);
    Package * package(const QModelIndex &index) const;
    void clear();
    void uncheckAll();
    void checkAll();
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);
    
    bool isGrouped() const;

    enum {
        SummaryRole = Qt::UserRole,
        InstalledRole,
        IdRole
    };
    

public slots:
    void addPackage(PackageKit::Package *package);
    void setGrouped(bool g);

private:
    QVariant icon(Package::State state) const;
    QList<Package*> m_packages;
    QList<Package*> m_checkedPackages;
    QMap<Package::State, QList<Package*> > m_groups;
    KIcon m_iconGeneric;
    bool  m_grouped;
    KIcon m_iconBugFix;
    KIcon m_iconLow;
    KIcon m_iconImportant;
    KIcon m_iconEnhancement;
    KIcon m_iconSecurity;
    KIcon m_iconNormal;
    KIcon m_iconBlocked;
    KIcon m_iconDownload;
    
};

#endif
