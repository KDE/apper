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

#ifndef CATEGORIZED_VIEW_H
#define CATEGORIZED_VIEW_H

#include <KCategorizedView>

class KDE_EXPORT CategorizedView : public KCategorizedView
{
    Q_OBJECT
public:
    CategorizedView(QWidget *parent = 0);

    virtual void setModel(QAbstractItemModel *model);
};

#endif