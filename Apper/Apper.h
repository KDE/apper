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

#ifndef APPER_H
#define APPER_H

#include <QApplication>
#include <QQuickView>

#include <Transaction>

using namespace PackageKit;

class MainUi;
class Apper : public QObject
{
Q_OBJECT

public:
    Apper();
    virtual ~Apper();

//    virtual int newInstance();

    Q_INVOKABLE bool supportRole(Transaction::Roles roles, Transaction::Role role);
    Q_INVOKABLE bool supportFilter(Transaction::Filters filters, Transaction::Filter filter);

private slots:
    void appClose();
    void kcmFinished();
    void decreaseAndKillRunning();
    void showUi();
    void showUpdates();
    void showSettings();

private:
//    MainUi *m_pkUi;
    QQuickView *m_view;
    void invoke(const QString &method_name, const QStringList &args);

    int m_running;
    bool m_init;
};


#endif
