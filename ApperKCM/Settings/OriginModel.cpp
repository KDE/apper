/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
 *   dantti85-pk@yahoo.com.br                                              *
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

#include <KpkStrings.h>
#include <KpkMacros.h>

#include <KDebug>
#include <KMessageBox>
#include <KLocale>

#include <Daemon>

using namespace PackageKit;

Q_DECLARE_METATYPE(Qt::CheckState)

OriginModel::OriginModel(QObject *parent) :
    QStandardItemModel(parent),
    m_finished(true)
{
    setHorizontalHeaderLabels(QStringList() << i18n("Origin of Packages"));
}


OriginModel::~OriginModel()
{
}

void OriginModel::addOriginItem(const QString &repo_id, const QString &details, bool enabled)
{
    if (m_finished) {
        // if we received a finished signal this is a new query
        QStandardItemModel::clear();
        setHorizontalHeaderLabels(QStringList() << i18n("Origin of Packages"));
        m_finished = false;
    }

    Qt::CheckState state = enabled ? Qt::Checked : Qt::Unchecked;
    QStandardItem *item = new QStandardItem(details);
    item->setCheckable(true);
    item->setCheckState(state);
    item->setData(repo_id, Qt::UserRole);
    item->setData(qVariantFromValue(state));
    appendRow(item);
}

void OriginModel::finished()
{
    m_finished = true;
}

void OriginModel::clearChanges()
{
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *repo = item(i);
        if (repo->checkState() != repo->data().value<Qt::CheckState>()) {
            repo->setCheckState(repo->data().value<Qt::CheckState>());
        }
    }
}

bool OriginModel::changed() const
{
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *repo = item(i);
        if (repo->checkState() != repo->data().value<Qt::CheckState>()) {
            return true;
        }
    }
    return false;
}

bool OriginModel::save()
{
    bool changed = false;
    for (int i = 0; i < rowCount(); ++i) {
        QStandardItem *repo = item(i);
        if (repo->checkState() != repo->data().value<Qt::CheckState>()) {
            Transaction *t = new Transaction(this);
            t->repoEnable(repo->data(Qt::UserRole).toString(),
                          static_cast<bool>(repo->checkState()));
            if (t->error()) {
                KMessageBox::sorry(0, KpkStrings::daemonError(t->error()));
                return false;
            }
            changed = true;
        }
    }

    // refresh the user cache if he or she enables/disables any of it
    if (changed) {
        SET_PROXY
        Transaction *t = new Transaction(this);
        t->refreshCache(true);
    }
    return true;
}

#include "OriginModel.moc"
