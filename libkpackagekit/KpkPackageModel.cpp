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

#include <config.h>

#include "KpkPackageModel.h"

#include <AppInstall.h>
#include <KpkStrings.h>
#include <KpkDelegate.h>

#include <KIconLoader>
#include <KDebug>
#include <KpkIcons.h>
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

KpkPackageModel::KpkPackageModel(QObject *parent)
: QAbstractItemModel(parent),
  m_packageCount(0),
  m_checkable(false)
{
    m_installedEmblem = KpkIcons::getIcon("dialog-ok-apply", QString()).pixmap(16, 16);
}

void KpkPackageModel::addPackage(const QSharedPointer<PackageKit::Package> &package,
                                 bool selected)
{
    if (package->info() == Enum::InfoBlocked) {
        return;
    }

#ifdef HAVE_APPINSTALL
    QList<QStringList> data;
    if (!m_checkable) {
        data = AppInstall::instance()->applications(package->name());

        foreach (const QStringList &list, data) {
            InternalPackage iPackage;
            iPackage.isPackage   = false;
            iPackage.name        = list.at(AppInstall::AppName);
            if (iPackage.name.isEmpty()) {
                iPackage.name = package->name();
            }
            iPackage.summary     = list.at(AppInstall::AppSummary);
            if (iPackage.summary.isEmpty()) {
                iPackage.summary = package->summary();
            }
            iPackage.icon        = list.at(AppInstall::AppIcon);
            iPackage.version     = package->version();
            iPackage.arch        = package->arch();
            iPackage.id          = package->id();
            iPackage.appId       = list.at(AppInstall::AppId);
            iPackage.info        = package->info();

            if (selected) {
                checkPackage(iPackage, false);
            }
            m_packages.append(iPackage);
        }
    }

    if (data.isEmpty()) {
#endif //HAVE_APPINSTALL
        InternalPackage iPackage;
        iPackage.name        = package->name();
        iPackage.summary     = package->summary();
        iPackage.version     = package->version();
        iPackage.arch        = package->arch();
        iPackage.id          = package->id();
        iPackage.info        = package->info();
#ifdef HAVE_APPINSTALL
        iPackage.icon = AppInstall::instance()->genericIcon(package->name());
#else
        iPackage.icon = package->iconPath();
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

void KpkPackageModel::addPackages(const QList<QSharedPointer<PackageKit::Package> > &packages,
                                  bool selected)
{
    foreach(const QSharedPointer<PackageKit::Package> &package, packages) {
        addPackage(package, selected);
    }
    finished();
}

void KpkPackageModel::addSelectedPackage(const QSharedPointer<PackageKit::Package> &package)
{
    addPackage(package, true);
}

QVariant KpkPackageModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);   
    if (!m_checkable && role == Qt::DisplayRole) {
        // the architecture text is too long
        return QString();
    }

    if (/*m_packageCount && */role == Qt::DisplayRole) {
        if (section == 0) {
            if (m_checkable) {
                return KpkStrings::packageQuantity(true,
                                                m_packages.size(),
                                                m_checkedPackages.size());
            }
            return KpkStrings::packageQuantity(false,
                                            m_packages.size(),
                                            0);
        } else if (section == 1) {
            return i18n("Version");
        } else if (section == 2) {
            return i18n("Architecture");
        } else if (section == 3) {
            return i18n("Summary");
        }
    }
    return QVariant();
}

int KpkPackageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_packageCount;
}

