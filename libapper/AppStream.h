/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti <dantti12@gmail.com>           *
 *   Copyright (C) 2012-2013 by Matthias Klumpp <matthias@tenstral.net>    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#ifndef APPSTREAM_H
#define APPSTREAM_H

#include "CategoryMatcher.h"

#include <QObject>
#include <QHash>

#include <kdemacros.h>

struct _AsDatabase;
typedef struct _AsDatabase AsDatabase;

struct _AsScreenshotService;
typedef struct _AsScreenshotService AsScreenshotService;

class Q_DECL_EXPORT AppStream : public QObject {
    public:
        struct Application {
            QString name;
            QString summary;
            QString description;
            QString icon_url;
            QString id;
            QStringList categories;
            QString screenshot;
            QString thumbnail;
        };
        static AppStream* instance();
        virtual ~AppStream();
        bool open();

        QList<Application> applications(const QString &pkgName) const;
        QString genericIcon(const QString &pkgName) const;
        QStringList findPkgNames(const CategoryMatcher &parser) const;
        QString thumbnail(const QString &pkgName) const;
        QString screenshot(const QString &pkgName) const;

    private:
        explicit AppStream(QObject *parent = 0);
        AsDatabase *m_asDB;

        QHash<QString, Application> m_appInfo;
        static AppStream         *m_instance;
};

#endif // APPSTREAM_H
