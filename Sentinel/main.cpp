/***************************************************************************
 *   Copyright (C) 2008-2010 by Daniel Nicoletti                           *
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

#include "ApperSentinel.h"
#include <version.h>

#include <KDebug>
#include <KConfig>
#include <KLocale>
#include <KAboutData>
#include <KCmdLineArgs>

int main(int argc, char **argv)
{
    KAboutData about("ApperSentinel",
                     "apper",
                     ki18n("Apper"),
                     KPK_VERSION,
                     ki18n("Apper Sentinel"),
                     KAboutData::License_GPL,
                     ki18n("(C) 2008-2011 Daniel Nicoletti"));

    about.addAuthor(ki18n("Daniel Nicoletti"), KLocalizedString(), "dantti85-pk@yahoo.com.br", "http://www.packagekit.org" );
    about.addAuthor(ki18n("Trever Fischer"), KLocalizedString(), "wm161@wm161.net", "http://wm161.net");

    about.addCredit(ki18n("Adrien Bustany"), ki18n("libpackagekit-qt and other stuff"),"@");
    about.setProgramIconName("applications-other");

    KCmdLineArgs::init(argc, argv, &about);

    if (!ApperSentinel::start()) {
        //kDebug() << "ApperSentinel is already running!";
        return 0;
    }

    ApperSentinel app;
    return app.exec();
}