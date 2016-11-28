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

#include <KDebug>

#include <KMessageBox>
#include <QFileInfo>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "config.h"

#include "SetupWizard.h"

int main(int argc, char** argv)
{
    KLocalizedString::setApplicationDomain("apper");

    KAboutData aboutData("apper-appsetup",
                "apper",
                APPER_VERSION,
                i18n("KDE Application Installer"),
                KAboutLicense::LicenseKey::GPL);

    aboutData.addAuthor(i18nc("@info:credit", "Daniel Nicoletti"), i18n("Developer"),
                        "dantti12@gmail.com");
    aboutData.addAuthor(i18nc("@info:credit", "Matthias Klumpp"), i18n("Developer"),
                        "matthias@tenstral.net");
    aboutData.setProductName("apper/limba");

    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    // Add --verbose as commandline option
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("verbose"), i18n("Show verbose information")));
    parser.addPositionalArgument(QLatin1String("file"), i18n("IPK package filename"));

    // Set if we are in verbose mode
    li_set_verbose_mode(parser.isSet("verbose"));

    QString fname;
    for(int i = 0; i < parser.positionalArguments().count(); i++) {
        fname = parser.positionalArguments()[i];
        QFileInfo file(fname);
        if (!file.exists())
            fname = "";
        else
            break;
    }
    
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
