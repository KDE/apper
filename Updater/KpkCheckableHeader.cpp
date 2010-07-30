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

#define UNIVERSAL_PADDING 4

KpkCheckableHeader::KpkCheckableHeader(Qt::Orientation orientation, QWidget *parent)
 : QHeaderView(orientation, parent),
   m_state(Qt::Unchecked),
   m_enabled(true)
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

      // Drawn it disable is enabled is false
      if (!m_enabled) {
//           option.state &= ~QStyle::State_Enabled;
//           option.state |= QStyle::State_Sunken;
          option.state |= QStyle::State_Sunken;
      } else{
          QPoint pos = mapFromGlobal(QCursor::pos());
          QRect rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option);

          if (insideCheckBox(rect, pos)) {
              option.state |= QStyle::State_HasFocus;
          }
      }
//       bool fooo = (option.state & QStyle::State_Enabled);
//       kDebug() << fooo;

      option.rect = option.rect;
      // draw item data as CheckBox
      painter->save();
      style->drawControl(QStyle::CE_CheckBox, &option, painter);
      painter->restore();
    }
}

bool KpkCheckableHeader::insideCheckBox(const QRect &rect, const QPoint &pos) const
{
//     kDebug() << rect << pos;
    if ((pos.x() >= rect.x() && (pos.x() <= rect.x() + rect.width())) &&
        (pos.y() >= rect.y() && (pos.y() <= rect.y() + rect.height()))) {
        return true;
    }
    return false;
}

QSize KpkCheckableHeader::sizeHint() const
{
    const QStyle *style = QApplication::style();
    QStyleOptionButton option;
    QRect rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option);

    QSize size = QHeaderView::sizeHint();
//     kDebug() << size << rect;
    if (size.height() < (rect.height() + 2 * UNIVERSAL_PADDING)) {
        size.setHeight(rect.height() + 2 * UNIVERSAL_PADDING);
    }
//     kDebug() << size;
    return size;
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
//     kDebug();
}

void KpkCheckableHeader::mousePressEvent(QMouseEvent *event)
{
    if (!m_enabled) {
        return;
    }

    const QStyle *style = QApplication::style();
    QStyleOptionButton option;
    option.rect.setSize(sizeHint());
    option.rect.setWidth(viewport()->width());
    QRect rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option);
    QPoint pos = mapFromGlobal(QCursor::pos());
    kDebug() << rect << pos;

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

void KpkCheckableHeader::setCheckState(Qt::CheckState state)
{
    m_state = state;
}

void KpkCheckableHeader::setCheckBoxEnabled(bool enabled)
{
    m_enabled = enabled;
    headerDataChanged(Qt::Horizontal, 0, 0);
}

#include "KpkCheckableHeader.moc"
