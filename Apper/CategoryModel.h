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

#ifndef CATEGORY_MODEL_H
#define CATEGORY_MODEL_H

#include <QObject>
#include <QStandardItemModel>
#include <QXmlStreamReader>

#include <Transaction>

#include <CategoryMatcher.h>

class CategoryModel : public QStandardItemModel
{
    Q_OBJECT
    Q_ENUMS(Roles)
public:
    typedef enum {
        SearchRole = Qt::UserRole,
        GroupRole,
        CategoryRole
    } Roles;
    explicit CategoryModel(QObject *parent = 0);
    ~CategoryModel();

    void setRoles(PackageKit::Transaction::Roles roles);
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    void setRootIndex(const QModelIndex &index);
    bool setParentIndex();
    bool hasParent() const;

Q_SIGNALS:
    void finished();

private Q_SLOTS:
    void category(const QString &parentId,
                  const QString &categoryId,
                  const QString &name,
                  const QString &summary,
                  const QString &icon);

private:
    void fillWithStandardGroups();
    void fillWithServiceGroups();
    QStandardItem* findCategory(const QString &categoryId, const QModelIndex &parent = QModelIndex()) const;
    void parseMenu(QXmlStreamReader &xml, const QString &parentIcon, QStandardItem *parent = 0);
    QList<CategoryMatcher> parseCategories(QXmlStreamReader &xml);

    PackageKit::Transaction::Roles  m_roles;
    PackageKit::Transaction::Groups m_groups;
    QModelIndex  m_rootIndex;
};

#endif
