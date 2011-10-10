/***************************************************************************
 *   Copyright (C) 2009-2010 by Daniel Nicoletti                           *
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

#include <config.h>

#include "AppInstall.h"

#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include <KLocale>
#include <KGlobal>

#define APP_PKG_NAME 0
#define APP_NAME     1
#define APP_SUMMARY  2
#define APP_ICON     3
#define APP_ID       4

#include <KDebug>

AppInstall* AppInstall::m_instance = 0;

AppInstall* AppInstall::instance()
{
    if(!m_instance)
        m_instance = new AppInstall(qApp);

    return m_instance;
}

AppInstall::AppInstall(QObject *parent)
 : QObject(parent)
{
#ifdef HAVE_APPINSTALL
    // load all the data in memory since querying SQLITE is really slow
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "app-install");
    db.setDatabaseName(AI_DB_PATH);
    db.open();
    QSqlQuery query(db);
    query.prepare(
        "SELECT "
            "package_name, "
            "application_name, "
            "application_summary, "
            "icon_name, "
            "application_id "
        "FROM "
            "applications");
//     query.bindValue(":name", KGlobal::locale()->language());
    KGlobal::locale()->insertCatalog("app-install-data");

    m_appInstall = new QHash<QString, QStringList>;
    if (query.exec()) {
        while (query.next()) {
            m_appInstall->insertMulti(query.value(APP_PKG_NAME).toString(),
                                      QStringList()
                                        << i18n(query.value(APP_NAME).toString().toUtf8())
                                        << i18n(query.value(APP_SUMMARY).toString().toUtf8())
                                        << query.value(APP_ICON).toString().split('.')[0]
                                        << query.value(APP_ID).toString());
        }
    }
#endif //HAVE_APPINSTALL
}

QList<QStringList> AppInstall::applications(const QString &pkgName) const
{
    QList<QStringList> ret;
    if (m_appInstall->contains(pkgName)) {
        ret = m_appInstall->values(pkgName);
    }
    return ret;
}

QString AppInstall::genericIcon(const QString &pkgName) const
{
//    kDebug() << pkgName;
    if (m_appInstall->contains(pkgName)) {
        foreach (const QStringList &list, applications(pkgName)) {
            if (!list.at(AppIcon).isEmpty()) {
                return list.at(AppIcon);
            }
        }
    }
    return QString();
}

QStringList AppInstall::pkgNamesFromWhere(const QString &where) const
{
    QStringList packages;
    QSqlDatabase db = QSqlDatabase::database("app-install");
    QSqlQuery query(db);
    query.prepare("SELECT package_name FROM applications WHERE " + where);
    if (query.exec()) {
        while (query.next()) {
            packages << query.value(0).toString();
        }
    }
    return packages;
}

QString AppInstall::thumbnail(const QString &pkgName) const
{
#ifdef HAVE_APPINSTALL
    return "http://screenshots.debian.net/thumbnail/" + pkgName;
#else
    Q_UNUSED(pkgName)
    return QString();
#endif //HAVE_APPINSTALL
}

QString AppInstall::screenshot(const QString &pkgName) const
{
#ifdef HAVE_APPINSTALL
    return "http://screenshots.debian.net/screenshot/" + pkgName;
#else
    Q_UNUSED(pkgName)
    return QString();
#endif //HAVE_APPINSTALL
}
