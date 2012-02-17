/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
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

#include <config.h>

#include "PackageModel.h"

#include <AppInstall.h>
#include <PkStrings.h>

#include <QPainter>

#include <KIconLoader>
#include <KDebug>
#include <PkIcons.h>
#include <KLocale>
#include <KCategorizedSortFilterProxyModel>

#ifndef HAVE_APPINSTALL
#include <QSqlDatabase>
#include <QSqlQuery>
#endif

#define APP_NAME    0
#define APP_SUMMARY 1
#define APP_ICON    2
#define APP_ID      3

#define ICON_SIZE 22
#define OVERLAY_SIZE 16

using namespace PackageKit;

PackageModel::PackageModel(QObject *parent)
: QAbstractItemModel(parent),
  m_packageCount(0),
  m_checkable(false),
  m_fetchSizesTransaction(0),
  m_fetchInstalledVersionsTransaction(0)
{
    m_installedEmblem = PkIcons::getIcon("dialog-ok-apply", QString()).pixmap(16, 16);
}

void PackageModel::addPackage(const PackageKit::Package &package, bool selected)
{
    if (package.info() == Package::InfoBlocked) {
        return;
    }

#ifdef HAVE_APPINSTALL
    QList<QStringList> data;
    if (!m_checkable) {
        data = AppInstall::instance()->applications(package.name());

        foreach (const QStringList &list, data) {
            InternalPackage iPackage;
            iPackage.isPackage   = false;
            iPackage.name        = list.at(AppInstall::AppName);
            if (iPackage.name.isEmpty()) {
                iPackage.name = package.name();
            }
            iPackage.summary     = list.at(AppInstall::AppSummary);
            if (iPackage.summary.isEmpty()) {
                iPackage.summary = package.summary();
            }
            iPackage.icon        = list.at(AppInstall::AppIcon);
            iPackage.version     = package.version();
            iPackage.arch        = package.arch();
            iPackage.repo        = package.data();
            iPackage.id          = package.id();
            iPackage.appId       = list.at(AppInstall::AppId);
            iPackage.info        = package.info();
            iPackage.size        = 0;

            if (selected) {
                checkPackage(iPackage, false);
            }
            m_packages.append(iPackage);
        }
    }

    if (data.isEmpty()) {
#endif //HAVE_APPINSTALL

        InternalPackage iPackage;
        iPackage.name    = package.name();
        iPackage.summary = package.summary();
        iPackage.version = package.version();
        iPackage.arch    = package.arch();
        iPackage.repo    = package.data();
        iPackage.id      = package.id();
        iPackage.info    = package.info();
        iPackage.size    = 0;

#ifdef HAVE_APPINSTALL
        iPackage.icon = AppInstall::instance()->genericIcon(package.name());
        if (m_checkable) {
            // in case of updates model only check if it's an app
            data = AppInstall::instance()->applications(package.name());
            if (!data.isEmpty() || !package.iconPath().isEmpty()) {
                iPackage.isPackage = false;
            } else {
                iPackage.isPackage = true;
            }
        } else {
            iPackage.isPackage = true;
        }
#else
        iPackage.icon = package.iconPath();
        if (iPackage.icon.isEmpty()) {
            iPackage.isPackage = true;
        } else {
            iPackage.isPackage = false;
            QSqlDatabase db = QSqlDatabase::database();
            QSqlQuery query(db);
            query.prepare("SELECT filename FROM cache WHERE package = :name");
            query.bindValue(":name", iPackage.name);
            if (query.exec()) {
                if (query.next()) {
                    QString filename = query.value(0).toString();
                    filename.remove(QRegExp(".desktop$")).remove(QRegExp("^/.*/"));
                    iPackage.appId = filename;
                }
            }
        }
#endif //HAVE_APPINSTALL

        if (selected) {
            checkPackage(iPackage, false);
        }
        m_packages.append(iPackage);

#ifdef HAVE_APPINSTALL
    }
#endif //HAVE_APPINSTALL
}

