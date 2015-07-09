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

#include "OriginModel.h"

#include <Daemon>
#include <PkStrings.h>

#include <KMessageBox>
#include <KLocalizedString>

#include <KDebug>

using namespace PackageKit;

OriginModel::OriginModel(QObject *parent) :
    QStandardItemModel(parent),
    m_finished(true)
{
    setHorizontalHeaderLabels(QStringList() << i18n("Origin of Packages"));
}


OriginModel::~OriginModel()
{
}

bool OriginModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && index.isValid()) {
        Transaction *transaction = Daemon::repoEnable(index.data(RepoId).toString(),
                                                      value.toBool());
        connect(transaction, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
                SLOT(errorCode(PackageKit::Transaction::Error,QString)));
        connect(transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                SLOT(setRepoFinished(PackageKit::Transaction::Exit)));
    }
    return false;
}

QVariantHash OriginModel::changes() const
{
    QVariantHash ret;
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *repo = item(i);
        bool currentState = repo->checkState();
        if (currentState != repo->data(RepoInitialState).toBool()) {
            ret[repo->data(RepoId).toString()] = currentState;
        }
    }
    return ret;
}

void OriginModel::addOriginItem(const QString &repo_id, const QString &details, bool enabled)
{
    if (m_finished) {
        // if we received a finished signal this is a new query
        removeRows(0, rowCount());
        m_finished = false;
    }

    QStandardItem *item = new QStandardItem(details);
    item->setCheckable(true);
    item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
    item->setData(repo_id, RepoId);
    item->setData(enabled, RepoInitialState);
    appendRow(item);
}

void OriginModel::finished()
{
    m_finished = true;
}

void OriginModel::errorCode(PackageKit::Transaction::Error error, const QString &details)
{
    if (error != Transaction::ErrorTransactionCancelled) {
        KMessageBox::detailedSorry(0, PkStrings::errorMessage(error), details, PkStrings::error(error), KMessageBox::Notify);
    }
}

void OriginModel::setRepoFinished(Transaction::Exit exit)
{
    if (exit == Transaction::ExitSuccess) {
        emit refreshRepoList();
    }
    sender()->deleteLater();
}

#include "OriginModel.moc"
