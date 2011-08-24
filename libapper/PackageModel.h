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
#include <KIcon>

#include <Package>

using namespace PackageKit;

class KDE_EXPORT PackageModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum {
        NameCol = 0,
        VersionCol,
        ArchCol,
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
        ApplicationFilterRole,
        PackageName,
    };
    typedef struct {
        QString    name;
        QString    version;
        QString    icon;
        QString    summary;
        QString    arch;
        QString    id;
        QString    appId;
        bool       isPackage;
        Package::Info info;
        double     size;
    } InternalPackage;

    explicit PackageModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool allSelected() const;
    QList<Package> selectedPackages() const;
    unsigned long downloadSize() const;
    void clear();
    /**
     * This removes all selected packages that are not in the model
     */
    void clearSelectedNotPresent();

    void setCheckable(bool checkable);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

public slots:
    void addPackage(const PackageKit::Package &package,
                    bool selected = false);
    void addPackages(const QList<Package> &packages,
                     bool selected = false);
    void addSelectedPackage(const PackageKit::Package &package);
    void rmSelectedPackage(const PackageModel::InternalPackage &package);

    void setAllChecked(bool checked);
    void checkPackage(const PackageModel::InternalPackage &package,
                      bool emitDataChanged = true);
    void uncheckPackage(const PackageModel::InternalPackage &package,
                        bool forceEmitUnchecked = false,
                        bool emitDataChanged = true);
    bool hasChanges() const;

    void uncheckInstalledPackages();
    void uncheckAvailablePackages();

    void finished();
    void fetchSizes();
    void fetchSizesFinished();
    void updateSize(const PackageKit::Package &package);

signals:
    void changed(bool value);
    void packageChecked(const PackageModel::InternalPackage &package);
    void packageUnchecked(const PackageModel::InternalPackage &package);

private:
    bool containsChecked(const QString &pid) const;

    int m_packageCount;
    bool                            m_checkable;
    QPixmap                         m_installedEmblem;
    QVector<InternalPackage>        m_packages;
    QHash<QString, InternalPackage> m_checkedPackages;
};

#endif
