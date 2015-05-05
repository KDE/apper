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
//#include <PkStrings.h>

#include <Daemon>

#include <QPainter>
#include <QStringBuilder>

#include <QIcon>
#include <QDebug>

#include <AppstreamQt/database.h>

#define ICON_SIZE 22
#define OVERLAY_SIZE 16

#include <Daemon>
#include <Details>

using namespace PackageKit;

PackageModel::PackageModel(QObject *parent)
: QAbstractItemModel(parent),
  m_finished(false),
  m_checkable(false),
  m_fetchSizesTransaction(0),
  m_fetchInstalledVersionsTransaction(0)
{
    m_roles[SortRole] = "roleSort";
    m_roles[NameRole] = "roleName";
    m_roles[SummaryRole] = "roleSummary";
    m_roles[RepoRole] = "roleRepo";
    m_roles[VersionRole] = "roleVersion";
    m_roles[ArchRole] = "roleArch";
    m_roles[IconRole] = "roleIcon";
    m_roles[IdRole] = "roleId";
    m_roles[CheckStateRole] = "roleChecked";
    m_roles[InfoRole] = "roleInfo";
    m_roles[ApplicationId] = "roleAppId";
    m_roles[IsPackageRole] = "roleIsPkg";
    m_roles[PackageName] = "rolePkgName";

    as = new Appstream::Database;
    if (!as->open()) {
        qDebug() << "Failed to open Appstream database";
        delete as;
        as = 0;
    }
    QList<Appstream::Component> apps = as->allComponents();
    qDebug() << apps.size() << "all Appstream database";
//    foreach (const Appstream::Component &app, apps) {
//        qDebug() << "name" << app.name() << app.packageNames() << app.id();
//    }
}

QHash<int, QByteArray> PackageModel::roleNames() const
{
    return m_roles;
}

void PackageModel::addSelectedPackagesFromModel(PackageModel *model)
{
    QList<InternalPackage> packages = model->internalSelectedPackages();
    foreach (const InternalPackage &package, packages) {
        addPackage(package.info, package.packageID, package.summary, true);
    }
    finished();
}

void PackageModel::addPackage(Transaction::Info info, const QString &packageID, const QString &summary, bool selected)
{
    if (m_finished) {
        qDebug() << Q_FUNC_INFO << "we are finished calling clear";
        clear();
    }

    switch(info) {
    case Transaction::InfoBlocked:
    case Transaction::InfoFinished:
    case Transaction::InfoCleanup:
        return;
    default:
        break;
    }

//    qDebug() << packageID;
    QList<Appstream::Component> applications;
    const QString &name = Transaction::packageName(packageID);
    if (!m_checkable) {
        if (as) {
            applications = as->findComponentsByPackageName(name);
        }

        bool hasDescription = false;
        foreach (const Appstream::Component &app, applications) {
            if (!app.description().isEmpty()) {
                hasDescription = true;
                break;
            }
        }

        foreach (const Appstream::Component &app, applications) {
            if (hasDescription && app.description().isEmpty()) {
                continue;
            }

            InternalPackage iPackage;
            iPackage.info = info;
            iPackage.packageID = packageID;
            iPackage.name = name;
            iPackage.version = Transaction::packageVersion(packageID);
            iPackage.arch = Transaction::packageArch(packageID);
            iPackage.repo = Transaction::packageData(packageID);
            iPackage.isPackage = false;
            if (app.name().isEmpty()) {
                iPackage.displayName = name;
            } else {
                iPackage.displayName = app.name();
            }

            if (!app.description().isEmpty()) {
                iPackage.summary = app.description();
            } else if (!app.summary().isEmpty()) {
                iPackage.summary = app.summary();
            } else {
                iPackage.summary = summary;
            }

//            iPackage.icon  = app.icon();
            iPackage.icon = app.iconUrl(QSize(64,64)).toString();
            if (iPackage.icon.isEmpty()) {
//                iPackage.icon = QIcon::fromTheme("package")..
            }
            qDebug() << app.iconUrl(QSize(64,64)) << app.icon();
            iPackage.appId = app.id();
            iPackage.size  = 0;

            if (selected) {
                checkPackage(iPackage, false);
            }
            m_packages.append(iPackage);
        }
    }

    if (applications.isEmpty()) {
        InternalPackage iPackage;
        iPackage.info = info;
        iPackage.packageID = packageID;
        iPackage.displayName = name;
        iPackage.name = name;
        iPackage.version = Transaction::packageVersion(packageID);
        iPackage.arch = Transaction::packageArch(packageID);
        iPackage.repo = Transaction::packageData(packageID);
        iPackage.summary = summary;
        iPackage.size = 0;
        iPackage.isPackage = true;

        if (selected) {
            checkPackage(iPackage, false);
        }
        m_packages.append(iPackage);

    }
}

