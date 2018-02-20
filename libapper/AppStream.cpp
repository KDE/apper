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

#include <AppStreamQt/pool.h>
#include <AppStreamQt/icon.h>
#include <AppStreamQt/image.h>
#include <AppStreamQt/screenshot.h>

#include "AppStream.h"

#include <QApplication>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(APPER_LIB)

AppStreamHelper* AppStreamHelper::m_instance = 0;

AppStreamHelper* AppStreamHelper::instance()
{
    if(!m_instance) {
        m_instance = new AppStreamHelper(qApp);
        m_instance->open();
    }

    return m_instance;
}

AppStreamHelper::AppStreamHelper(QObject *parent)
 : QObject(parent)
{
#ifdef HAVE_APPSTREAM
    // create new AppStream metadata pool
    m_pool = new AppStream::Pool(this);
#endif //HAVE_APPSTREAM
}

AppStreamHelper::~AppStreamHelper()
{
}

bool AppStreamHelper::open()
{
#ifdef HAVE_APPSTREAM    
    QString error;
    if (!m_pool->load(&error)) {
        qCWarning(APPER_LIB) << "Unable to open AppStream metadata pool:" << error;
        return false;
    }

//    // cache application data (we should actually search the data directly via as_pool_search()...)
//    auto cptArray = as_pool_get_components(m_pool);
//    if (cptArray == NULL) {
//        qWarning("AppStream application array way NULL! (This should never happen)");
//        return false;
//    }

//    for (uint i = 0; i < cptArray->len; i++) {
//        auto cpt = AS_COMPONENT (g_ptr_array_index(cptArray, i));
//        // we only want apps at time
//        auto cptKind = as_component_get_kind (cpt);
//        if ((cptKind != AS_COMPONENT_KIND_DESKTOP_APP) &&
//            (cptKind != AS_COMPONENT_KIND_CONSOLE_APP) &&
//            (cptKind != AS_COMPONENT_KIND_WEB_APP))
//            continue;

//        Application app;
//        // Application name
//        app.name = QString::fromUtf8(as_component_get_name(cpt));

//        // Package name
//        auto pkgnameC = as_component_get_pkgname(cpt);
//        QString pkgName;
//        if (pkgnameC != NULL)
//            pkgName = QString::fromUtf8(pkgnameC);

//        // Desktop file
//        app.id = QString::fromUtf8(as_component_get_id(cpt));

//        // Summary
//        app.summary = QString::fromUtf8(as_component_get_summary(cpt));

//        // Description
//        app.description = QString::fromUtf8(as_component_get_description(cpt));

//        // Application stock icon
//        auto icons = as_component_get_icons(cpt);
//        for (uint i = 0; i < icons->len; i++) {
//            auto icon = AS_ICON (g_ptr_array_index (icons, i));
//            if (as_icon_get_kind (icon) != AS_ICON_KIND_STOCK)
//                 app.icon_url = QString::fromUtf8(as_icon_get_filename (icon));
//        }

//        // Application categories
//        app.categories = QStringList();
//        auto cats = as_component_get_categories(cpt);
//        for (uint i = 0; i < cats->len; i++) {
//            auto category = (const gchar*) g_ptr_array_index (cats, i);
//            app.categories << QString::fromUtf8(category);
//        }

//        // add default screenshot urls
//        auto scrs = as_component_get_screenshots (cpt);

//        // find default screenshot, if possible (otherwise we get a random one)
//        AsScreenshot *scr = NULL;
//        for (uint i = 0; i < scrs->len; i++) {
//            scr = AS_SCREENSHOT (g_ptr_array_index (scrs, i));
//            if (as_screenshot_get_kind (scr) == AS_SCREENSHOT_KIND_DEFAULT)
//                break;
//        }

//        if (scr != NULL) {
//            auto imgs = as_screenshot_get_images (scr);
//            for (uint i = 0; i < imgs->len; i++) {
//                auto img = AS_IMAGE (g_ptr_array_index (imgs, i));
//                if ((as_image_get_kind (img) == AS_IMAGE_KIND_SOURCE) && (app.screenshot.isEmpty())) {
//                    app.screenshot = QString::fromUtf8(as_image_get_url (img));
//                } else if ((as_image_get_kind (img) == AS_IMAGE_KIND_THUMBNAIL) && (app.thumbnail.isEmpty())) {
//                    app.thumbnail = QString::fromUtf8(as_image_get_url (img));
//                }

//                if ((!app.screenshot.isEmpty()) && (!app.thumbnail.isEmpty()))
//                    break;
//            }
//        }

//        m_appInfo.insertMulti(pkgName, app);
//    }

    const QList<AppStream::Component> apps = m_pool->componentsByKind(AppStream::Component::KindDesktopApp);
    for (const AppStream::Component &app : apps) {
        const QStringList pkgNames = app.packageNames();
        for (const QString &pkgName : pkgNames) {
            m_appInfo.insertMulti(pkgName, app);
        }
    }

    return true;
#else
    return false;
#endif
}

QList<AppStream::Component> AppStreamHelper::applications(const QString &pkgName) const
{
    return m_appInfo.values(pkgName);
}

QString AppStreamHelper::genericIcon(const QString &pkgName) const
{
    if (m_appInfo.contains(pkgName)) {
//        const QList<AppStream::Component> apps = applications(pkgName);
//        for (const AppStream::Component &app : apps) {
//            if (!app.icon_url.isEmpty()) {
//                return app.icon_url;
//            }
//        }
    }

    return QString();
}

QStringList AppStreamHelper::findPkgNames(const CategoryMatcher &/*parser*/) const
{
    QStringList packages;

//    QHash<QString, Application>::const_iterator i = m_appInfo.constBegin();
//    while (i != m_appInfo.constEnd()) {
//        if (parser.match(i.value().categories)) {
////            kDebug() << i.key() << categories;
//            packages << i.key();
//        }
//        ++i;
//    }

    return packages;
}

QUrl AppStreamHelper::thumbnail(const QString &pkgName) const
{
    QUrl url;
#ifdef HAVE_APPSTREAM
    if (m_appInfo.contains(pkgName)) {
        AppStream::Image thumb;
        AppStream::Component app = m_appInfo.value(pkgName);
        const QList<AppStream::Screenshot> screenshots = app.screenshots();
        for (const AppStream::Screenshot &screenshot : screenshots) {
            const QList<AppStream::Image> images = screenshot.images();
            for (const AppStream::Image &image : images) {
                if (image.kind() == AppStream::Image::KindThumbnail) {
                    if (thumb.kind() == AppStream::Image::KindUnknown || image.size().height() < thumb.size().height()) {
                        thumb = image;
                        url = image.url();
                    }
                }
            }

            if (screenshot.isDefault() && !url.isEmpty()) {
                break;
            }
        }
    }

    return url;
#else
    Q_UNUSED(pkgName)
    return url;
#endif //HAVE_APPSTREAM
}

QUrl AppStreamHelper::screenshot(const QString &pkgName) const
{
    QUrl url;
#ifdef HAVE_APPSTREAM
    if (m_appInfo.contains(pkgName)) {
        AppStream::Component app = m_appInfo.value(pkgName);
        const QList<AppStream::Screenshot> screenshots = app.screenshots();
        for (const AppStream::Screenshot &screenshot : screenshots) {
            const QList<AppStream::Image> images = screenshot.images();
            for (const AppStream::Image &image : images) {
                url = image.url();
                break;
            }

            if (screenshot.isDefault() && !url.isEmpty()) {
                break;
            }
        }
    }

#else
    Q_UNUSED(pkgName)
#endif //HAVE_APPSTREAM
    return url;
}