void PackageModel::addPackages(const QList<Package> &packages,
                                  bool selected)
{
    foreach(const Package &package, packages) {
        addPackage(package, selected);
    }
    finished();
}

void PackageModel::addSelectedPackage(const PackageKit::Package &package)
{
    addPackage(package, true);
}

QVariant PackageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);

    if (/*m_packageCount && */role == Qt::DisplayRole) {
        if (section == NameCol) {
            if (m_checkable) {
                return PkStrings::packageQuantity(true,
                                                   m_packages.size(),
                                                   m_checkedPackages.size());
            }
            return i18n("Name");
        } else if (section == VersionCol) {
            return i18n("Version");
        } else if (section == CurrentVersionCol) {
            return i18n("Installed Version");
        } else if (section == ArchCol) {
            return i18n("Arch");
        } else if (section == OriginCol) {
            return i18n("Origin");
        } else if (section == SizeCol) {
            return i18n("Size");
        } else if (section == ActionCol) {
            return i18n("Action");
        }
    }
    return QVariant();
}

int PackageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_packageCount;
}

QModelIndex PackageModel::index(int row, int column, const QModelIndex &parent) const
{
//   kDebug() << parent.isValid() << m_packageCount << row << column;
    // Check to see if the index isn't out of list
    if (!parent.isValid() && m_packageCount > row) {
        return createIndex(row, column);
    }
    return QModelIndex();
}

QModelIndex PackageModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

QVariant PackageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const InternalPackage &package = m_packages[index.row()];

    if (index.column() == NameCol) {
        switch (role) {
        case Qt::CheckStateRole:
            if (!m_checkable) {
                return QVariant();
            }
        case CheckStateRole:
            if (containsChecked(package.id)) {
                return Qt::Checked;
            }
            return Qt::Unchecked;
        case ApplicationFilterRole:
            // if we are an application return 'a', if package 'p'
            return package.isPackage ? QString('p') : QString('a');
        case Qt::DisplayRole:
            return package.name;
        case Qt::DecorationRole:
        {
            QPixmap icon = QPixmap(44, ICON_SIZE);
            icon.fill(Qt::transparent);
            if (!package.icon.isEmpty()) {
                QPixmap pixmap = PkIcons::getIcon(package.icon, QString()).pixmap(ICON_SIZE, ICON_SIZE);
                if (!pixmap.isNull()) {
                    QPainter painter(&icon);
                    painter.drawPixmap(QPoint(2, 0), pixmap);
                }
            }

            if (package.info == Package::InfoInstalled ||
                package.info == Package::InfoCollectionInstalled) {
                QPainter painter(&icon);
                QPoint startPoint;
                // bottom right corner
                startPoint = QPoint(44 - OVERLAY_SIZE, 4);
                painter.drawPixmap(startPoint, m_installedEmblem);
            } else if (m_checkable) {
                QIcon emblemIcon = PkIcons::packageIcon(package.info);
                QPainter painter(&icon);
                QPoint startPoint;
                // bottom right corner
                startPoint = QPoint(44 - OVERLAY_SIZE, 4);
                painter.drawPixmap(startPoint, emblemIcon.pixmap(OVERLAY_SIZE, OVERLAY_SIZE));
            }
            return icon;
        }
        case PackageName:
            return package.id.split(';')[0];
        case Qt::ToolTipRole:
            if (m_checkable) {
                return PkStrings::info(package.info);
            } else {
                return i18n("Version: %1\nArchitecture: %2", package.version, package.arch);
            }
        }
    } else if (role == Qt::DisplayRole) {
        if (index.column() == VersionCol) {
            return package.version;
        } else if (index.column() == CurrentVersionCol) {
                return package.currentVersion;
        } else if (index.column() == ArchCol) {
            return package.arch;
        } else if (index.column() == OriginCol) {
            return package.repo;
        } else if (index.column() == SizeCol) {
            return package.size ? KGlobal::locale()->formatByteSize(package.size) : QString();
        }
    } else if (index.column() == SizeCol && role == Qt::TextAlignmentRole) {
        return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
    }

    switch (role) {
    case IconRole:
        return package.icon;
    case SortRole:
        return package.name + ' ' + package.version + ' ' + package.arch;
    case CheckStateRole:
        if (containsChecked(package.id)) {
            return Qt::Checked;
        }
        return Qt::Unchecked;
    case IdRole:
        return package.id;
    case NameRole:
        return package.name;
    case SummaryRole:
        return package.summary;
    case VersionRole:
        return package.version;
    case ArchRole:
        return package.arch;
    case OriginCol:
        return package.repo;
    case InfoRole:
        return package.info;
    case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
        if (package.info == Package::InfoInstalled ||
            package.info == Package::InfoCollectionInstalled) {
            return i18n("To be Removed");
        } else {
            return i18n("To be Installed");
        }
    case KCategorizedSortFilterProxyModel::CategorySortRole:
        // USING 0 here seems to let things unsorted
        return package.isPackage ? 1 : 0; // Packages comes after applications
    case ApplicationId:
        return package.appId;
    default:
        return QVariant();
    }

    return QVariant();
}

