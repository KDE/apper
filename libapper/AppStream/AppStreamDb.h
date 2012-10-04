/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti <dantti12@gmail.com>           *
 *   Copyright (C) 2012 by Matthias Klumpp <matthias@tenstral.net>         *
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

#ifndef APPSTREAMDB_H
#define APPSTREAMDB_H

#include "CategoryMatcher.h"

#include <QObject>
#include <QHash>

#include <kdemacros.h>

#undef slots
#include <xapian.h>
#define slots

class KDE_EXPORT AppStreamDb : public QObject {
    public:
        struct Application {
            QString name;
            QString summary;
            QString icon;
            QString id;
            QStringList categories;
        };
        static AppStreamDb* instance();

        QList<Application> applications(const QString &pkgName) const;
        QString genericIcon(const QString &pkgName) const;
        QStringList findPkgNames(const CategoryMatcher &parser) const;
        QString thumbnail(const QString &pkgName) const;
        QString screenshot(const QString &pkgName) const;

    private:
        AppStreamDb(QObject *parent = 0);
        Xapian::Database m_xapianDB;

        QHash<QString, Application> m_appInfo;
        static AppStreamDb         *m_instance;

        void processXapianDoc (Xapian::Document doc);
};

#endif // APPSTREAMDB_H
