/***************************************************************************
 *   Copyright (C) 2009-2010 by Daniel Nicoletti                           *
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

#ifndef FILTERS_MENU_H
#define FILTERS_MENU_H

#include <QMenu>
#include <Transaction>

using namespace PackageKit;

class FiltersMenu : public QMenu
{
Q_OBJECT
public:
    explicit FiltersMenu(Transaction::Filters filters, QWidget *parent = 0);
    ~FiltersMenu();

    Transaction::Filters filters() const;
    QString filterApplications() const;

signals:
    void filtersChanged();
    void filterApplications(const QString &filter);

private slots:
    void filterAppTriggered(bool checked);

private:
    QAction *m_applications;
    QList<QAction*> m_actions;
    QHash<QAction *, Transaction::Filter> m_filtersAction;
};

#endif