bool PackageModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && m_packageCount > index.row()) {
        if (value.toBool()) {
            checkPackage(m_packages.at(index.row()));
        } else {
            uncheckPackage(m_packages.at(index.row()));
        }

        emit changed(!m_checkedPackages.isEmpty());

        return true;
    }
    return false;
}

Qt::ItemFlags PackageModel::flags(const QModelIndex &index) const
{
    if (index.column() == NameCol) {
        return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | QAbstractItemModel::flags(index);
    }
    return QAbstractItemModel::flags(index);
}

int PackageModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_checkable) {
        // when the model is checkable the action column is not shown
        return ActionCol;
    } else {
        return ActionCol + 1;
    }
}

void PackageModel::rmSelectedPackage(const PackageModel::InternalPackage &package)
{
    QString pkgId = package.id;
    for (int i = 0; i < m_packages.size(); i++) {
        if (m_packages.at(i).id == pkgId) {
            beginRemoveRows(QModelIndex(), i, i);
            m_packages.remove(i);
            endRemoveRows();
            i--; // we have to decrease the pointer otherwise
                 // we will miss some packages
        }
    }
}

void PackageModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, m_packageCount);
    m_packageCount = 0;
    m_packages.clear();
    m_fetchSizesTransaction = 0;
    m_fetchInstalledVersionsTransaction = 0;
    endRemoveRows();
}

