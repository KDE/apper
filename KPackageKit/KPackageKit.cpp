/***************************************************************************
 *   Copyright (C) 2008-2009 by Daniel Nicoletti                           *
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

#include "KPackageKit.h"

#include <KGlobal>
#include <KStartupInfo>
#include <KCmdLineArgs>
#include <KStandardDirs>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <QStringList>
#include <KCModuleInfo>

#include <QDBusConnection>

#include "KpkInstallFiles.h"
#include "KpkInstallMimeType.h"
#include "KpkInstallPackageName.h"
#include "KpkInstallProvideFile.h"
#include "KpkRemovePackageByFile.h"
#include "KpkBackendDetails.h"

namespace kpackagekit {

KPackageKit::KPackageKit()
 : KUniqueApplication(),
   m_pkUi(0),
   m_running(0),
   m_init(false)
{
    setQuitOnLastWindowClosed(false);
}

KPackageKit::~KPackageKit()
{
}


void KPackageKit::init()
{
    if (!m_init) {
        // If something goes wrong at least kpackagekitSmartIcon
        // will show the error
        QDBusMessage message;
        message = QDBusMessage::createMethodCall("org.kde.KPackageKitSmartIcon",
                                                 "/",
                                                 "org.kde.KPackageKitSmartIcon",
                                                 QLatin1String("UpdateProxy"));
        QDBusMessage reply = QDBusConnection::sessionBus().call(message);
        if (reply.type() != QDBusMessage::ReplyMessage) {
            kWarning() << "Message did not receive a reply";
        }
        m_init = true;
    }
}

void KPackageKit::appClose()
{
    //check whether we can close
    if (!m_running && !m_pkUi) {
        quit();
    }
}

void KPackageKit::kcmFinished()
{
    // kcm is finished we set to 0 to be able to quit
    m_pkUi->deleteLater();
    m_pkUi = 0;
//     m_addrmPWI = 0;
//     m_updatePWI = 0;
//     m_settingsPWI = 0;
    appClose();
}

void KPackageKit::decreaseAndKillRunning()
{
    m_running--;
    sender()->deleteLater();
    appClose();
}

int KPackageKit::newInstance()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    bool notSet = true;
    if (args->count()) {
        // grab the list of files
        KUrl::List urls;
        for (int i = 0; i < args->count(); i++) {
            urls << args->url(i);
        }
        KpkInstallFiles *helper = new KpkInstallFiles(urls, this);
        connect(helper, SIGNAL(close()), this, SLOT(decreaseAndKillRunning()));
        QTimer::singleShot(0, helper, SLOT(start()));
        m_running++;
        notSet = false;
    }

    if (args->isSet("updates")) {
        kDebug() << "SHOW UPDATES!";
        QTimer::singleShot(0, this, SLOT(showUpdates()));
        notSet = false;
    }
    if (args->isSet("settings")) {
        kDebug() << "SHOW SETTINGS!";
        QTimer::singleShot(0, this, SLOT(showSettings()));
        notSet = false;
    }

    if (args->isSet("install-mime-type")) {
        kDebug() << "install-mime-type!" << args->getOptionList("install-mime-type");
        KpkInstallMimeType *helper;
        helper = new KpkInstallMimeType(args->getOptionList("install-mime-type"), this);
        connect(helper, SIGNAL(close()), this, SLOT(decreaseAndKillRunning()));
        QTimer::singleShot(0, helper, SLOT(start()));
        m_running++;
        notSet = false;
    }

    if (args->isSet("install-package-name")) {
        KpkInstallPackageName *helper;
        helper = new KpkInstallPackageName(args->getOptionList("install-package-name"), this);
        connect(helper, SIGNAL(close()), this, SLOT(decreaseAndKillRunning()));
        QTimer::singleShot(0, helper, SLOT(start()));
        m_running++;
        notSet = false;
    }

    if (args->isSet("install-provide-file")) {
        KpkInstallProvideFile *helper;
        helper = new KpkInstallProvideFile(args->getOptionList("install-provide-file"), this);
        connect(helper, SIGNAL(close()), this, SLOT(decreaseAndKillRunning()));
        QTimer::singleShot(0, helper, SLOT(start()));
        m_running++;
        notSet = false;
    }

    if (args->isSet("remove-package-by-file")) {
        KpkRemovePackageByFile *helper;
        helper = new KpkRemovePackageByFile(args->getOptionList("remove-package-by-file"), this);
        connect(helper, SIGNAL(close()), this, SLOT(decreaseAndKillRunning()));
        QTimer::singleShot(0, helper, SLOT(start()));
        m_running++;
        notSet = false;
    }

    if (args->isSet("backend-details")) {
        KpkBackendDetails *helper;
        helper = new KpkBackendDetails;
        connect(helper, SIGNAL(finished()), this, SLOT(decreaseAndKillRunning()));
        QTimer::singleShot(0, helper, SLOT(show()));
        m_running++;
        notSet = false;
    }

    if (notSet) {
        kDebug() << "SHOW UI!";
        QTimer::singleShot(0, this, SLOT(showUi()));
    }
    QTimer::singleShot(0, this, SLOT(init()));

    args->clear();
    return 0;
}

void KPackageKit::showUi()
{
    if (!m_pkUi) {
        m_pkUi = new KpkMainUi();
        connect(m_pkUi, SIGNAL(finished()), this, SLOT (kcmFinished()));
    }
    // Show all
    m_pkUi->showAll();
    m_pkUi->show();
    m_pkUi->activateWindow();
}

void KPackageKit::showUpdates()
{
    if (!m_pkUi) {
        m_pkUi = new KpkMainUi();
        connect(m_pkUi, SIGNAL(finished()), this, SLOT(kcmFinished()));
    }
    m_pkUi->showUpdates();
    m_pkUi->show();
    m_pkUi->activateWindow();
}

void KPackageKit::showSettings()
{
    if (!m_pkUi) {
        m_pkUi = new KpkMainUi();
        connect(m_pkUi, SIGNAL(finished()), this, SLOT(kcmFinished()));
    }
    m_pkUi->showSettings();
    m_pkUi->show();
    m_pkUi->activateWindow();
}

}

#include "KPackageKit.moc"
