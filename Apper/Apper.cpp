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
#include <KCmdLineArgs>
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

Apper::Apper()
 : KUniqueApplication(),
   m_pkUi(0),
   m_running(0)
{
    setQuitOnLastWindowClosed(false);
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

int Apper::newInstance()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    bool notSet = true;
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
        invoke("InstallPackageFiles", urls);
        notSet = false;
    }

    if (args->isSet("updates")) {
//         kDebug() << "SHOW UPDATES!";
        QTimer::singleShot(0, this, SLOT(showUpdates()));
        notSet = false;
    }
    if (args->isSet("settings")) {
//         kDebug() << "SHOW SETTINGS!";
        QTimer::singleShot(0, this, SLOT(showSettings()));
        notSet = false;
    }

    if (args->isSet("install-mime-type")) {
        invoke("InstallMimeTypes", args->getOptionList("install-mime-type"));
        notSet = false;
    }

    if (args->isSet("install-package-name")) {
        invoke("InstallPackageNames", args->getOptionList("install-package-name"));
        notSet = false;
    }

    if (args->isSet("install-provide-file")) {
        invoke("InstallProvideFiles", args->getOptionList("install-provide-file"));
        notSet = false;
    }

    if (args->isSet("install-catalog")) {
        invoke("InstallCatalogs", args->getOptionList("install-catalog"));
        notSet = false;
    }

    if (args->isSet("remove-package-by-file")) {
        invoke("RemovePackageByFiles", args->getOptionList("remove-package-by-file"));
        notSet = false;
    }

    if (args->isSet("backend-details")) {
        BackendDetails *helper;
        helper = new BackendDetails;
        connect(helper, SIGNAL(finished()), this, SLOT(decreaseAndKillRunning()));
        QTimer::singleShot(0, helper, SLOT(show()));
        m_running++;
        notSet = false;
    }

    if (notSet) {
//         kDebug() << "SHOW UI!";
        QTimer::singleShot(0, this, SLOT(showUi()));
    }

    args->clear();
    return 0;
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
    m_pkUi->showUpdates(true);
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
