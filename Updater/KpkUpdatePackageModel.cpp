/***************************************************************************
 *   Copyright (C) 2008-2010 by Daniel Nicoletti                           *
 *   dantti85-pk@yahoo.com.br                                              *
 *   Copyright (C) 2008 by Trever Fischer                                  *
 *   wm161@wm161.net                                                       *
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

#include "KpkUpdatePackageModel.h"
#include <KpkStrings.h>
#include <KIconLoader>
#include <KDebug>
#include <KpkIcons.h>
#include <KLocale>

using namespace PackageKit;

KpkUpdatePackageModel::KpkUpdatePackageModel(QObject *parent, QAbstractItemView *packageView)
: QAbstractItemModel(parent),
  m_packageView(packageView)
{
}

KpkUpdatePackageModel::KpkUpdatePackageModel(const QList<QSharedPointer<PackageKit::Package> > &packages, QObject *parent, QAbstractItemView *packageView)
: QAbstractItemModel(parent),
  m_packageView(packageView)
{
    foreach(QSharedPointer<PackageKit::Package> p, packages) {
        addPackage(p);
    }
}

QVariant KpkUpdatePackageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);
    if (role == Qt::DisplayRole) {
        switch(section) {
            case 0:
                return QVariant(i18n("Package"));
            case 1:
                return QVariant(i18n("Action"));
        }
    }
    return QVariant();
}

int KpkUpdatePackageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_packages.size();
}

QModelIndex KpkUpdatePackageModel::index(int row, int column, const QModelIndex &parent) const
{
    QSharedPointer<PackageKit::Package> pkg;
    // not grouped and parent invalid
    // means a normal package
    if (!parent.isValid() && m_packages.size() > row) {
        pkg = m_packages.at(row);
        return createIndex(row, column, pkg.data());
    } else {
        return QModelIndex();
    }
}

QModelIndex KpkUpdatePackageModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

//     PackageKit::Package *p = static_cast<PackageKit::Package *>(index.internalPointer());
//     if (p) {
//         return createIndex(m_groups.keys().indexOf(p->info()), 0);
//     } else {
        return QModelIndex();
//     }
}

//TODO: Make this not hideous.
QVariant KpkUpdatePackageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    PackageKit::Package *pkg = static_cast<PackageKit::Package*>(index.internalPointer());
    if (pkg) {
        switch (role) {
        case NameRole:
            return pkg->name() + " - " + pkg->version() + (pkg->arch().isNull() ? NULL : " (" + pkg->arch() + ')');
        case IconRole:
            return KpkIcons::packageIcon(pkg->info());;
        case SummaryRole:
            return pkg->summary();
        case Qt::CheckStateRole:
            if (containsChecked(pkg->id())) {
                return Qt::Checked;
            }
            return Qt::Unchecked;
        case IdRole:
            return pkg->id();
        default:
            return QVariant();
        }
    }
    return QVariant();
}

void KpkUpdatePackageModel::checkPackage(QSharedPointer<PackageKit::Package> package)
{
    if (package->info() != Enum::InfoBlocked && !containsChecked(package->id())) {
        m_checkedPackages[package->id()] = package;
    }
}

void KpkUpdatePackageModel::uncheckPackage(const QSharedPointer<PackageKit::Package> package)
{
    if (containsChecked(package->id())) {
        m_checkedPackages.remove(package->id());
    }
}

bool KpkUpdatePackageModel::containsChecked(const QString &pid) const
{
    if (m_checkedPackages.isEmpty()) {
        return false;
    }
    return m_checkedPackages.contains(pid);
}

bool KpkUpdatePackageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole) {
        QSharedPointer<PackageKit::Package> p = package(index);
        if (!p) {
            p = m_packages.at(index.row());
        }
        if (value.toBool()) {
            checkPackage(p);
        } else {
            uncheckPackage(p);
        }
        emit dataChanged(index, index);
        // emit this so the package icon is also updated
        emit dataChanged(index,
                            index.sibling(index.row(),
                            index.column() - 1));
        return true;
    }
    return false;
}

Qt::ItemFlags KpkUpdatePackageModel::flags(const QModelIndex &index) const
{
    if (package(index)) {
        return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | QAbstractItemModel::flags(index);
    }
    return QAbstractItemModel::flags(index);
}

int KpkUpdatePackageModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QSharedPointer<PackageKit::Package> KpkUpdatePackageModel::package(const QModelIndex &index) const
{
    return m_packages.at(index.row());
}

void KpkUpdatePackageModel::addSelectedPackage(QSharedPointer<PackageKit::Package> package)
{
    checkPackage(package);
    addPackage(package);
}

void KpkUpdatePackageModel::addPackage(QSharedPointer<PackageKit::Package> package)
{
//     kDebug() << package->name();
    if (package->info() == Enum::InfoBlocked) {
        return;
    }
    // check to see if the list of info has any package
    beginInsertRows(QModelIndex(), m_packages.size(), m_packages.size());
    m_packages.append(package);
    endInsertRows();
}

void KpkUpdatePackageModel::removePackage(QSharedPointer<PackageKit::Package> package)
{
    beginRemoveRows(QModelIndex(), m_packages.size() - 1, m_packages.size() - 1);
    m_packages.removeOne(package);
    endRemoveRows();
}

void KpkUpdatePackageModel::clear()
{
    m_packages.clear();
    reset();
}

void KpkUpdatePackageModel::setAllChecked(bool checked)
{
    if (checked) {
        m_checkedPackages.clear();
        foreach(QSharedPointer<PackageKit::Package> package, m_packages) {
            checkPackage(package);
        }
        emit dataChanged(createIndex(0, 0),
                         createIndex(m_packages.size(), 0));
    } else {
        m_checkedPackages.clear();
        emit dataChanged(createIndex(0, 0),
                         createIndex(m_packages.size(), 0));
    }
}

QList<QSharedPointer<PackageKit::Package> > KpkUpdatePackageModel::selectedPackages() const
{
    return m_checkedPackages.values();
}

QList<QSharedPointer<PackageKit::Package> > KpkUpdatePackageModel::packagesWithInfo(Enum::Info info) const
{
//     return m_groups[info];
}

bool KpkUpdatePackageModel::allSelected() const
{
    foreach(QSharedPointer<PackageKit::Package> p, m_packages) {
        if (p->info() != Enum::InfoBlocked && !containsChecked(p->id())) {
            return false;
        }
    }
    return true;
}
