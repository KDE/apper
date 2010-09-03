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
#include <KCategorizedSortFilterProxyModel>

#define APP_NAME    0
#define APP_SUMMARY 1
#define APP_ICON    2

using namespace PackageKit;

KpkPackageModel::KpkPackageModel(QObject *parent, QAbstractItemView *packageView)
: QAbstractItemModel(parent),
  m_packageView(packageView),
  m_checkable(false),
  m_appInstall(0)
{
}

void KpkPackageModel::addPackage(const QSharedPointer<PackageKit::Package> &package,
                                 bool selected)
{
    if (package->info() == Enum::InfoBlocked) {
        return;
    }

    if (selected) {
        checkPackage(package);
    }

#ifdef HAVE_APPINSTALL
    QList<QStringList> data;
    if (m_appInstall) {
        data = m_appInstall->values(package->name());
    }

    foreach (const QStringList &list, data) {
        InternalPackage iPackage;
        iPackage.package     = package;
        iPackage.application = 0;
        iPackage.name        = list.at(APP_NAME);
        iPackage.summary     = list.at(APP_SUMMARY);
        iPackage.icon        = list.at(APP_ICON).split('.')[0];
        kDebug() << list.at(APP_ICON);
        iPackage.version     = package->version();
        iPackage.arch        = package->arch();
        iPackage.info        = package->info();

        // check to see if the list of info has any package
        beginInsertRows(QModelIndex(), m_packages.size(), m_packages.size());
        m_packages.append(iPackage);
        endInsertRows();
    }

    if (data.isEmpty()) {
#endif //HAVE_APPINSTALL
        InternalPackage iPackage;
        iPackage.package     = package;
        iPackage.application = 1;
        iPackage.name        = package->name();
        iPackage.summary     = package->summary();
        iPackage.version     = package->version();
        iPackage.arch        = package->arch();
        iPackage.info        = package->info();

        beginInsertRows(QModelIndex(), m_packages.size(), m_packages.size());
        m_packages.append(iPackage);
        endInsertRows();
#ifdef HAVE_APPINSTALL
    }
#endif //HAVE_APPINSTALL
}

void KpkPackageModel::addPackages(const QList<QSharedPointer<PackageKit::Package> > &packages,
                                  bool selected)
{
    foreach(const QSharedPointer<PackageKit::Package> &package, packages) {
        addPackage(package, selected);
    }
}

void KpkPackageModel::addSelectedPackage(const QSharedPointer<PackageKit::Package> &package)
{
    addPackage(package, true);
}

// void KpkPackageModel::addResolvedPackage(const QSharedPointer<PackageKit::Package> &package)
// {
//     kDebug() << package->id();
//     if (m_checkedPackages.contains(package->id())) {
//         if (package->info() != m_checkedPackages[package->id()]->info()) {
//             uncheckPackage(package, true);
//         }
//     }
// }

QVariant KpkPackageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);
    if (role == Qt::DisplayRole && section == 0) {
        if (m_checkable) {
            return KpkStrings::packageQuantity(true,
                                               m_packages.size(),
                                               m_checkedPackages.size());
        }
        return KpkStrings::packageQuantity(false,
                                           m_packages.size(),
                                           0);
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
        QSharedPointer<PackageKit::Package> pkg = m_packages.at(row).package;
        return createIndex(row, column, pkg.data());
    }
    return QModelIndex();
}

QModelIndex KpkPackageModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

QVariant KpkPackageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    InternalPackage package = m_packages.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        if (property("kbd").toBool()) {
            return package.name;
        } else {
            return QVariant();
        }
    case IconRole:
        return KpkIcons::packageIcon(package.info);
    case SortRole:
        return package.name + ' ' + package.version + ' ' + package.arch;
    case Qt::CheckStateRole:
        if (!m_checkable && !property("kbd").toBool()) {
            return QVariant();
        }
    case CheckStateRole:
        if (containsChecked(package.id)) {
            return Qt::Checked;
        }
        return Qt::Unchecked;
    case IdRole:
        return package.id;
    case KExtendableItemDelegate::ShowExtensionIndicatorRole:
        return true;
    case NameRole:
        return package.name;
    case SummaryRole:
        return package.summary;
    case VersionRole:
        return package.version;
    case ArchRole:
        return package.arch;
    case IconPathRole:
        if (package.icon.isNull()) {
            QString icon = package.package->iconPath();
            package.icon = icon.isNull() ? "" : icon;
        }
        return package.icon;
    case InfoRole:
        return package.info;
    case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
        if (package.info == Enum::InfoInstalled ||
            package.info == Enum::InfoCollectionInstalled) {
            return i18n("To be Removed");
        } else {
            return i18n("To be Installed");
        }
    case KCategorizedSortFilterProxyModel::CategorySortRole:
