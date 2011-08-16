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

#include "CheckableHeader.h"

#include <QPainter>
#include <QApplication>
#include <QMouseEvent>

#include <KDebug>

#define UNIVERSAL_PADDING 3

CheckableHeader::CheckableHeader(Qt::Orientation orientation, QWidget *parent)
 : QHeaderView(orientation, parent),
   m_state(Qt::Unchecked),
   m_visible(true)
{
}

void CheckableHeader::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    const QStyle *style = QApplication::style();
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    if (logicalIndex == 0 && m_visible) {
        QStyleOptionButton option;
        option.state = QStyle::State_None;
        option.rect = rect;
        if (QApplication::isRightToLeft()) {
            option.rect.setRight(rect.right() - UNIVERSAL_PADDING);
        } else {
            option.rect.setLeft(rect.left() + UNIVERSAL_PADDING);
        }
        option.rect.setLeft(rect.left() + UNIVERSAL_PADDING);
        switch (m_state) {
        case Qt::Unchecked :
            option.state |= QStyle::State_Off;
            break;
        case Qt::PartiallyChecked :
            option.state |= QStyle::State_NoChange;
            break;
        case Qt::Checked :
            option.state |= QStyle::State_On;
            break;
        }

        // Get the cursor position to check if it has focus
        QPoint pos = mapFromGlobal(QCursor::pos());
        QRect rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option);

        if (insideCheckBox(rect, pos)) {
            option.state |= QStyle::State_HasFocus;
        }

        // draw item data as CheckBox
        painter->save();
        style->drawControl(QStyle::CE_CheckBox, &option, painter);
        painter->restore();
    }
}

bool CheckableHeader::insideCheckBox(const QRect &rect, const QPoint &pos) const
{
//     kDebug() << rect << pos;
    if ((pos.x() >= rect.x() && (pos.x() <= rect.x() + rect.width())) &&
        (pos.y() >= rect.y() && (pos.y() <= rect.y() + rect.height()))) {
        return true;
    }
    return false;
}

QSize CheckableHeader::sizeHint() const
{
    const QStyle *style = QApplication::style();
    QStyleOptionButton option;
    QRect rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option);

    QSize size = QHeaderView::sizeHint();
//     kDebug() << size << rect;
    if (size.height() < (rect.height() + 2 * UNIVERSAL_PADDING)) {
        size.setHeight(rect.height() + 2 * UNIVERSAL_PADDING);
    }

    return size;
}

QSize CheckableHeader::sectionSizeFromContents(int logicalIndex) const
{
    QSize size = QHeaderView::sectionSizeFromContents(logicalIndex);
    if (logicalIndex == 0) {      
        const QStyle *style = QApplication::style();
        QStyleOptionButton option;
        QRect rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option);
        
        QString text = model()->headerData(0, Qt::Horizontal).toString();
        QFontMetrics metric = QFontMetrics(QFont());
        int textSize = metric.width(text);
        
        int minimunSize = textSize + 2 * (rect.width() + 2 * (UNIVERSAL_PADDING + 1));
        if (size.width() < minimunSize) {
            size.setWidth(minimunSize);
        }
    }
    return size;
}

void CheckableHeader::mouseMoveEvent(QMouseEvent *event)
{
    headerDataChanged(Qt::Horizontal, 0, 0);
    QHeaderView::mouseMoveEvent(event);
}

void CheckableHeader::leaveEvent(QEvent *event)
{
    headerDataChanged(Qt::Horizontal, 0, 0);
    QHeaderView::leaveEvent(event);
//     kDebug();
}

void CheckableHeader::mousePressEvent(QMouseEvent *event)
{
    if (!m_visible) {
        return;
    }

    const QStyle *style = QApplication::style();
    QStyleOptionButton option;
    option.rect.setSize(sizeHint());
    option.rect.setWidth(viewport()->width());
    QRect rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option);
    QPoint pos = mapFromGlobal(QCursor::pos());
//     kDebug() << rect << pos;

    if (insideCheckBox(rect, pos)) {
        if (m_state == Qt::Checked) {
            m_state = Qt::Unchecked;
        } else {
            m_state = Qt::Checked;
        }
        emit toggled(m_state);
        headerDataChanged(Qt::Horizontal, 0, 0);
    } else {
        QHeaderView::mousePressEvent(event);
    }
}

void CheckableHeader::setCheckState(Qt::CheckState state)
{
    m_state = state;
}

void CheckableHeader::setCheckBoxVisible(bool visible)
{
    m_visible = visible;
    headerDataChanged(Qt::Horizontal, 0, 0);
}

#include "CheckableHeader.moc"
