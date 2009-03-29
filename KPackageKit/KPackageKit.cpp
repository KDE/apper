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

#include "KpkInstallMimeType.h"

namespace kpackagekit {

KPackageKit::KPackageKit()
 : KUniqueApplication(),
   m_pkUi(0),
   m_addrmPWI(0),
   m_updatePWI(0),
   m_settingsPWI(0)
{
    m_instFiles = new KpkInstallFiles(this);
    connect(m_instFiles, SIGNAL(appClose()), this, SLOT(appClose()));
    // register Meta Type so we can queue que connection
    qRegisterMetaType<KUrl::List>("KUrl::List &");
    connect(this, SIGNAL(installFiles(KUrl::List &)),
            m_instFiles, SLOT(installFiles(KUrl::List &)),
            Qt::QueuedConnection);
}

KPackageKit::~KPackageKit()
{
}

void KPackageKit::appClose()
{
    //check whether we can close
    if (m_instFiles->canClose() && !m_pkUi) {
        quit();
    }
}

void KPackageKit::kcmFinished()
{
    // kcm is finished we set to 0 to be able to quit
    m_pkUi = 0;
    m_addrmPWI = 0;
    m_updatePWI = 0;
    m_settingsPWI = 0;
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
        emit installFiles(urls);
        notSet = false;
    }

    if (args->isSet("updates")) {
        kDebug() << "SHOW UPDATES!";
        showUpdates();
        notSet = false;
    }
    if (args->isSet("settings")) {
        kDebug() << "SHOW SETTINGS!";
        showSettings();
        notSet = false;
    }

    if (args->isSet("install-mime-type")) {
        kDebug() << "install-mime-type!" << args->getOptionList("install-mime-type");
        KpkInstallMimeType *mime = new KpkInstallMimeType(this);
        mime->installMimeType(args->getOptionList("install-mime-type"));
        notSet = false;
    }

    if (notSet) {
        kDebug() << "SHOW UI!";
        showUi();
    }


    args->clear();
    return 0;
}

void KPackageKit::showUi()
{
    if (!m_pkUi) {
        kDebug() << "GO UI!";
        m_pkUi = new KCMultiDialog();
        m_pkUi->setCaption(QString());
        m_pkUi->setWindowIcon(KIcon("applications-other"));
        connect(m_pkUi, SIGNAL(finished()), this, SLOT (appClose()));
        m_addrmPWI    = m_pkUi->addModule(KCModuleInfo::KCModuleInfo("kpk_addrm.desktop"));
        m_updatePWI   = m_pkUi->addModule(KCModuleInfo::KCModuleInfo("kpk_update.desktop"));
        m_settingsPWI = m_pkUi->addModule(KCModuleInfo::KCModuleInfo("kpk_settings.desktop"));
        m_pkUi->show();
        m_pkUi->activateWindow();
        m_pkUi->raise();
    } else {
        kDebug() << "RAISE UI!";
        // check to see if all are added
        if (!m_addrmPWI) {
            m_addrmPWI    = m_pkUi->addModule(KCModuleInfo::KCModuleInfo("kpk_addrm.desktop"));
        }
        if (!m_updatePWI) {
            m_updatePWI   = m_pkUi->addModule(KCModuleInfo::KCModuleInfo("kpk_update.desktop"));
        }
        if (!m_settingsPWI) {
            m_settingsPWI = m_pkUi->addModule(KCModuleInfo::KCModuleInfo("kpk_settings.desktop"));
        }
        m_pkUi->setCurrentPage(m_addrmPWI);
        m_pkUi->activateWindow();
        m_pkUi->raise();
    }
}

void KPackageKit::showUpdates()
{
    if (!m_pkUi) {
        kDebug() << "GO UI!";
        m_pkUi = new KCMultiDialog();
        m_pkUi->setCaption(QString());
        m_pkUi->setWindowIcon(KIcon("applications-other"));
        connect(m_pkUi, SIGNAL(finished()), this, SLOT(appClose()));
        m_updatePWI = m_pkUi->addModule(KCModuleInfo::KCModuleInfo("kpk_update.desktop"));
        m_pkUi->show();
        m_pkUi->raise();
    } else {
        if (!m_updatePWI) {
            m_updatePWI = m_pkUi->addModule(KCModuleInfo::KCModuleInfo("kpk_update.desktop"));
        }
        m_pkUi->setCurrentPage(m_updatePWI);
        m_pkUi->activateWindow();
    }
}

void KPackageKit::showSettings()
{
    if (!m_pkUi) {
        kDebug() << "GO UI!";
        m_pkUi = new KCMultiDialog();
        m_pkUi->setCaption(QString());
        m_pkUi->setWindowIcon(KIcon("applications-other"));
        connect(m_pkUi, SIGNAL(finished()), this, SLOT(appClose()));
        m_settingsPWI = m_pkUi->addModule(KCModuleInfo::KCModuleInfo("kpk_settings.desktop"));
        m_pkUi->show();
        m_pkUi->raise();
    } else {
        if (!m_settingsPWI) {
            m_settingsPWI   = m_pkUi->addModule(KCModuleInfo::KCModuleInfo("kpk_settings.desktop"));
        }
        m_pkUi->setCurrentPage(m_settingsPWI);
        m_pkUi->activateWindow();
    }
}

}

#include "KPackageKit.moc"
