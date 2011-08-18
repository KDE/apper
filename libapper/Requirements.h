/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
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

#ifndef REQUIREMENTS_H
#define REQUIREMENTS_H

#include <KDialog>
#include <QModelIndex>

namespace Ui {
    class Requirements;
}

class SimulateModel;
class Requirements : public KDialog
{
    Q_OBJECT
public:
    explicit Requirements(SimulateModel *model, QWidget *parent = 0);
    ~Requirements();

public slots:
    void show();
    virtual void setPlainCaption(const QString &caption);

private slots:
    void actionClicked(const QModelIndex &index);

private:
    bool m_hideAutoConfirm;

    Ui::Requirements *ui;
};

#endif
