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

#ifndef ORIGIN_MODEL_H
#define ORIGIN_MODEL_H

#include <QStandardItemModel>

#include <Transaction>

class OriginModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum {
        RepoId = Qt::UserRole,
        RepoInitialState
    } RepoRole;
    explicit OriginModel(QObject *parent = 0);
    ~OriginModel();

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    QVariantHash changes() const;

signals:
    void refreshRepoList();

public slots:
    void addOriginItem(const QString &repo_id, const QString &details, bool enabled);
    void finished();

private slots:
    void errorCode(PackageKit::Transaction::Error error, const QString &details);
    void setRepoFinished(PackageKit::Transaction::Exit exit);

private:
    bool m_finished;
};

#endif
