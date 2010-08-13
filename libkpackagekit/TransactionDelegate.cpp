/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
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

#include "TransactionDelegate.h"

#include <QApplication>

#include "KpkStrings.h"

using namespace PackageKit;

TransactionDelegate::TransactionDelegate(QObject *parent)
 : QStyledItemDelegate(parent)
{
}

void TransactionDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    if (index.column() == 0) {
        int progress = index.data(Qt::UserRole).toInt();
        Enum::Info info = static_cast<Enum::Info>(index.data(Qt::UserRole + 1).toInt());
        QString text;
        if (progress == 100) {
            text = KpkStrings::infoPast(info);
        } else {
            text = KpkStrings::infoPresent(info);
        }

        QStyleOptionProgressBar progressBarOption;
        progressBarOption.rect = option.rect;
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = progress;
        progressBarOption.text = text;
        progressBarOption.textVisible = true;

        QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                        &progressBarOption, painter);
    } else
        QStyledItemDelegate::paint(painter, option, index);
}

#include "TransactionDelegate.moc"
