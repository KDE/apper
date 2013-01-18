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

#include "TransactionDelegate.h"
#include "PkTransactionProgressModel.h"

#include <QApplication>

#include "PkStrings.h"

#define UNIVERSAL_PADDING 2

using namespace PackageKit;

TransactionDelegate::TransactionDelegate(QObject *parent) :
    QStyledItemDelegate(parent),
    m_minWidth(0)
{
}

void TransactionDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt1(option);
    if (opt1.state & QStyle::State_HasFocus) {
        opt1.state ^= QStyle::State_HasFocus;
    }
    QStyledItemDelegate::paint(painter, opt1, index);
    if (index.column() == 0 && !index.data(PkTransactionProgressModel::RoleRepo).toBool()) {
        int  progress = index.data(PkTransactionProgressModel::RoleProgress).toInt();
        QString text  = index.data(Qt::DisplayRole).toString();

        QStyleOptionProgressBarV2 progressBarOption;
        progressBarOption.state = QStyle::State_Enabled;
        progressBarOption.direction = QApplication::layoutDirection();
        progressBarOption.rect = opt1.rect;
        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = progress;
        progressBarOption.textAlignment = Qt::AlignCenter;
        progressBarOption.text = text;
        progressBarOption.textVisible = true;

        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    }
}

QSize TransactionDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.rheight() += 2 * UNIVERSAL_PADDING;
    size.rwidth()  += 2 * UNIVERSAL_PADDING;
    // The first column keeps resizing
    // this avoids it being smaller
    if (index.column() == 0) {
        if (size.width() < m_minWidth) {
            size.setWidth(m_minWidth);
        } else {
            TransactionDelegate *delegate = const_cast<TransactionDelegate*>(this);
            delegate->m_minWidth = size.width();
        }
    }
    return size;
}

#include "TransactionDelegate.moc"
