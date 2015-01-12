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

#include <config.h>

#include <appstream.h>

#include "AppStream.h"

#include <QApplication>
#include <QVariant>
#include <QStringBuilder>

#include <KLocale>
#include <KGlobal>
#include <KDebug>

AppStream* AppStream::m_instance = 0;

AppStream* AppStream::instance()
{
    if(!m_instance) {
        m_instance = new AppStream(qApp);
        m_instance->open();
    }

    return m_instance;
}

AppStream::AppStream(QObject *parent)
 : QObject(parent)
{
#ifdef HAVE_APPSTREAM
    // create new AppStream database and screenshot service
    m_asDB = as_database_new();
#endif //HAVE_APPSTREAM
}

AppStream::~AppStream()
{
#ifdef HAVE_APPSTREAM
    g_object_unref(m_asDB);
#endif
}

bool AppStream::open()
{
#ifdef HAVE_APPSTREAM
    bool ret = as_database_open(m_asDB);
    if (!ret) {
        qWarning("Unable to open AppStream Xapian database!");
        return false;
    }

    // cache application data (we might use the db directly, later (making use of AppstreamSearchQuery))
    GPtrArray *cptArray;
    cptArray = as_database_get_all_components(m_asDB);
    if (cptArray == NULL) {
        qWarning("AppStream application array way NULL! (This should never happen)");
        return false;
    }

    for (uint i = 0; i < cptArray->len; i++) {
        AsComponent *cpt;
        GPtrArray *sshot_array;
        AsScreenshot *sshot;
        gchar **pkgs;
        cpt = (AsComponent*) g_ptr_array_index(cptArray, i);
        // we only want desktop apps at time
        if (as_component_get_kind (cpt) != AS_COMPONENT_KIND_DESKTOP_APP)
            continue;

        Application app;
        // Application name
        app.name = QString::fromUtf8(as_component_get_name(cpt));

        // Package name
        pkgs = as_component_get_pkgnames(cpt);
        QString pkgName;
        if (pkgs != NULL)
            pkgName = QString::fromUtf8(pkgs[0]);

        // Desktop file
        app.id = QString::fromUtf8(as_component_get_id(cpt));

        // Summary
        app.summary = QString::fromUtf8(as_component_get_summary(cpt));

        // Description
        app.description = QString::fromUtf8(as_component_get_description(cpt));

        // Application stock icon
        app.icon_url = QString::fromUtf8(as_component_get_icon_url (cpt, 64, 64));

        // Application categories
        gchar **cats = as_component_get_categories(cpt);
        if (cats != NULL) {
            app.categories = QStringList();
            for (int j = 0; cats[j] != NULL; j++) {
                app.categories << QString::fromUtf8(cats[j]);
            }
        }
        g_strfreev(cats);

        // add default screenshot urls
        sshot_array = as_component_get_screenshots (cpt);

        // find default screenshot, if possible
        sshot = NULL;
        for (uint i = 0; i < sshot_array->len; i++) {
            sshot = (AsScreenshot*) g_ptr_array_index (sshot_array, 0);
            if (as_screenshot_get_kind (sshot) == AS_SCREENSHOT_KIND_DEFAULT)
                break;
        }

        if (sshot != NULL) {
            GPtrArray *imgs;
            imgs = as_screenshot_get_images (sshot);
            for (uint i = 0; i < imgs->len; i++) {
                AsImage *img;
                img = (AsImage*) g_ptr_array_index (imgs, i);
                if ((as_image_get_kind (img) == AS_IMAGE_KIND_SOURCE) && (app.screenshot.isEmpty())) {
                    app.screenshot = QString::fromUtf8(as_image_get_url (img));
                } else if ((as_image_get_kind (img) == AS_IMAGE_KIND_THUMBNAIL) && (app.thumbnail.isEmpty())) {
                    app.thumbnail = QString::fromUtf8(as_image_get_url (img));
                }

                if ((!app.screenshot.isEmpty()) && (!app.thumbnail.isEmpty()))
                    break;
            }
        }

        m_appInfo.insertMulti(pkgName, app);
    }
    g_ptr_array_unref(cptArray);

    return true;
#else
    return false;
#endif
}

QList<AppStream::Application> AppStream::applications(const QString &pkgName) const
{
    return m_appInfo.values(pkgName);
}

QString AppStream::genericIcon(const QString &pkgName) const
{
    if (m_appInfo.contains(pkgName)) {
        foreach (const Application &app, applications(pkgName)) {
            if (!app.icon_url.isEmpty()) {
                return app.icon_url;
            }
        }
    }

    return QString();
}

QStringList AppStream::findPkgNames(const CategoryMatcher &parser) const
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

QString AppStream::thumbnail(const QString &pkgName) const
{
#ifdef HAVE_APPSTREAM
    QString url = "";
    if (m_appInfo.contains(pkgName)) {
        Application app = m_appInfo.value(pkgName);
        url = app.thumbnail;
    }

    return url;
#else
    Q_UNUSED(pkgName)
    return QString();
#endif //HAVE_APPSTREAM
}

QString AppStream::screenshot(const QString &pkgName) const
{
#ifdef HAVE_APPSTREAM
    QString url = "";
    if (m_appInfo.contains(pkgName)) {
        Application app = m_appInfo.value(pkgName);
        url = app.screenshot;
    }

    return url;
#else
    Q_UNUSED(pkgName)
    return QString();
#endif //HAVE_APPSTREAM
}
