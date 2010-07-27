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

#include "KpkCheckableHeader.h"

#include <QPainter>
#include <QApplication>
#include <QMouseEvent>

#include <KDebug>

KpkCheckableHeader::KpkCheckableHeader(Qt::Orientation orientation, QWidget *parent)
 : QHeaderView(orientation, parent)
{
}

void KpkCheckableHeader::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    const QStyle *style = QApplication::style();
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if (logicalIndex == 0) {
      QStyleOptionButton option;
      option.rect = rect;
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

      QPoint pos = mapFromGlobal(QCursor::pos());
      QRect rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option);
      kDebug() << rect << pos;
      if (!(pos.x() > rect.width()) &&
          pos.y() < rect.height() &&
          pos.x() >= 0 && pos.y() >= 0) {
          option.state |= QStyle::State_HasFocus;
      }

      option.rect = option.rect;
      // draw item data as CheckBox
      style->drawControl(QStyle::CE_CheckBox, &option, painter);
    }
}

void KpkCheckableHeader::mouseMoveEvent(QMouseEvent *event)
{
    headerDataChanged(Qt::Horizontal, 0, 0);
    QHeaderView::mouseMoveEvent(event);
}

void KpkCheckableHeader::leaveEvent(QEvent *event)
{
    headerDataChanged(Qt::Horizontal, 0, 0);
    QHeaderView::leaveEvent(event);
    kDebug();
}

void KpkCheckableHeader::mousePressEvent(QMouseEvent *event)
{
    const QStyle *style = QApplication::style();
    QStyleOptionButton option;
    QRect rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option);
    if (rect.width() >= event->pos().x()) {
        if (m_state == Qt::Checked) {
            m_state = Qt::Unchecked;
        } else {
            m_state = Qt::Checked;
        }
        headerDataChanged(Qt::Horizontal, 0, 0);
    } else {
        QHeaderView::mousePressEvent(event);
    }
}

void KpkCheckableHeader::setCheckState(Qt::CheckState state)
{
    m_state = state;
}

#include "KpkCheckableHeader.moc"