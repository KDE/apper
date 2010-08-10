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

#include "KpkPackageModel.h"
#include <KpkStrings.h>
#include <KIconLoader>
#include <KDebug>
#include <KpkIcons.h>
#include <KLocale>
#include "KpkDelegate.h"

using namespace PackageKit;

KpkPackageModel::KpkPackageModel(QObject *parent, QAbstractItemView *packageView)
: QAbstractItemModel(parent),
  m_packageView(packageView),
  m_checkable(false)
{
}

KpkPackageModel::KpkPackageModel(const QList<QSharedPointer<PackageKit::Package> > &packages, QObject *parent, QAbstractItemView *packageView)
: QAbstractItemModel(parent),
  m_packageView(packageView)
{
    foreach(QSharedPointer<PackageKit::Package> p, packages) {
        addPackage(p);
    }
}

QVariant KpkPackageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);
    if (role == Qt::DisplayRole && section == 0) {
        if (m_checkable) {
            return KpkStrings::infoUpdate(Enum::InfoNormal,
                                          m_packages.size(),
                                          m_checkedPackages.size());
        }
        return i18np("1 Package", "%1 Packages", m_packages.size());
    }
    return QVariant();
}

int KpkPackageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_packages.size();
}

QModelIndex KpkPackageModel::index(int row, int column, const QModelIndex &parent) const
{
    // Check to see if the index isn't out of list
    if (!parent.isValid() && m_packages.size() > row) {
        QSharedPointer<PackageKit::Package> pkg = m_packages.at(row);
        return createIndex(row, column, pkg.data());
    }
    return QModelIndex();
}

QModelIndex KpkPackageModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

//TODO: Make this not hideous.
QVariant KpkPackageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    PackageKit::Package *pkg = static_cast<PackageKit::Package*>(index.internalPointer());
    if (pkg) {
        switch (role) {
        case Qt::DisplayRole:
            if (property("kbd").toBool()) {
                return pkg->name();
            } else {
                return QVariant();
            }
        case IconRole:
            return KpkIcons::packageIcon(pkg->info());;
        case SortRole:
            return pkg->name() + ' ' + pkg->version() + ' ' + pkg->arch();
        case Qt::CheckStateRole:
            if (!m_checkable) {
                return QVariant();
            }
        case CheckStateRole:
            if (containsChecked(pkg->id())) {
                return Qt::Checked;
            }
            return Qt::Unchecked;
        case IdRole:
            return pkg->id();
        case KExtendableItemDelegate::ShowExtensionIndicatorRole:
            return true;
        case NameRole:
            return pkg->name();
        case SummaryRole:
            return pkg->summary();
        case VersionRole:
            return pkg->version();
        case ArchRole:
            return pkg->arch();
        case IconPathRole:
            return pkg->iconPath();
        case InstalledRole:
            return pkg->info() == Enum::InfoInstalled;
        default:
            return QVariant();
        }
    }
    return QVariant();
}

void KpkPackageModel::checkPackage(QSharedPointer<PackageKit::Package> package)
{
    if (package->info() != Enum::InfoBlocked && !containsChecked(package->id())) {
        m_checkedPackages[package->id()] = package;
    }
}

void KpkPackageModel::uncheckPackage(const QSharedPointer<PackageKit::Package> package)
{
    if (containsChecked(package->id())) {
        m_checkedPackages.remove(package->id());
    }
}

bool KpkPackageModel::containsChecked(const QString &pid) const
{
    if (m_checkedPackages.isEmpty()) {
        return false;
    }
    return m_checkedPackages.contains(pid);
}

bool KpkPackageModel::setData(const QModelIndex &index, const QVariant &value, int role)
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

Qt::ItemFlags KpkPackageModel::flags(const QModelIndex &index) const
{
    if (package(index)) {
        return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | QAbstractItemModel::flags(index);
    }
    return QAbstractItemModel::flags(index);
}

int KpkPackageModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QSharedPointer<PackageKit::Package> KpkPackageModel::package(const QModelIndex &index) const
{
    return m_packages.at(index.row());
}

void KpkPackageModel::addSelectedPackage(QSharedPointer<PackageKit::Package> package)
{
    checkPackage(package);
    addPackage(package);
}

void KpkPackageModel::addPackage(QSharedPointer<PackageKit::Package> package)
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

void KpkPackageModel::removePackage(QSharedPointer<PackageKit::Package> package)
{
    beginRemoveRows(QModelIndex(), m_packages.size() - 1, m_packages.size() - 1);
    m_packages.remove(m_packages.indexOf(package));
    endRemoveRows();
}

void KpkPackageModel::clear()
{
    m_packages.clear();
    reset();
}

void KpkPackageModel::setAllChecked(bool checked)
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

QList<QSharedPointer<PackageKit::Package> > KpkPackageModel::selectedPackages() const
{
    return m_checkedPackages.values();
}

bool KpkPackageModel::allSelected() const
{
    foreach(QSharedPointer<PackageKit::Package> p, m_packages) {
        if (p->info() != Enum::InfoBlocked && !containsChecked(p->id())) {
            return false;
        }
    }
    return true;
}

void KpkPackageModel::setCheckable(bool checkable)
{
    m_checkable = checkable;
}
