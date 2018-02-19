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

#include <AppStreamQt/component.h>

#include <QObject>
#include <QHash>

namespace AppStream {
class Pool;
}

struct _AsScreenshotService;
typedef struct _AsScreenshotService AsScreenshotService;

class Q_DECL_EXPORT AppStreamHelper : public QObject {
    public:
        static AppStreamHelper* instance();
        virtual ~AppStreamHelper();
        bool open();

        QList<AppStream::Component> applications(const QString &pkgName) const;
        QString genericIcon(const QString &pkgName) const;
        QStringList findPkgNames(const CategoryMatcher &parser) const;
        QUrl thumbnail(const QString &pkgName) const;
        QUrl screenshot(const QString &pkgName) const;

    private:
        explicit AppStreamHelper(QObject *parent = 0);
        AppStream::Pool *m_pool;

        QHash<QString, AppStream::Component> m_appInfo;
        static AppStreamHelper         *m_instance;
};

#endif // APPSTREAM_H