#ifdef HAVE_APPINSTALL
        if (m_sortByApp) {
            return package.application;
        }
#endif //HAVE_APPINSTALL
        if (package.info == Enum::InfoInstalled ||
            package.info == Enum::InfoCollectionInstalled) {
            return 0;
        } else {
            return 1;
        }
    case ApplicationSortRole:
        if (package.application) {
            return 0;
        } else {
            return 1;
        }
    default:
        return QVariant();
    }

    return QVariant();
}

void KpkPackageModel::checkPackage(const QSharedPointer<PackageKit::Package> &package)
{
    kDebug() << sender();
    if (!containsChecked(package->id()) && package->info() != Enum::InfoBlocked) {
        m_checkedPackages[package->id()] = package;
        if (sender() == 0) {
            emit packageChecked(package);
        }
    }
}

void KpkPackageModel::uncheckPackage(const QSharedPointer<PackageKit::Package> &package, bool forceEmit)
{
    if (containsChecked(package->id())) {
        m_checkedPackages.remove(package->id());
        if (forceEmit || sender() == 0) {
            emit packageUnchecked(package);
        }

        for (int i = 0; i < m_packages.size(); ++i) {
            if (m_packages.at(i).package->id() == package->id()) {
                emit dataChanged(createIndex(0, 0),
                                 createIndex(i, 0));
                break;
            }

        }
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
            p = m_packages.at(index.row()).package;
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
    return m_packages.at(index.row()).package;
}

void KpkPackageModel::rmSelectedPackage(const QSharedPointer<PackageKit::Package> &package)
{
    for (int i = 0; i < m_packages.size(); i++) {
        if (m_packages.at(i).package->id() == package->id()) {
            beginRemoveRows(QModelIndex(), i, i);
            m_packages.remove(i);
            endRemoveRows();
            break;
        }
    }
}

void KpkPackageModel::clear()
{
    m_packages.clear();
    reset();
}

void KpkPackageModel::clearSelectedNotPresent()
{
    QVector<QSharedPointer<PackageKit::Package> > uncheckPackages;
    foreach (const QSharedPointer<PackageKit::Package> &package, m_checkedPackages.values()) {
        bool found = false;
        for (int i = 0; i < m_packages.size(); ++i) {
            if (m_packages.at(i).package->id() == package->id()) {
                found = true;
                break;
            }
        }
        if (!found) {
            uncheckPackages << package;
        }
    }

    for (int i = 0; i < uncheckPackages.size(); ++i) {
        uncheckPackage(uncheckPackages.at(i));
    }
}

void KpkPackageModel::resolveSelected()
{
    if (!m_checkedPackages.isEmpty()) {
//         Transaction *t;
        // TODO fix packagekit-qt
        foreach (const QSharedPointer<PackageKit::Package> &package, m_checkedPackages.values()) {
            uncheckPackage(package, true);
        }
        // TODO WHAT do I DO? yum backend doesn't reply to this...
//         t = Client::instance()->resolve(packages);
//         connect(t, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
//                 this, SLOT(addResolvedPackage(const QSharedPointer<PackageKit::Package> &)));
    }
}

void KpkPackageModel::setAllChecked(bool checked)
{
    if (checked) {
        m_checkedPackages.clear();
        for (int i = 0; i < m_packages.size(); i++) {
            checkPackage(m_packages.at(i).package);
        }
        emit dataChanged(createIndex(0, 0),
                         createIndex(m_packages.size(), 0));
    } else {
        foreach (const QSharedPointer<PackageKit::Package> &package, m_checkedPackages.values()) {
            uncheckPackage(package, true);
        }
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
    foreach (const InternalPackage &package, m_packages) {
        if (package.package->info() != Enum::InfoBlocked &&
            !containsChecked(package.package->id())) {
            return false;
        }
    }
    return true;
}

void KpkPackageModel::setCheckable(bool checkable)
{
    m_checkable = checkable;
}

#ifdef HAVE_APPINSTALL
void KpkPackageModel::setAppInstallData(QHash<QString, QStringList> *data, bool sortByApp)
{
    m_appInstall = data;
    m_sortByApp = sortByApp;
    layoutChanged();
}
#endif //HAVE_APPINSTALL
