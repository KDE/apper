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
#include <version.h>

#include <QDBusMessage>
#include <QDBusConnection>

#include <KDebug>
#include <KConfig>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KUrl>

int invoke(const QString &method_name, const QStringList &args)
{
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.freedesktop.PackageKit",
                                             "/org/freedesktop/PackageKit",
                                             "org.freedesktop.PackageKit.Modify",
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
    KAboutData about("apper",
                     "apper", // DO NOT change this catalog unless you know it will not break translations!
                     ki18n("Apper"),
                     APP_VERSION,
                     ki18n("Apper is an Application to Get and Manage Software"),
                     KAboutData::License_GPL,
                     ki18n("(C) 2008-2011 Daniel Nicoletti"));

    about.addAuthor(ki18n("Daniel Nicoletti"), KLocalizedString(), "dantti12@gmail.com", "http://dantti.wordpress.com");
    about.addCredit(ki18n("Adrien Bustany"), ki18n("libpackagekit-qt and other stuff"), "@");

    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add("updates", ki18n("Show updates"));
    options.add("settings", ki18n("Show settings"));
    options.add("backend-details", ki18n("Show backend details"));
    options.add("install-mime-type <mime-type>", ki18n("Mime type installer"));
    options.add("install-package-name <name>", ki18n("Package name installer"));
    options.add("install-provide-file <file>", ki18n("Single file installer"));
    options.add("install-font-resource <lang>", ki18n("Font resource installer"));
    options.add("install-catalog <file>", ki18n("Catalog installer"));
    options.add("remove-package-by-file <filename>", ki18n("Single package remover"));
    options.add("+[package]", ki18n("Package file to install"));
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->count()) {
        // grab the list of files
        QStringList urls;
        for (int i = 0; i < args->count(); i++) {
            urls << args->url(i).url();
        }

        // TODO remote files are copied to /tmp
        // what will happen if we call the other process to
        // install and this very one closes? will the files
        // in /tmp be deleted?
        return invoke("InstallPackageFiles", urls);
    }

    if (args->isSet("install-mime-type")) {
        return invoke("InstallMimeTypes", args->getOptionList("install-mime-type"));
    }

    if (args->isSet("install-package-name")) {
        return invoke("InstallPackageNames", args->getOptionList("install-package-name"));
    }

    if (args->isSet("install-provide-file")) {
        return invoke("InstallProvideFiles", args->getOptionList("install-provide-file"));
    }

    if (args->isSet("install-font-resource")) {
        QStringList fonts;
        foreach (const QString &font, args->getOptionList("install-font-resource")) {
            if (font.startsWith(QLatin1String(":lang="))) {
                fonts << font;
            } else {
                fonts << QString(":lang=%1").arg(font);
            }
        }
        return invoke("InstallFontconfigResources", fonts);
    }

    if (args->isSet("install-catalog")) {
        return invoke("InstallCatalogs", args->getOptionList("install-catalog"));
    }

    if (args->isSet("remove-package-by-file")) {
        return invoke("RemovePackageByFiles", args->getOptionList("remove-package-by-file"));
    }

    Apper::addCmdLineOptions();

    if (!Apper::start())
    {
        qDebug() << "Apper is already running!";
        return 0;
    }

    Apper app;

    return app.exec();
}
