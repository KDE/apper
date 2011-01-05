/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
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

#include "Apper.h"
#include <version.h>

#include <KDebug>
#include <KConfig>
#include <KAboutData>
#include <KCmdLineArgs>

int main(int argc, char **argv)
{
    KAboutData about("apper",
                     "apper", // DO NOT change this catalog unless you know it will not break translations!
                     ki18n("Apper"),
                     KPK_VERSION,
                     ki18n("Apper is an Application to Get and Manage Software"),
                     KAboutData::License_GPL,
                     ki18n("(C) 2008-2011 Daniel Nicoletti"));

    about.addAuthor(ki18n("Daniel Nicoletti"), KLocalizedString(), "dantti85-pk@yahoo.com.br", "http://www.packagekit.org");
    about.addCredit(ki18n("Adrien Bustany"), ki18n("libpackagekit-qt and other stuff"), "@");

    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add("updates", ki18n("Show updates"));
    options.add("settings", ki18n("Show settings"));
    options.add("backend-details", ki18n("Show backend details"));
    options.add("install-mime-type <mime-type>", ki18n("Mime type installer"));
    options.add("install-package-name <name>", ki18n("Package name installer"));
    options.add("install-provide-file <file>", ki18n("Single file installer"));
    options.add("install-catalog <file>", ki18n("Catalog installer"));
    options.add("remove-package-by-file <filename>", ki18n("Single package remover"));
    options.add("+[package]", ki18n("Package file to install"));
    KCmdLineArgs::addCmdLineOptions(options);

    Apper::addCmdLineOptions();

    if (!Apper::start())
    {
        qDebug() << "Apper is already running!";
        return 0;
    }

    Apper app;

    return app.exec();
}
