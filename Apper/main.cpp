/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
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

#include <KDebug>
#include <KConfig>
#include <KAboutData>
#include <KLocalizedString>

#include <KUrl>
#include <QCommandLineParser>
#include <QCommandLineOption>

int invoke(const QString &method_name, const QStringList &args)
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.freedesktop.PackageKit"),
                                             QLatin1String("/org/freedesktop/PackageKit"),
                                             QLatin1String("org.freedesktop.PackageKit.Modify"),
                                             method_name);
    message << (uint) 0;
    message << args;
    message << QString();

    // This call must block otherwise this application closes before
    // smarticon is activated
    QDBusMessage reply = QDBusConnection::sessionBus().call(message, QDBus::Block);
    return reply.type() == QDBusMessage::ErrorMessage ? 1 : 0;
}

int main(int argc, char **argv)
{
    KAboutData aboutData("apper",
                     "apper", // DO NOT change this catalog unless you know it will not break translations!
                     APP_VERSION,
                     i18n("Apper is an application to get and manage software"),
                     KAboutLicense::LicenseKey::GPL);
    aboutData.addAuthor(i18n("Daniel Nicoletti"), QString(), "dantti12@gmail.com", "http://dantti.wordpress.com");
    aboutData.addCredit(i18n("Adrien Bustany"), i18n("libpackagekit-qt and other stuff"), "@");
    aboutData.setProgramIconName("applications-other");

    Apper app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    //PORTING SCRIPT: adapt aboutdata variable if necessary
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("updates"), i18n("Show updates")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("settings"), i18n("Show settings")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("backend-details"), i18n("Show backend details")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("type"), i18n("Mime type installer"), QLatin1String("mime-type")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("name"), i18n("Package name installer"), QLatin1String("name")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("file"), i18n("Single file installer"), QLatin1String("file")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("resource"), i18n("Font resource installer"), QLatin1String("lang")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("catalog"), i18n("Catalog installer"), QLatin1String("file")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("file"), i18n("Single package remover"), QLatin1String("filename")));
    parser.addPositionalArgument(QLatin1String("[package]"), i18n("Package file to install"));

	//! PORTING TODO Fix this
#if 0
    if (parser.positionalArguments().count()) {
        // grab the list of files
        QStringList urls;
        for (int i = 0; i < parser.positionalArguments().count(); i++) {
            urls << parser.positionalArguments()->url(i).url();
        }

        // TODO remote files are copied to /tmp
        // what will happen if we call the other process to
        // install and this very one closes? will the files
        // in /tmp be deleted?
        return invoke("InstallPackageFiles", urls);
    }
#endif

    if (parser.isSet("install-mime-type")) {
        return invoke("InstallMimeTypes", parser.values("install-mime-type"));
    }

    if (parser.isSet("install-package-name")) {
        return invoke("InstallPackageNames", parser.values("install-package-name"));
    }

    if (parser.isSet("install-provide-file")) {
        return invoke("InstallProvideFiles", parser.values("install-provide-file"));
    }

    if (parser.isSet("install-font-resource")) {
        QStringList fonts;
        foreach (const QString &font, parser.values("install-font-resource")) {
            if (font.startsWith(QLatin1String(":lang="))) {
                fonts << font;
            } else {
                fonts << QString(":lang=%1").arg(font);
            }
        }
        return invoke("InstallFontconfigResources", fonts);
    }

    if (parser.isSet("install-catalog")) {
        return invoke("InstallCatalogs", parser.values("install-catalog"));
    }

    if (parser.isSet("remove-package-by-file")) {
        return invoke("RemovePackageByFiles", parser.values("remove-package-by-file"));
    }

    return app.exec();
}
