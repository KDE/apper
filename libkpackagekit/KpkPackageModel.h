/***************************************************************************
 *   Copyright (C) 2008-2010 by Daniel Nicoletti                           *
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
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifndef KPK_PACKAGE_MODEL_H
#define KPK_PACKAGE_MODEL_H

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <KIcon>

#include <QPackageKit>

using namespace PackageKit;

class KDE_EXPORT KpkPackageModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(bool groupPackages READ isGrouped WRITE setGrouped)

public:
    explicit KpkPackageModel(QObject *parent = 0, QAbstractItemView *packageView = 0);
    explicit KpkPackageModel(const QList<Package*> &packages, QObject *parent = 0, QAbstractItemView *packageView = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool allSelected() const;
    QList<Package*> selectedPackages() const;
    QList<Package*> packagesWithInfo(Enum::Info) const;
    void removePackage(Package *package);
    Package *package(const QModelIndex &index) const;
    void clear();
    void uncheckAll();
    void checkAll();
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    bool isGrouped() const;

    enum {
        NameRole = Qt::UserRole,
        SummaryRole,
        InstalledRole,
        IconRole,
        IdRole,
        GroupRole,
        CheckedRole,
        StateRole
    };

public slots:
    void addPackage(PackageKit::Package *package);
    void addSelectedPackage(PackageKit::Package *package);
    void setGrouped(bool g);

private:
    bool containsChecked(Package *package) const;
    void checkPackage(Package *package);
    void uncheckPackage(const Package *package);
    int checkedGroupCount(Enum::Info info) const;

    QAbstractItemView *m_packageView;
    QList<Package*> m_packages;
    QHash<QString, Package*> m_checkedPackages;
    QHash<Enum::Info, int> m_checkedGroupCount;
//     QList<> m_checkedPackages;
    QMap<Enum::Info, QList<Package*> > m_groups;
    bool  m_grouped;

};

#endif
