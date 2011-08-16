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

#ifndef CHECKABLE_HEADER_H
#define CHECKABLE_HEADER_H

#include <QHeaderView>

class CheckableHeader : public QHeaderView
{
Q_OBJECT
public:
    explicit CheckableHeader(Qt::Orientation orientation, QWidget *parent = 0);

    int sectionSizeHint(int logicalIndex) const;
    QSize sizeHint() const;

public slots:
    void setCheckState(Qt::CheckState state);
    void setCheckBoxVisible(bool visible);

signals:
    void toggled(bool checked);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    QSize sectionSizeFromContents(int logicalIndex) const;

private:
    bool insideCheckBox(const QRect &rect, const QPoint &pos) const;

    Qt::CheckState m_state;
    bool m_visible;
};

#endif
