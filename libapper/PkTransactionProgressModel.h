/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
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

#ifndef PK_TRANSACTION_PROGRESS_MODEL_H
#define PK_TRANSACTION_PROGRESS_MODEL_H

#include <QStandardItemModel>

#include <Transaction>

#include <kdemacros.h>

class KDE_EXPORT PkTransactionProgressModel: public QStandardItemModel
{
    Q_OBJECT
public:
    enum PackageRoles {
        RoleInfo = Qt::UserRole + 1,
        RolePkgName,
        RolePkgSummary,
        RoleFinished,
        RoleProgress,
        RoleId,
        RoleRepo
    };
    explicit PkTransactionProgressModel(QObject *parent = 0);
    ~PkTransactionProgressModel();

    void clear();

public slots:
    void currentPackage(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary);
    void currentRepo(const QString &repoId, const QString &description, bool enabled);
    void itemProgress(const QString &id, PackageKit::Transaction::Status status, uint percentage);

private:
    void itemFinished(QStandardItem *stdItem);
    QStandardItem* findLastItem(const QString &packageID);
};

#endif // PK_TRANSACTION_PROGRESS_MODEL_H
