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

#ifndef GRAPHICS_OPACITY_DROP_SHADOW_EFFECT_H
#define GRAPHICS_OPACITY_DROP_SHADOW_EFFECT_H

#include <QGraphicsDropShadowEffect>

class GraphicsOpacityDropShadowEffect : public QGraphicsDropShadowEffect
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity USER true)
public:
    explicit GraphicsOpacityDropShadowEffect(QObject *parent = nullptr);
    ~GraphicsOpacityDropShadowEffect() override;

    qreal opacity() const;
    void  setOpacity(qreal opacity);

    void draw(QPainter *painter) override;

private:
    qreal m_opacity;
};

#endif
