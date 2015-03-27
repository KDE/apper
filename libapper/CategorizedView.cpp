/***************************************************************************
 *   Copyright (C) 2009 by Rafael FernÃ¡ndez LÃ³pez <ereslibre@kde.org>    *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "CategorizedView.h"

#include "CategoryDrawer.h"

#include <KFileItemDelegate>
#include <QSortFilterProxyModel>
#include <KDebug>

CategorizedView::CategorizedView(QWidget *parent)
    : KCategorizedView(parent)
{
    setWordWrap(true);
    CategoryDrawer *drawer = new CategoryDrawer(this);
    setCategoryDrawer(drawer);
}

void CategorizedView::setModel(QAbstractItemModel *model)
{
    KCategorizedView::setModel(model);
// Don't set a fixed grid as this breaks the layout. Let Qt figure out the size itself
#if 0
    //     KFileItemDelegate *delegate = qobject_cast<KFileItemDelegate*>(itemDelegate());
//     kDebug() << delegate->maximumSize();
//     if (delegate) {
        int maxWidth = -1;
        int maxHeight = -1;
        for (int i = 0; i < model->rowCount(); ++i) {
            const QModelIndex index = model->index(i, modelColumn(), rootIndex());
            const QSize size = sizeHintForIndex(index);
            maxWidth = qMax(maxWidth, size.width());
            maxHeight = qMax(maxHeight, size.height());
//             kDebug() << size << index.data(Qt::DisplayRole);
        }
//          kDebug() << maxWidth << maxHeight;
        setGridSize(QSize(maxWidth, maxHeight ));
//         delegate->setMaximumSize(QSize(maxWidth, maxHeight));
// //     }
#endif
}
