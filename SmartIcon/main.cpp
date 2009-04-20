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

#include "KPackageKitSmartIcon.h"
#include <version.h>

#include <KDebug>
#include <KConfig>
#include <KLocale>
#include <KAboutData>
#include <KCmdLineArgs>

int main(int argc, char **argv)
{
    KAboutData about(
    "kpackagekit-smart-icon", "kpackagekit", ki18n("KPackageKit"),
    KPK_VERSION, ki18n("KPackageKit Tray Icon"),
    KAboutData::License_GPL, ki18n("(C) 2008-2009 Daniel Nicoletti"),
    KLocalizedString(), "http://www.packagekit.org/");

    about.addAuthor(ki18n("Daniel Nicoletti"), KLocalizedString(), "dantti85-pk@yahoo.com.br", "http://www.packagekit.org" );
    about.addAuthor(ki18n("Trever Fischer"), KLocalizedString(), "wm161@wm161.net", "http://wm161.net");

    about.addCredit(ki18n("Adrien Bustany"), ki18n("libpackagekit-qt and other stuff"),"@");

    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    //TODO: icky string
    options.add( "update", ki18n("Perform an automatic system update according to system settings.") );
    KCmdLineArgs::addCmdLineOptions(options);

    kpackagekit::KPackageKit_Smart_Icon::addCmdLineOptions();

    if (!kpackagekit::KPackageKit_Smart_Icon::start()) {
        //kDebug() << "KPackageKit-Smart-Icon is already running!";
        return 0;
    }

    kpackagekit::KPackageKit_Smart_Icon app;
    app.exec();
    return 0;
}
