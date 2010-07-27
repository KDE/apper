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

#ifndef KPK_CHECKABLE_HEADER_H
#define KPK_CHECKABLE_HEADER_H

#include <QHeaderView>

class KpkCheckableHeader : public QHeaderView
{
Q_OBJECT
public:
    KpkCheckableHeader(Qt::Orientation orientation, QWidget *parent = 0);

public slots:
    void setCheckState(Qt::CheckState state);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    Qt::CheckState m_state;
};

#endif
