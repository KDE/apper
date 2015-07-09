/***************************************************************************
 *   Copyright (C) 2009-2011 by Daniel Nicoletti                           *
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

#ifndef ABSTRACT_IS_RUNNING_H
#define ABSTRACT_IS_RUNNING_H

#include <kdemacros.h>

#include <QObject>

class AbstractIsRunning : public QObject
{
    Q_OBJECT
public:
    explicit AbstractIsRunning(QObject *parent = 0);
    ~AbstractIsRunning();

    bool isRunning() const;

Q_SIGNALS:
    void close();

protected Q_SLOTS:
    void increaseRunning();
    void decreaseRunning();

private:
    int m_running;
};

#endif