void PackageModel::addSelectedPackage(Transaction::Info info, const QString &packageID, const QString &summary)
{
    addPackage(info, packageID, summary, true);
}

int PackageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_finished) {
        return 0;
    }
    return m_packages.size();
}

QModelIndex PackageModel::index(int row, int column, const QModelIndex &parent) const
{
//   kDebug() << parent.isValid() << m_packageCount << row << column;
    // Check to see if the index isn't out of list
    if (!parent.isValid() && m_packages.size() > row) {
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

    switch (role) {
    case IconRole:
        return package.icon;
    case SortRole:
        return QString(package.displayName % QLatin1Char(' ') % package.version % QLatin1Char(' ') % package.arch);
    case IsPackageRole:
        return package.isPackage;
    case CheckStateRole:
        return containsChecked(package.packageID) ? Qt::Checked : Qt::Unchecked;
    case IdRole:
        return package.packageID;
    case NameRole:
        return package.displayName;
    case PackageName:
        return package.name;
    case SummaryRole:
        return package.summary;
    case VersionRole:
        return package.version;
    case ArchRole:
        return package.arch;
    case RepoRole:
        return package.repo;
    case InfoRole:
        return qVariantFromValue(package.info);
    case ApplicationId:
        return package.appId;
    default:
        return QVariant();
    }

    return QVariant();
}

int PackageModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

void PackageModel::removePackage(const QString &packageID)
{
    int i = 0;
    while (i < m_packages.size()) {
        InternalPackage iPackage = m_packages[i];
        if (iPackage.packageID == packageID &&
                iPackage.info != Transaction::InfoUntrusted) {
            beginRemoveRows(QModelIndex(), i, i);
            m_packages.remove(i);
            endRemoveRows();

            // since we removed one entry we don't
            // need to increase the counter
            continue;
        }
        ++i;
    }
}

void PackageModel::clear()
{
    qDebug() << Q_FUNC_INFO;
    beginRemoveRows(QModelIndex(), 0, m_packages.size());
    m_finished = false;
    m_packages.clear();
    m_fetchSizesTransaction = 0;
    m_fetchInstalledVersionsTransaction = 0;

    if (m_getUpdatesTransaction) {
        m_getUpdatesTransaction->disconnect(this);
        m_getUpdatesTransaction->cancel();
    }
    endRemoveRows();

    emit rowCountChanged();
}

void PackageModel::clearSelectedNotPresent()
{
    foreach (const InternalPackage &package, m_checkedPackages) {
        bool notFound = true;
        foreach (const InternalPackage &iPackage, m_packages) {
            if (iPackage.packageID == package.packageID) {
                notFound = false;
                break;
            }
        }

        if (notFound) {
            // Uncheck the package If it's not in the model
            uncheckPackage(package.packageID);
        }
    }
}

bool PackageModel::checkable() const
{
    return m_checkable;
}

void PackageModel::uncheckInstalledPackages()
{
    foreach (const InternalPackage &package, m_checkedPackages) {
        if (package.info == Transaction::InfoInstalled ||
                package.info == Transaction::InfoCollectionInstalled) {
            uncheckPackage(package.packageID, true);
        }
    }
}

void PackageModel::uncheckAvailablePackages()
{
    foreach (const InternalPackage &package, m_checkedPackages) {
        if (package.info == Transaction::InfoAvailable ||
                package.info == Transaction::InfoCollectionAvailable) {
            uncheckPackage(package.packageID, true);
        }
    }
}

void PackageModel::finished()
{
    qDebug() << Q_FUNC_INFO << sender();
    Transaction *trans = qobject_cast<Transaction*>(sender());
    qDebug() << Q_FUNC_INFO << trans << sender();
    if (trans /*== m_getUpdatesTransaction*/) {
//        m_getUpdatesTransaction = 0;
        // When pkd dies this method is called twice
        // pk-qt2 bug..
//        trans->disconnect(this, SLOT(finished()));
    }

    // The whole structure is about to change
    beginInsertRows(QModelIndex(), 0, m_packages.size() - 1);
    m_finished = true;
    endInsertRows();

    emit rowCountChanged();
    emit changed(!m_checkedPackages.isEmpty());
}

void PackageModel::fetchSizes()
{
    if (m_fetchSizesTransaction) {
        return;
    }

    // get package size
    QStringList pkgs;
    foreach (const InternalPackage &p, m_packages) {
        pkgs << p.packageID;
    }
    if (!pkgs.isEmpty()) {
        m_fetchSizesTransaction = Daemon::getDetails(pkgs);
        connect(m_fetchSizesTransaction, SIGNAL(details(PackageKit::Details)),
                SLOT(updateSize(PackageKit::Details)));
        connect(m_fetchSizesTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                this, SLOT(fetchSizesFinished()));
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
    // emit this after all is changed otherwise on large models it will
    // be hell slow...
    emit dataChanged(createIndex(0, 0), createIndex(m_packages.size(), 0));
    emit changed(!m_checkedPackages.isEmpty());
}

void PackageModel::updateSize(const PackageKit::Details &details)
{
    // if size is 0 don't waste time looking for the package
    qulonglong size  = details.size();
    if (size == 0) {
        return;
    }

    for (int i = 0; i < m_packages.size(); ++i) {
        const QString &packageId = details.packageId();
        if (packageId == m_packages[i].packageID) {
            m_packages[i].size = size;
            if (m_checkable) {
                // updates the checked packages as well
                if (m_checkedPackages.contains(packageId)) {
                    // Avoid checking packages that aren't checked
                    m_checkedPackages[packageId].size = size;
                }
                break;
            }

            if (m_checkable) {
                // checkable models don't have duplicated package ids
                // so don't waste time scanning all list
                break;
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
        pkgs << Transaction::packageName(p.packageID);
    }

    if (!pkgs.isEmpty()) {
        m_fetchInstalledVersionsTransaction = Daemon::resolve(pkgs, Transaction::FilterInstalled);;
        connect(m_fetchInstalledVersionsTransaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                this, SLOT(updateCurrentVersion(PackageKit::Transaction::Info,QString,QString)));
        connect(m_fetchInstalledVersionsTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                this, SLOT(fetchCurrentVersionsFinished()));
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
    emit dataChanged(createIndex(0, 0), createIndex(m_packages.size(), 0));
    emit changed(!m_checkedPackages.isEmpty());
}

void PackageModel::updateCurrentVersion(Transaction::Info info, const QString &packageID, const QString &summary)
{
    Q_UNUSED(info)
    Q_UNUSED(summary)
    // if current version is empty don't waste time looking
    if (!Transaction::packageVersion(packageID).isEmpty()) {
        for (int i = 0; i < m_packages.size(); ++i) {
            if (Transaction::packageName(packageID) == Transaction::packageName(m_packages[i].packageID) &&
                Transaction::packageArch(packageID) == m_packages[i].arch) {
                m_packages[i].currentVersion = Transaction::packageVersion(packageID);
                if (m_checkable) {
                    // updates the checked packages as well
                    if (m_checkedPackages.contains(m_packages[i].packageID)) {
                        // Avoid checking packages that aren't checked
                        m_checkedPackages[m_packages[i].packageID].currentVersion = Transaction::packageVersion(packageID);
                    }
                    break;
                }
            }
        }
    }
}

void PackageModel::getUpdates(bool fetchCurrentVersions, bool selected)
{
    clear();
    m_getUpdatesTransaction = Daemon::getUpdates();
    if (selected) {
        connect(m_getUpdatesTransaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                this, SLOT(addSelectedPackage(PackageKit::Transaction::Info,QString,QString)));
    } else {
        connect(m_getUpdatesTransaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                this, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
    }
//    connect(m_getUpdatesTransaction, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
//            this, SLOT(errorCode(PackageKit::Transaction::Error,QString)));
//    connect(m_getUpdatesTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
//            m_busySeq, SLOT(stop()));
//    connect(m_getUpdatesTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
//            this, SLOT(finished()));
    // This is required to estimate download size
    connect(m_getUpdatesTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(fetchSizes()));
    if (fetchCurrentVersions) {
        connect(m_getUpdatesTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                this, SLOT(fetchCurrentVersions()));
    }
    connect(m_getUpdatesTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(finished()));
    // get all updates
}

void PackageModel::getInstalled()
{
    clear();
    m_getUpdatesTransaction = Daemon::getPackages(Transaction::FilterInstalled);
    connect(m_getUpdatesTransaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            this, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
//    connect(m_getUpdatesTransaction, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
//            this, SLOT(errorCode(PackageKit::Transaction::Error,QString)));
//    connect(m_getUpdatesTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
//            m_busySeq, SLOT(stop()));
//    connect(m_getUpdatesTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
//            this, SLOT(finished()));
    // This is required to estimate download size
    connect(m_getUpdatesTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(fetchSizes()));
    connect(m_getUpdatesTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(finished()));
}

void PackageModel::toggleSelection(const QString &packageID)
{
    if (containsChecked(packageID)) {
        uncheckPackage(packageID, true);
    } else {
        foreach (const InternalPackage &package, m_packages) {
            if (package.packageID == packageID) {
                checkPackage(package);
                break;
            }
        }
    }
}

bool PackageModel::hasChanges() const
{
    return !m_checkedPackages.isEmpty();
}

int PackageModel::countInfo(PackageKit::Transaction::Info info) const
{
    int ret = 0;
    foreach (const InternalPackage &package, m_packages) {
        if (package.info == info) {
            ++ret;
        }
    }
    return ret;
}

void PackageModel::checkPackage(const InternalPackage &package, bool emitDataChanged)
{
    QString pkgId = package.packageID;
    if (!containsChecked(pkgId)) {
        m_checkedPackages[pkgId] = package;

        // A checkable model does not have duplicated entries
        if (emitDataChanged || !m_checkable || !m_packages.isEmpty()) {
            // This is a slow operation so in case the user
            // is unchecking all of the packages there is
            // no need to emit data changed for every item
            for (int i = 0; i < m_packages.size(); ++i) {
                if (m_packages[i].packageID == pkgId) {
                    QModelIndex index = createIndex(i, 0);
                    emit dataChanged(index, index);
                }
            }

            // The model might not be displayed yet
            if (m_finished) {
                emit changed(!m_checkedPackages.isEmpty());
            }
        }
    }
}

void PackageModel::uncheckPackage(const QString &packageID,
                                  bool forceEmitUnchecked,
                                  bool emitDataChanged)
{
    if (containsChecked(packageID)) {
        m_checkedPackages.remove(packageID);
        if (forceEmitUnchecked || sender() == 0) {
            // The package might be removed by rmSelectedPackage
            // If we don't copy it the browse model won't uncheck there
            // right package
            emit packageUnchecked(packageID);
        }

        if (emitDataChanged || !m_checkable) {
            // This is a slow operation so in case the user
            // is unchecking all of the packages there is
            // no need to emit data changed for every item
            for (int i = 0; i < m_packages.size(); ++i) {
                if (m_packages[i].packageID == packageID) {
                    QModelIndex index = createIndex(i, 0);
                    emit dataChanged(index, index);
                }
            }

            // The model might not be displayed yet
            if (m_finished) {
                emit changed(!m_checkedPackages.isEmpty());
            }
        }
    }
}

QList<PackageModel::InternalPackage> PackageModel::internalSelectedPackages() const
{
    QList<InternalPackage> ret;
    QHash<QString, InternalPackage>::const_iterator i = m_checkedPackages.constBegin();
    while (i != m_checkedPackages.constEnd()) {
        ret << i.value();
        ++i;
    }
    return ret;
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
        foreach (const InternalPackage &package, m_packages) {
            checkPackage(package, false);
        }
        emit dataChanged(createIndex(0, 0),
                         createIndex(m_packages.size(), 0));
    } else {
        // This is a very slow operation, which in here we try to optimize
        foreach (const InternalPackage &package, m_checkedPackages) {
            uncheckPackage(package.packageID, true, false);
        }
        emit dataChanged(createIndex(0, 0),
                         createIndex(m_packages.size(), 0));
    }
    emit changed(!m_checkedPackages.isEmpty());
}

QStringList PackageModel::selectedPackagesToInstall() const
{
    QStringList list;
    foreach (const InternalPackage &package, m_checkedPackages) {
        if (package.info != Transaction::InfoInstalled &&
                package.info != Transaction::InfoCollectionInstalled) {
            // append the packages are not installed
            list << package.packageID;
        }
    }
    return list;
}

QStringList PackageModel::selectedPackagesToRemove() const
{
    QStringList list;
    foreach (const InternalPackage &package, m_checkedPackages) {
        if (package.info == Transaction::InfoInstalled ||
                package.info == Transaction::InfoCollectionInstalled) {
            // check what packages are installed and marked to be removed
            list << package.packageID;
        }
    }
    return list;
}

QStringList PackageModel::packagesWithInfo(Transaction::Info info) const
{
    QStringList list;
    foreach (const InternalPackage &package, m_packages) {
        if (package.info == info) {
            // Append to the list if the package matches the info value
            list << package.packageID;
        }
    }
    return list;
}

QStringList PackageModel::packageIDs() const
{
    QStringList list;
    foreach (const InternalPackage &package, m_packages) {
        list << package.packageID;
    }
    return list;
}

unsigned long PackageModel::downloadSize() const
{
    unsigned long size = 0;
    foreach (const InternalPackage &package, m_checkedPackages) {
        size += package.size;
    }
    return size;
}

bool PackageModel::allSelected() const
{
    foreach (const InternalPackage &package, m_packages) {
        if (!containsChecked(package.packageID)) {
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
