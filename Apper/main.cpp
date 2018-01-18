/***************************************************************************
 *   Copyright (C) 2008-2018 by Daniel Nicoletti                           *
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

#include "Apper.h"
#include <config.h>

#include <QDBusMessage>
#include <QDBusConnection>

#include <QLoggingCategory>

#include <KConfig>
#include <KAboutData>
#include <KLocalizedString>

#include <QIcon>
#include <QDir>

int main(int argc, char **argv)
{
    QApplication::setWindowIcon(QIcon::fromTheme(QLatin1String("system-software-install")));

    Apper app(argc, argv);
    KLocalizedString::setApplicationDomain("apper");

    KAboutData aboutData(QLatin1String("apper"), // DO NOT change this catalog unless you know it will not break translations!
                         i18n("Apper"),
                         QLatin1String(APPER_VERSION),
                         i18n("Apper is an application to get and manage software"),
                         KAboutLicense::LicenseKey::GPL);
    aboutData.addAuthor(i18n("(C) 2008-2018 Daniel Nicoletti"), QString(), QLatin1String("dantti12@gmail.com"), QLatin1String("http://dantti.wordpress.com"));
    aboutData.addCredit(i18n("Adrien Bustany"), i18n("libpackagekit-qt and other stuff"), QLatin1String("@"));
    aboutData.addAuthor(i18n("Matthias Klumpp"), QString(), QLatin1String("matthias@tenstral.net"));

    KAboutData::setApplicationData(aboutData);

    app.activate(app.arguments(), QDir::currentPath());

    return app.exec();
}
