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

#ifndef PK_SEARCH_FILE_H
#define PK_SEARCH_FILE_H

#include "SessionTask.h"

class PkSearchFile : public SessionTask
{
    Q_OBJECT
public:
    PkSearchFile(const QString &file_name,
                 const QString &interaction,
                 const QDBusMessage &message,
                 QWidget *parent = nullptr);
    ~PkSearchFile() override;

protected:
    void notFound() override;
    void searchSuccess() override;

private:
    QString      m_fileName;
    QDBusMessage m_message;
};

#endif
