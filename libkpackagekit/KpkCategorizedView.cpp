/***************************************************************************
 *   Copyright (C) 2009 by Rafael FernÃ¡ndez LÃ³pez <ereslibre@kde.org>      *
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

#include "KpkCategorizedView.h"

#include <KFileItemDelegate>

KpkCategorizedView::KpkCategorizedView(QWidget *parent)
    : KCategorizedView(parent)
{
    setWordWrap(true);
}

void KpkCategorizedView::setModel(QAbstractItemModel *model)
{
    KCategorizedView::setModel(model);
    int maxWidth = -1;
    int maxHeight = -1;
    for (int i = 0; i < model->rowCount(); ++i) {
        const QModelIndex index = model->index(i, modelColumn(), rootIndex());
        const QSize size = sizeHintForIndex(index);
        maxWidth = qMax(maxWidth, size.width());
        maxHeight = qMax(maxHeight, size.height());
    }
    setGridSize(QSize(maxWidth, maxHeight ));
    static_cast<KFileItemDelegate*>(itemDelegate())->setMaximumSize(QSize(maxWidth, maxHeight));
}
