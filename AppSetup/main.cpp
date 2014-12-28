/* This file is part of Apper
 *
 * Copyright (C) 2012 Matthias Klumpp <matthias@tenstral.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <limba.h>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KDebug>
#include <KApplication>
#include <KMessageBox>
#include <QFileInfo>

#include "SetupWizard.h"

int main(int argc, char** argv)
{
    KAboutData aboutData("apper-appsetup", "apper", ki18n("KDE Application Installer"), "0.2",
                         ki18n("KDE Application Installer"), KAboutData::License_GPL,
                         ki18n("(C) 2014, Matthias Klumpp"));

    aboutData.addAuthor(ki18nc("@info:credit", "Daniel Nicoletti"), ki18n("Developer"),
                        "dantti12@gmail.com");
    aboutData.addAuthor(ki18nc("@info:credit", "Matthias Klumpp"), ki18n("Developer"),
                        "matthias@tenstral.net");
    aboutData.setProductName("apper/limba");

    KCmdLineArgs::init(argc, argv, &aboutData);
    // Add --verbose as commandline option
    KCmdLineOptions options;
    options.add("verbose", ki18n("Show verbose information"));
    options.add("+file", ki18n("IPK package filename"));
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    // Set if we are in verbose mode
    li_set_verbose_mode(args->isSet("verbose"));

    QString fname;
    for(int i = 0; i < args->count(); i++) {
        fname = args->arg(i);
        QFileInfo file(fname);
        if (!file.exists())
            fname = "";
        else
            break;
    }
    args->clear();

    KApplication app;

    // Check if we have a package
    if (fname.isEmpty()) {
        KMessageBox::sorry (0, i18n("We did not receive a path to an IPK package as parameter."),
                            i18n("Package not found!"));
        return 1;
    }

    // Create & run the setup wizard
    SetupWizard *wizard = new SetupWizard();
    wizard->initialize(fname);
    wizard->show();
    return app.exec();
}
