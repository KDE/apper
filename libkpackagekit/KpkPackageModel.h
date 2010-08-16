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
public:
    enum {
        SortRole = Qt::UserRole,
        NameRole,
        SummaryRole,
        VersionRole,
        ArchRole,
        IconRole,
        IconPathRole,
        IdRole,
        CheckStateRole,
        InstalledRole
    };

    explicit KpkPackageModel(QObject *parent = 0, QAbstractItemView *packageView = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool allSelected() const;
    QList<QSharedPointer<PackageKit::Package> > selectedPackages() const;
    QSharedPointer<PackageKit::Package> package(const QModelIndex &index) const;
    void clear();
    void resolveSelected();

    void setCheckable(bool checkable);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

public slots:
    void addPackage(const QSharedPointer<PackageKit::Package> &package,
                    bool selected = false);
    void addPackages(const QList<QSharedPointer<PackageKit::Package> > &packages,
                     bool selected = false);
    void addResolvedPackage(const QSharedPointer<PackageKit::Package> &package);
    void addSelectedPackage(const QSharedPointer<PackageKit::Package> &package);
    void rmSelectedPackage(const QSharedPointer<PackageKit::Package> &package);
    void setAllChecked(bool checked);

    void checkPackage(const QSharedPointer<PackageKit::Package> &package);
    void uncheckPackage(const QSharedPointer<PackageKit::Package> &package, bool forceEmit = false);

signals:
    void packageChecked(const QSharedPointer<PackageKit::Package> &package);
    void packageUnchecked(const QSharedPointer<PackageKit::Package> &package);

private:
    bool containsChecked(const QString &pid) const;

    QAbstractItemView *m_packageView;
    QVector<QSharedPointer<PackageKit::Package> > m_packages;
    QHash<QString, QSharedPointer<PackageKit::Package> > m_checkedPackages;
    bool m_checkable;
};

#endif
