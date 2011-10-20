/***************************************************************************
 *   Copyright (C) 2011 by Daniel Nicoletti                                *
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

#ifndef INFO_WIDGET_H
#define INFO_WIDGET_H

#include <QWidget>
#include <QAbstractItemModel>
#include <KIcon>

namespace Ui {
    class InfoWidget;
}

class FilesModel;
class InfoWidget : public QWidget
{
    Q_OBJECT
public:
    InfoWidget(QWidget *parent = 0);
    ~InfoWidget();

    void setDescription(const QString &description);
    void setIcon(const KIcon &icon);
    void setDetails(const QString &details);
    void addWidget(QWidget *widget);

private:
    Ui::InfoWidget *ui;
};

#endif