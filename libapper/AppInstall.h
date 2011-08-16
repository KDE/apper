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

#ifndef APPINSTALL_H
#define APPINSTALL_H

#include <QObject>
#include <QHash>

#include <kdemacros.h>

class KDE_EXPORT AppInstall : public QObject {
    public:
        typedef enum {
            AppName = 0,
            AppSummary,
            AppIcon,
            AppId
        } Position;
        static AppInstall* instance();

        QList<QStringList> applications(const QString &pkgName) const;
        QString genericIcon(const QString &pkgName) const;
        QStringList pkgNamesFromWhere(const QString &where) const;
        QString thumbnail(const QString &pkgName) const;
        QString screenshot(const QString &pkgName) const;

    private:
        AppInstall(QObject *parent = 0);

        QHash<QString, QStringList> *m_appInstall;
        static AppInstall           *m_instance;
};

#endif
