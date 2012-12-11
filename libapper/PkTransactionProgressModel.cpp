/***************************************************************************
 *   Copyright (C) 2010-2011 by Daniel Nicoletti                           *
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

#include "PkTransactionProgressModel.h"

#include <KLocale>
#include <KDebug>

#include <PkStrings.h>

#include "PkTransaction.h"

using namespace PackageKit;

PkTransactionProgressModel::PkTransactionProgressModel(QObject *parent) :
    QStandardItemModel(parent)
{
    QHash<int, QByteArray> roles = roleNames();
    roles[RoleInfo] = "rInfo";
    roles[RolePkgName] = "rPkgName";
    roles[RolePkgSummary] = "rPkgSummary";
    roles[RoleFinished] = "rFinished";
    roles[RoleProgress] = "rProgress";
    roles[RoleId] = "rId";
    roles[RoleRepo] = "rRepo";
    setRoleNames(roles);
}

PkTransactionProgressModel::~PkTransactionProgressModel()
{
}

void PkTransactionProgressModel::currentRepo(const QString &repoId, const QString &description, bool enabled)
{
    Q_UNUSED(enabled)

    PkTransaction *transaction = qobject_cast<PkTransaction *>(sender());
    if (transaction && transaction->flags() & Transaction::TransactionFlagSimulate) {
        return;
    }

    QStandardItem *stdItem = new QStandardItem(description);
    stdItem->setData(repoId, RoleId);
    stdItem->setData(true,   RoleRepo);
    appendRow(stdItem);
}

void PkTransactionProgressModel::itemProgress(const QString &id, Transaction::Status status, uint percentage)
{
    Q_UNUSED(status)

    PkTransaction *transaction = qobject_cast<PkTransaction *>(sender());
    if (transaction && transaction->flags() & Transaction::TransactionFlagSimulate) {
        return;
    }

    QStandardItem *stdItem = findLastItem(id);
    if (stdItem && !stdItem->data(RoleFinished).toBool()) {
        // if the progress is unknown (101), make it empty
        if (percentage == 101) {
            percentage = 0;
        }
        if (stdItem->data(RoleProgress).toUInt() != percentage) {
            stdItem->setData(percentage, RoleProgress);
        }
    }
}

void PkTransactionProgressModel::clear()
{
    removeRows(0, rowCount());
}

void PkTransactionProgressModel::currentPackage(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary)
{
    PkTransaction *transaction = qobject_cast<PkTransaction *>(sender());
    if (transaction && transaction->flags() & Transaction::TransactionFlagSimulate) {
        return;
    }

    if (!packageID.isEmpty()) {
        QStandardItem *stdItem = findLastItem(packageID);
        // If there is alread some packages check to see if it has
        // finished, if the progress is 100 create a new item for the next task
        if (stdItem && !stdItem->data(RoleFinished).toBool()) {
            // if the item status (info) changed update it
            if (stdItem->data(RoleInfo).value<Transaction::Info>() != info) {
                // If the package task has finished set progress to 100
                if (info == Transaction::InfoFinished) {
                    itemFinished(stdItem);
                } else {
                    stdItem->setData(qVariantFromValue(info), RoleInfo);
                    stdItem->setText(PkStrings::infoPresent(info));
                }
            }
        } else if (info != Transaction::InfoFinished) {
            QList<QStandardItem *> items;
            // It's a new package create it and append it
            stdItem = new QStandardItem;
            stdItem->setText(PkStrings::infoPresent(info));
            stdItem->setData(Transaction::packageName(packageID), RolePkgName);
            stdItem->setData(summary, RolePkgSummary);
            stdItem->setData(qVariantFromValue(info), RoleInfo);
            stdItem->setData(0,         RoleProgress);
            stdItem->setData(false,     RoleFinished);
            stdItem->setData(packageID, RoleId);
            stdItem->setData(false,     RoleRepo);
            items << stdItem;

            stdItem = new QStandardItem(Transaction::packageName(packageID));
            stdItem->setToolTip(Transaction::packageVersion(packageID));
            items << stdItem;

            stdItem = new QStandardItem(summary);
            stdItem->setToolTip(summary);
            items << stdItem;

            appendRow(items);
        }
    }
}

void PkTransactionProgressModel::itemFinished(QStandardItem *stdItem)
{
    // Point to the item before it
    int count = stdItem->row() - 1;

    // Find the last finished item
    bool found = false;
    while (count >= 0) {
        // Put it after the finished item
        // so that running items can be kept
        // at the bottom
        if (item(count)->data(RoleFinished).toBool()) {
            // make sure it won't end in the same position
            if (count + 1 != stdItem->row()) {
                QList<QStandardItem*> items;
                items = takeRow(stdItem->row());
                insertRow(count + 1, items);
            }
            found = true;
            break;
        }
        --count;
    }

    // If it's not at the top of the list
    // and no FINISHED Item was found move it there
    if (!found && stdItem->row() != 0) {
        insertRow(0, takeRow(stdItem->row()));
    }

    Transaction::Info info = stdItem->data(RoleInfo).value<Transaction::Info>();
    stdItem->setText(PkStrings::infoPast(info));
    stdItem->setData(100,  RoleProgress);
    stdItem->setData(true, RoleFinished);
}

QStandardItem* PkTransactionProgressModel::findLastItem(const QString &packageID)
{
    int rows = rowCount() - 1;
    for (int i = rows; i >= 0; --i) {
        QStandardItem *stdItem = item(i);
        if (stdItem->data(RoleId).toString() == packageID) {
            return stdItem;
        }
    }
    return 0;
}
