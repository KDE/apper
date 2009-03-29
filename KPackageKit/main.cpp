/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
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

#include "KPackageKit.h"
#include <version.h>

#include <KDebug>
#include <KConfig>
#include <KLocale>
#include <KAboutData>
#include <KCmdLineArgs>

int main(int argc, char **argv)
{
    KAboutData about("KPackageKit",
                     0,
                     ki18n("KPackageKit"),
                     KPK_VERSION,
                     ki18n("KPackageKit user interface"),
                     KAboutData::License_GPL, ki18n("(C) 2008-2009 Daniel Nicoletti"),
                     KLocalizedString());

    about.addAuthor(ki18n("Daniel Nicoletti"), KLocalizedString(), "dantti85-pk@yahoo.com.br", "http://www.packagekit.org");

    about.addCredit(ki18n("Adrien Bustany"), ki18n("libpackagekit-qt and other stuff"), "@");

    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add("updates", ki18n("Show Updates"));
    options.add("settings", ki18n("Show Settings"));
    options.add("install-mime-type <mime-type>", ki18n("Mime Type Installer"));
    options.add("+[package]", ki18n("Package file to install"));
    KCmdLineArgs::addCmdLineOptions(options);

    kpackagekit::KPackageKit::addCmdLineOptions();

    if (!kpackagekit::KPackageKit::start())
    {
        qDebug() << "KPackageKit is already running!";
        return 0;
    }

    kpackagekit::KPackageKit app;

    return app.exec();
}
