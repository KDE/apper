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

#include "BackendDetails.h"
#include "MainUi.h"

#include <KGlobal>
#include <KStartupInfo>
#include <KStandardDirs>
#include <KConfig>
#include <KConfigGroup>
#include <KCModuleInfo>
#include <KWindowSystem>
#include <KDebug>
#include <QStringList>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QTimer>
#include <QUrl>
#include <KDBusService>
#include <KAboutData>
#include <QCommandLineParser>

Apper::Apper(int& argc, char** argv)
 : QApplication(argc, argv),
   m_pkUi(0),
   m_running(0)
{
    setQuitOnLastWindowClosed(false);

    KDBusService *service = new KDBusService(KDBusService::Unique);
    connect(this, &Apper::aboutToQuit, service, &KDBusService::deleteLater);
    connect(service, &KDBusService::activateRequested, this, &Apper::activate);
}

Apper::~Apper()
{
}

void Apper::appClose()
{
    //check whether we can close
    if (!m_running && !m_pkUi) {
        quit();
    }
}

void Apper::kcmFinished()
{
    // kcm is finished we set to 0 to be able to quit
    m_pkUi->deleteLater();
    m_pkUi = 0;
    appClose();
}

void Apper::decreaseAndKillRunning()
{
    m_running--;
    sender()->deleteLater();
    appClose();
}

void Apper::activate(const QStringList& arguments, const QString& workingDirectory)
{
    Q_UNUSED(workingDirectory);

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
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

    KAboutData::applicationData().setupCommandLine(&parser);
    parser.process(arguments);
    KAboutData::applicationData().processCommandLine(&parser);

    auto args = parser.positionalArguments();

    if (args.count()) {
        // grab the list of files
        QStringList urls;
        for (int i = 0; i < args.count(); i++) {
            urls << args[i];
        }

        // TODO remote files are copied to /tmp
        // what will happen if we call the other process to
        // install and this very one closes? will the files
        // in /tmp be deleted?
        invoke("InstallPackageFiles", urls);
        return;
    }

    if (parser.isSet("updates")) {
        QTimer::singleShot(0, this, SLOT(showUpdates()));
        return;
    }
    if (parser.isSet("settings")) {
        QTimer::singleShot(0, this, SLOT(showSettings()));
        return;
    }

    if (parser.isSet("install-mime-type")) {
        invoke("InstallMimeTypes", parser.values("install-mime-type"));
        return;
    }

    if (parser.isSet("install-package-name")) {
        invoke("InstallPackageNames", parser.values("install-package-name"));
        return;
    }

    if (parser.isSet("install-provide-file")) {
        invoke("InstallProvideFiles", parser.values("install-provide-file"));
        return;
    }

    if (parser.isSet("install-catalog")) {
        invoke("InstallCatalogs", parser.values("install-catalog"));
        return;
    }

    if (parser.isSet("remove-package-by-file")) {
        invoke("RemovePackageByFiles", parser.values("remove-package-by-file"));
        return;
    }

    if (parser.isSet("backend-details")) {
        BackendDetails *helper;
        helper = new BackendDetails;
        connect(helper, SIGNAL(finished()), this, SLOT(decreaseAndKillRunning()));
        QTimer::singleShot(0, helper, SLOT(show()));
        m_running++;
        return;
    }

    // If we are here, we neet to show/activate the main UI
    QTimer::singleShot(0, this, SLOT(showUi()));
}

void Apper::showUi()
{
    if (!m_pkUi) {
        m_pkUi = new MainUi();
        connect(m_pkUi, SIGNAL(finished()), this, SLOT (kcmFinished()));
    }
    // Show all
    m_pkUi->showAll();
    m_pkUi->show();
    KWindowSystem::forceActiveWindow(m_pkUi->winId());
}

void Apper::showUpdates()
{
    if (!m_pkUi) {
        m_pkUi = new MainUi();
        connect(m_pkUi, SIGNAL(finished()), this, SLOT(kcmFinished()));
    }
    m_pkUi->showUpdates();
    m_pkUi->show();
    KWindowSystem::forceActiveWindow(m_pkUi->winId());
}

void Apper::showSettings()
{
    if (!m_pkUi) {
        m_pkUi = new MainUi();
        connect(m_pkUi, SIGNAL(finished()), this, SLOT(kcmFinished()));
    }
    m_pkUi->showSettings();
    m_pkUi->show();
    KWindowSystem::forceActiveWindow(m_pkUi->winId());
}

void Apper::invoke(const QString &method_name, const QStringList &args)
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
    QDBusConnection::sessionBus().call(message, QDBus::BlockWithGui);

    QTimer::singleShot(0, this, SLOT(appClose()));
}

#include "Apper.moc"
