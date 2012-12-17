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

#ifndef PK_IS_INSTALLED_H
#define PK_IS_INSTALLED_H

#include "SessionTask.h"

class PkIsInstalled : public SessionTask
{
Q_OBJECT
public:
    PkIsInstalled(const QString &package_name,
                  const QString &interaction,
                  const QDBusMessage &message,
                  QWidget *parent = 0);
    ~PkIsInstalled();

private slots:
    void searchFinished(PkTransaction::ExitStatus status);

private:
    QString      m_packageName;
    QDBusMessage m_message;
};

#endif