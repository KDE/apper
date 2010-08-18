/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
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

#include "KpkModelOrigin.h"

#include <KpkStrings.h>

#include <KDebug>
#include <KMessageBox>
#include <KLocale>

#include <QPackageKit>

using namespace PackageKit;

Q_DECLARE_METATYPE(Qt::CheckState)

KpkModelOrigin::KpkModelOrigin(QObject *parent)
 : QStandardItemModel(parent),
   m_finished(true)
{
    setHorizontalHeaderLabels(QStringList() << i18n("Origin of Packages"));
}


KpkModelOrigin::~KpkModelOrigin()
{
}

void KpkModelOrigin::addOriginItem(const QString &repo_id, const QString &details, bool enabled)
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

void KpkModelOrigin::finished()
{
    m_finished = true;
}

void KpkModelOrigin::clearChanges()
{
    for (int i = 0; i < rowCount(); i++) {
        QStandardItem *repo = item(i);
        if (repo->checkState() != repo->data().value<Qt::CheckState>()) {
            repo->setCheckState(repo->data().value<Qt::CheckState>());
        }
    }
}

bool KpkModelOrigin::changed() const
{
    for (int i = 0; i < rowCount(); i++) {
        QStandardItem *repo = item(i);
        if (repo->checkState() != repo->data().value<Qt::CheckState>()) {
            return true;
        }
    }
    return false;
}

bool KpkModelOrigin::save()
{
    for (int i = 0; i < rowCount(); i++) {
        QStandardItem *repo = item(i);
        if (repo->checkState() != repo->data().value<Qt::CheckState>()) {
            Transaction *t;
            t = Client::instance()->repoEnable(repo->data(Qt::UserRole).toString(),
                                               static_cast<bool>(repo->checkState()));
            if (t->error()) {
                KMessageBox::sorry(0, KpkStrings::daemonError(t->error()));
                return false;
            }
        }
    }
    return true;
}

#include "KpkModelOrigin.moc"
