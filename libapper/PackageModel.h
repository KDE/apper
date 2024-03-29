/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
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

#ifndef PACKAGE_MODEL_H
#define PACKAGE_MODEL_H

#include <QAbstractItemModel>
#include <QAbstractItemView>

#include <Transaction>
#include <Details>

class Q_DECL_EXPORT PackageModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_PROPERTY(bool checkable READ checkable WRITE setCheckable NOTIFY changed)
    Q_PROPERTY(QString selectionStateText READ selectionStateText NOTIFY changed)
public:
    enum {
        NameCol = 0,
        VersionCol,
        CurrentVersionCol,
        ArchCol,
        OriginCol,
        SizeCol,
        ActionCol
    };
    enum {
        SortRole = Qt::UserRole,
        NameRole,
        SummaryRole,
        VersionRole,
        ArchRole,
        IconRole,
        IdRole,
        CheckStateRole,
        InfoRole,
        ApplicationId,
        IsPackageRole,
        PackageName,
        InfoIconRole
    };
    typedef struct {
        QString    displayName;
        QString    pkgName;
        QString    version;
        QString    arch;
        QString    repo;
        QString    packageID;
        QString    summary;
        PackageKit::Transaction::Info info;
        QString    icon;
        QString    appId;
        QString    currentVersion;
        bool       isPackage = true;
        double     size = 0;
    } InternalPackage;

    explicit PackageModel(QObject *parent = nullptr);

    Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE bool allSelected() const;
    Q_INVOKABLE QStringList selectedPackagesToInstall() const;
    Q_INVOKABLE QStringList selectedPackagesToRemove() const;
    Q_INVOKABLE QStringList packagesWithInfo(PackageKit::Transaction::Info info) const;
    Q_INVOKABLE QStringList packageIDs() const;
    unsigned long downloadSize() const;
    Q_INVOKABLE void clear();
    /**
     * This removes all selected packages that are not in the model
     */
    Q_INVOKABLE void clearSelectedNotPresent();

    bool checkable() const;
    void setCheckable(bool checkable);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    QHash<int,QByteArray> roleNames() const override;

public Q_SLOTS:
    void addSelectedPackagesFromModel(PackageModel *model);
    void addNotSelectedPackage(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary);
    void addPackage(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary, bool selected = false);
    void addSelectedPackage(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary);
    void removePackage(const QString &packageID);

    void checkAll();
    void setAllChecked(bool checked);
    void checkPackage(const PackageModel::InternalPackage &package,
                      bool emitDataChanged = true);
    void uncheckAll();
    void uncheckPackageDefault(const QString &packageID);
    void uncheckPackage(const QString &packageID,
                        bool forceEmitUnchecked = false,
                        bool emitDataChanged = true);
    void uncheckPackageLogic(const QString &packageID,
                             bool forceEmitUnchecked = false,
                             bool emitDataChanged = true);
    bool hasChanges() const;
    int countInfo(PackageKit::Transaction::Info info) const;

    void uncheckInstalledPackages();
    void uncheckAvailablePackages();

    void finished();

    void fetchSizes();
    void fetchSizesFinished();
    void updateSize(const PackageKit::Details &details);

    void fetchCurrentVersions();
    void fetchCurrentVersionsFinished();
    void updateCurrentVersion(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary);

    void getUpdates(bool fetchCurrentVersions, bool selected);
    void toggleSelection(const QString &packageID);
    QString selectionStateText() const;

Q_SIGNALS:
    void changed(bool value);
    void packageUnchecked(const QString &packageID);

private:
    QList<InternalPackage> internalSelectedPackages() const;
    bool containsChecked(const QString &pid) const;

    bool                            m_finished = true;
    bool                            m_checkable;
    QPixmap                         m_installedEmblem;
    QVector<InternalPackage>        m_packages;
    QHash<QString, InternalPackage> m_checkedPackages;
    PackageKit::Transaction *m_getUpdatesTransaction = nullptr;
    PackageKit::Transaction *m_fetchSizesTransaction;
    PackageKit::Transaction *m_fetchInstalledVersionsTransaction;
    QHash<int, QByteArray> m_roles;
};

#endif