QModelIndex KpkPackageModel::index(int row, int column, const QModelIndex &parent) const
{
//   kDebug() << parent.isValid() << m_packageCount << row << column;
    // Check to see if the index isn't out of list
    if (!parent.isValid() && m_packageCount > row) {
        return createIndex(row, column);
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

    if (index.column() == 0) {
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
                QPixmap pixmap = KpkIcons::getIcon(package.icon, QString()).pixmap(ICON_SIZE, ICON_SIZE);
                if (!pixmap.isNull()) {
                    QPainter painter(&icon);
                    painter.drawPixmap(QPoint(2, 0), pixmap);
                }
            }

            if (package.info == Enum::InfoInstalled ||
                package.info == Enum::InfoCollectionInstalled) {
                QPainter painter(&icon);
                QPoint startPoint;
                // bottom right corner
                startPoint = QPoint(44 - OVERLAY_SIZE, 4);
                painter.drawPixmap(startPoint, m_installedEmblem);
            } else if (m_checkable) {
                QIcon emblemIcon = KpkIcons::packageIcon(package.info);
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
        }
    } else if (index.column() == 1) {
        if (role == Qt::DisplayRole) {
            return package.version;
        }
    } else if (index.column() == 2) {
        if (role == Qt::DisplayRole) {
            return package.arch;
        }
    } else if (index.column() == 3) {
        if (role == Qt::DisplayRole) {
            return package.summary;
        }
    }

    switch (role) {
    case Qt::ForegroundRole:
        if (package.info != Enum::InfoInstalled &&
            package.info != Enum::InfoCollectionInstalled) {
            QColor foregroundColor = QApplication::palette().color(QPalette::Text);
            foregroundColor.setAlphaF(0.75);
            return QBrush(foregroundColor);
        }
        break;
    case Qt::FontRole:
        if (package.info != Enum::InfoInstalled &&
            package.info != Enum::InfoCollectionInstalled) {
            QFont font;
            font.setItalic(true);
            return font;
        }
        break;
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
        // USING 0 here seems to let things unsorted
        return package.isPackage ? 2 : 1; // Packages comes after aplications
    case ApplicationId:
        return package.appId;
    default:
        return QVariant();
    }

    return QVariant();
}

bool KpkPackageModel::setData(const QModelIndex &index, const QVariant &value, int role)
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

Qt::ItemFlags KpkPackageModel::flags(const QModelIndex &index) const
{
    if (index.column() == 0) {
        return Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | QAbstractItemModel::flags(index);
    }
    return QAbstractItemModel::flags(index);
}

int KpkPackageModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if (m_checkable) {
        // when the model is checkable we have one less column
        return 4;
    } else {
        return 5;
    }
}

void KpkPackageModel::rmSelectedPackage(const KpkPackageModel::InternalPackage &package)
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

void KpkPackageModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, m_packageCount);
    m_packageCount = 0;
    m_packages.clear();
    endRemoveRows();
}

void KpkPackageModel::clearSelectedNotPresent()
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

void KpkPackageModel::uncheckInstalledPackages()
{
    foreach (const InternalPackage &package, m_checkedPackages.values()) {
        if (package.info == Enum::InfoInstalled ||
            package.info == Enum::InfoCollectionInstalled) {
            uncheckPackage(package, true);
        }
    }
}

void KpkPackageModel::uncheckAvailablePackages()
{
    foreach (const InternalPackage &package, m_checkedPackages.values()) {
        if (package.info == Enum::InfoAvailable ||
            package.info == Enum::InfoCollectionAvailable) {
            uncheckPackage(package, true);
        }
    }
}

void KpkPackageModel::finished()
{
    // The whole structure is about to change
    beginInsertRows(QModelIndex(), 0, m_packages.size() - 1);
    m_packageCount = m_packages.size();
    endInsertRows();

    emit changed(!m_checkedPackages.isEmpty());
}

bool KpkPackageModel::hasChanges() const
{
    return !m_checkedPackages.isEmpty();
}

void KpkPackageModel::checkPackage(const InternalPackage &package, bool emitDataChanged)
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

void KpkPackageModel::uncheckPackage(const InternalPackage &package,
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

bool KpkPackageModel::containsChecked(const QString &pid) const
{
    if (m_checkedPackages.isEmpty()) {
        return false;
    }
    return m_checkedPackages.contains(pid);
}

void KpkPackageModel::setAllChecked(bool checked)
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

QList<QSharedPointer<PackageKit::Package> > KpkPackageModel::selectedPackages() const
{
    QList<QSharedPointer<PackageKit::Package> > list;
    foreach (const InternalPackage &package, m_checkedPackages.values()) {
        QSharedPointer<Package> pkg = QSharedPointer<Package>(new Package(package.id, package.info, package.summary));
        list << pkg;
    }
    return list;
}

bool KpkPackageModel::allSelected() const
{
    foreach (const InternalPackage &package, m_packages) {
        if (!containsChecked(package.id)) {
            return false;
        }
    }
    return true;
}

void KpkPackageModel::setCheckable(bool checkable)
{
    m_checkable = checkable;
}