void PackageModel::clearSelectedNotPresent()
{
    QVector<InternalPackage> uncheckPackages;
    foreach (const InternalPackage &package, m_checkedPackages.values()) {
        bool found = false;
        QString pkgId = package.id;
        for (int i = 0; i < m_packages.size(); ++i) {
            if (m_packages.at(i).id == pkgId) {
                found = true;
                // TODO with applications we probably can't break
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

void PackageModel::uncheckInstalledPackages()
{
    foreach (const InternalPackage &package, m_checkedPackages.values()) {
        if (package.info == Package::InfoInstalled ||
            package.info == Package::InfoCollectionInstalled) {
            uncheckPackage(package, true);
        }
    }
}

void PackageModel::uncheckAvailablePackages()
{
    foreach (const InternalPackage &package, m_checkedPackages.values()) {
        if (package.info == Package::InfoAvailable ||
            package.info == Package::InfoCollectionAvailable) {
            uncheckPackage(package, true);
        }
    }
}

void PackageModel::finished()
{
    Transaction *trans = qobject_cast<Transaction*>(sender());
    if (trans) {
        // When pkd dies this method is called twice
        // pk-qt2 bug..
        trans->disconnect(this, SLOT(finished()));
    }

    // The whole structure is about to change
    beginInsertRows(QModelIndex(), 0, m_packages.size() - 1);
    m_packageCount = m_packages.size();
    endInsertRows();

    emit changed(!m_checkedPackages.isEmpty());
}

void PackageModel::fetchSizes()
{
    if (m_fetchSizesTransaction) {
        return;
    }

    // get package size
    QList<Package> pkgs;
    foreach (const InternalPackage &p, m_packages) {
        pkgs << Package(p.id);
    }

    if (!pkgs.isEmpty()) {
        m_fetchSizesTransaction = new Transaction(this);
        connect(m_fetchSizesTransaction, SIGNAL(package(PackageKit::Package)),
                this, SLOT(updateSize(PackageKit::Package)));
        connect(m_fetchSizesTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                this, SLOT(fetchSizesFinished()));
        m_fetchSizesTransaction->getDetails(pkgs);
    }
}

void PackageModel::fetchSizesFinished()
{
    Transaction *trans = qobject_cast<Transaction*>(sender());
    if (trans) {
        // When pkd dies this method is called twice
        // pk-qt2 bug..
        trans->disconnect(this, SLOT(fetchSizesFinished()));
    }
    // emit this after all is changed other =wise on large models it will
    // be hell slow...
    emit dataChanged(createIndex(0, SizeCol), createIndex(m_packageCount, SizeCol));
    emit changed(!m_checkedPackages.isEmpty());
}

void PackageModel::updateSize(const PackageKit::Package &package)
{
    // if size is 0 don't waste time looking for the package
    if (package.size()) {
        for (int i = 0; i < m_packages.size(); ++i) {
            if (package.id() == m_packages[i].id) {
                m_packages[i].size = package.size();
                if (m_checkable) {
                    // updates the checked packages as well
                    if (m_checkedPackages.contains(package.id())) {
                        // Avoid checking packages that aren't checked
                        m_checkedPackages[package.id()].size = package.size();
                    }
                    break;
                }
            }
        }
    }
}

void PackageModel::fetchCurrentVersions()
{
    if (m_fetchInstalledVersionsTransaction) {
        return;
    }

    // get package current version
    QStringList pkgs;
    foreach (const InternalPackage &p, m_packages) {
        pkgs << p.name;
    }

    if (!pkgs.isEmpty()) {
        m_fetchInstalledVersionsTransaction = new Transaction(this);
        connect(m_fetchInstalledVersionsTransaction, SIGNAL(package(PackageKit::Package)),
                this, SLOT(updateCurrentVersion(PackageKit::Package)));
        connect(m_fetchInstalledVersionsTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                this, SLOT(fetchCurrentVersionsFinished()));
        m_fetchInstalledVersionsTransaction->resolve(pkgs, Transaction::FilterInstalled);
    }
}

void PackageModel::fetchCurrentVersionsFinished()
{
    Transaction *trans = qobject_cast<Transaction*>(sender());
    if (trans) {
        // When pkd dies this method is called twice
        // pk-qt2 bug..
        trans->disconnect(this, SLOT(fetchCurrentVersionsFinished()));
    }
    // emit this after all is changed otherwise on large models it will
    // be hell slow...
    emit dataChanged(createIndex(0, CurrentVersionCol), createIndex(m_packageCount, CurrentVersionCol));
    emit changed(!m_checkedPackages.isEmpty());
}

void PackageModel::updateCurrentVersion(const PackageKit::Package &package)
{
    // if current version is empty don't waste time looking
    if (!package.version().isEmpty()) {
        for (int i = 0; i < m_packages.size(); ++i) {
            if (package.name() == m_packages[i].name &&
                package.arch() == m_packages[i].arch) {
                m_packages[i].currentVersion = package.version();
                if (m_checkable) {
                    // updates the checked packages as well
                    if (m_checkedPackages.contains(m_packages[i].id)) {
                        // Avoid checking packages that aren't checked
                        m_checkedPackages[m_packages[i].id].currentVersion = package.version();
                    }
                    break;
                }
            }
        }
    }
}

bool PackageModel::hasChanges() const
{
    return !m_checkedPackages.isEmpty();
}

void PackageModel::checkPackage(const InternalPackage &package, bool emitDataChanged)
{
    QString pkgId = package.id;
    if (!containsChecked(pkgId)) {
        m_checkedPackages[pkgId] = package;

        // A checkable model does not have duplicated entries
        if (emitDataChanged && m_packageCount && !m_checkable) {
            // This is a slow operation so in case the user
            // is unchecking all of the packages there is
            // no need to emit data changed for every item
            for (int i = 0; i < m_packages.size(); ++i) {
                if (m_packages.at(i).id == pkgId) {
                    QModelIndex index = createIndex(i, 0);
                    emit dataChanged(index, index);
                }
            }

            // The model might not be displayed yet
            if (m_packageCount) {
                emit changed(!m_checkedPackages.isEmpty());
            }
        }
    }
}

void PackageModel::uncheckPackage(const InternalPackage &package,
                                  bool forceEmitUnchecked,
                                  bool emitDataChanged)
{
    QString pkgId = package.id;
    if (containsChecked(pkgId)) {
        m_checkedPackages.remove(pkgId);
        if (forceEmitUnchecked || sender() == 0) {
            // The package might be removed by rmSelectedPackage
            // If we don't copy it the browse model won't uncheck there
            // right package
            InternalPackage iPackage = package;
            emit packageUnchecked(iPackage);
        }

        if (emitDataChanged && !m_checkable) {
            // This is a slow operation so in case the user
            // is unchecking all of the packages there is
            // no need to emit data changed for every item
            for (int i = 0; i < m_packages.size(); ++i) {
                if (m_packages.at(i).id == pkgId) {
                    QModelIndex index = createIndex(i, 0);
                    emit dataChanged(index, index);
                }
            }

            // The model might not be displayed yet
            if (m_packageCount) {
                emit changed(!m_checkedPackages.isEmpty());
            }
        }
    }
}

bool PackageModel::containsChecked(const QString &pid) const
{
    if (m_checkedPackages.isEmpty()) {
        return false;
    }
    return m_checkedPackages.contains(pid);
}

void PackageModel::setAllChecked(bool checked)
{
    if (checked) {
        m_checkedPackages.clear();
        for (int i = 0; i < m_packages.size(); i++) {
            InternalPackage package = m_packages.at(i);
            checkPackage(package, false);
        }
        emit dataChanged(createIndex(0, 0),
                         createIndex(m_packages.size(), 0));
    } else {
        // This is a very slow operation, which in here we try to optimize
        foreach (const InternalPackage &package, m_checkedPackages.values()) {
            uncheckPackage(package, true, false);
        }
        emit dataChanged(createIndex(0, 0),
                         createIndex(m_packages.size(), 0));
    }
    emit changed(!m_checkedPackages.isEmpty());
}

QList<Package> PackageModel::selectedPackages() const
{
    QList<Package> list;
    foreach (const InternalPackage &package, m_checkedPackages.values()) {
        list.append(Package(package.id, package.info, package.summary));
    }
    return list;
}

unsigned long PackageModel::downloadSize() const
{
    unsigned long size = 0;
    foreach (const InternalPackage &package, m_checkedPackages.values()) {
        size += package.size;
    }
    return size;
}

bool PackageModel::allSelected() const
{
    foreach (const InternalPackage &package, m_packages) {
        if (!containsChecked(package.id)) {
            return false;
        }
    }
    return true;
}

void PackageModel::setCheckable(bool checkable)
{
    m_checkable = checkable;
}

#include "PackageModel.moc"
