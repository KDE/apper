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

#ifndef KPKMODELORIGIN_H
#define KPKMODELORIGIN_H

#include <QPackageKit>
#include <QAbstractListModel>
#include <QHash>

using namespace PackageKit;

class KpkModelOrigin : public QAbstractListModel
{
Q_OBJECT
public:
    KpkModelOrigin(QObject *parent = 0);
    ~KpkModelOrigin();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    int rowCount(const QModelIndex &index) const;
    bool changed() const;
    bool save();
    void clearChanges();

public slots:
    void addOriginItem(const QString &repo_id, const QString &details, bool enabled);
    void finished();

signals:
    void stateChanged();

private:
    Client *m_client;
    QList< QHash<Qt::ItemDataRole, QVariant> > m_items;
    QHash<QString, Qt::CheckState> m_actualState;
    bool m_finished;
};

#endif
