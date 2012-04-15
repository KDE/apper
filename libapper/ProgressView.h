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

#ifndef PROGRESS_VIEW_H
#define PROGRESS_VIEW_H

#include <QStandardItemModel>
#include <QTreeView>
#include <QLabel>
#include <QAbstractSlider>
#include <QStyledItemDelegate>

#include <Package>

using namespace PackageKit;

class TransactionDelegate;
class ProgressView: public QTreeView
{
    Q_OBJECT
public:
    typedef enum {
        RoleInfo = Qt::UserRole + 1,
        RoleFinished,
        RoleProgress,
        RoleId
    } PackageRoles;
    ProgressView(QWidget *parent = 0);
    ~ProgressView();

    void setSubProgress(int value);
    void handleRepo(bool handle);
    void clear();

public slots:
    void currentPackage(const PackageKit::Package &package);
    void currentRepo(const QString& repoId, const QString& description, bool enabled);

private slots:
    void followBottom(int value);
    void rangeChanged(int min, int max);

private:
    void itemFinished(QStandardItem *item);
    QList<QStandardItem *> findItems(const QString &packageId);

    QStyledItemDelegate *m_defaultDelegate;
    TransactionDelegate *m_delegate;
    QStandardItemModel  *m_model;
    QScrollBar          *m_scrollBar;
    QString              m_lastPackageId;
    bool                 m_keepScrollBarBottom;
};

#endif
