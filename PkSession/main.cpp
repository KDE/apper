/***************************************************************************
 *   Copyright (C) 2008-2010 by Daniel Nicoletti                           *
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

#include "PkSession.h"
#include <config.h>

#include <KDebug>
#include <KConfig>
#include <KLocalizedString>
#include <KAboutData>
#include <KCmdLineArgs>

int main(int argc, char **argv)
{
    KLocalizedString::setApplicationDomain("apper");

    KAboutData aboutData("PkSession",
                     "apper",
                     APPER_VERSION,
                     i18n("Apper PackageKit Session helper"),
                     KAboutLicense::GPL);

    aboutData.addAuthor(i18n("Daniel Nicoletti"), QString(), "dantti12@gmail.com", "http://dantti.wordpress.com" );
    aboutData.addAuthor(i18n("Trever Fischer"), QString(), "wm161@wm161.net", "http://wm161.net");

    aboutData.addCredit(i18n("Adrien Bustany"), i18n("libpackagekit-qt and other stuff"),"@");
    aboutData.setProgramIconName("applications-other");
    KAboutData::setApplicationData(aboutData);

    //! KCmdLineArgs::init(argc, argv);
	Q_UNUSED(argc);
	Q_UNUSED(argv);

    if (!PkSession::start()) {
        //kDebug() << "PkSession is already running!";
        return 0;
    }

    PkSession app;
    return app.exec();
}
