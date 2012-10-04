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

#include <config.h>

#include "AppStreamDb.h"

#include <QApplication>
#include <QVariant>
#include <QStringBuilder>

#include <KLocale>
#include <KGlobal>
#include <KDebug>

#include <iostream>
#include <string>

#include "database-common.h"

using namespace std;

AppStreamDb* AppStreamDb::m_instance = 0;

AppStreamDb* AppStreamDb::instance()
{
    if(!m_instance)
        m_instance = new AppStreamDb(qApp);

    return m_instance;
}

AppStreamDb::AppStreamDb(QObject *parent)
 : QObject(parent)
{
#ifdef HAVE_APPSTREAM
    try {
        // hardcode path for now, since we don't use LibAppStreamDb in Apper
        m_xapianDB = Xapian::Database("/var/cache/software-center/xapian");
    } catch (const Xapian::Error &error) {
        qWarning("Unable to load AppStreamDb Xapian database: %s", error.get_msg ().c_str ());
        return;
    }

    // cache application data (we might use the db directly later)
    // Iterate through all Xapian documents
    Xapian::PostingIterator it = m_xapianDB.postlist_begin (string());
    while (it != m_xapianDB.postlist_end(string())) {
        Xapian::docid did = *it;

        Xapian::Document doc = m_xapianDB.get_document (did);
        processXapianDoc (doc);

        ++it;
    }
#endif //HAVE_APPSTREAM
}

void AppStreamDb::processXapianDoc(Xapian::Document doc)
{
    Application app;

    // Application name
    app.name = QString::fromUtf8(doc.get_value(AppStream::APPNAME).c_str());

    // Package name
    QString pkgName = QString::fromUtf8(doc.get_value (AppStream::PKGNAME).c_str());

    // Desktop file
    app.id = QString::fromUtf8(doc.get_value(AppStream::DESKTOP_FILE).c_str());

    // Summary
    app.summary = QString::fromUtf8(doc.get_value(AppStream::SUMMARY).c_str());

    // Application stock icon
    app.icon = QString::fromUtf8(doc.get_value(AppStream::ICON).c_str());

    // Application categories
    QString categories = QString::fromUtf8(doc.get_value(AppStream::CATEGORIES).c_str());
    app.categories = categories.split(QLatin1String(";"));

    m_appInfo.insertMulti(pkgName, app);
}

QList<AppStreamDb::Application> AppStreamDb::applications(const QString &pkgName) const
{
    return m_appInfo.values(pkgName);;
}

QString AppStreamDb::genericIcon(const QString &pkgName) const
{
    if (m_appInfo.contains(pkgName)) {
        foreach (const Application &app, applications(pkgName)) {
            if (!app.icon.isEmpty()) {
                return app.icon;
            }
        }
    }

    return QString();
}

QStringList AppStreamDb::findPkgNames(const CategoryMatcher &parser) const
{
    QStringList packages;

    QHash<QString, Application>::const_iterator i = m_appInfo.constBegin();
    while (i != m_appInfo.constEnd()) {
        if (parser.match(i.value().categories)) {
//            kDebug() << i.key() << categories;
            packages << i.key();
        }
        ++i;
    }

    return packages;
}

QString AppStreamDb::thumbnail(const QString &pkgName) const
{
#ifdef HAVE_APPSTREAM
    return QLatin1String("http://screenshots.debian.net/thumbnail/") % pkgName;
#else
    Q_UNUSED(pkgName)
    return QString();
#endif //HAVE_APPSTREAM
}

QString AppStreamDb::screenshot(const QString &pkgName) const
{
#ifdef HAVE_APPSTREAM
    return QLatin1String("http://screenshots.debian.net/screenshot/") % pkgName;
#else
    Q_UNUSED(pkgName)
    return QString();
#endif //HAVE_APPSTREAM
}
