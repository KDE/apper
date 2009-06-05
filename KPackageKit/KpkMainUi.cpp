/***************************************************************************
 *   Copyright (C) 2009 by Daniel Nicoletti                                *
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

#include "KpkMainUi.h"

#include <KDebug>
#include <KConfig>
#include <KLocale>
#include <KMessageBox>

using namespace PackageKit;

KpkMainUi::KpkMainUi(QWidget *parent)
  : KCMultiDialog(parent),
    m_addrmPWI(0),
    m_updatePWI(0),
    m_settingsPWI(0)
{
    setCaption(QString());
    setWindowIcon(KIcon("applications-other"));

    KConfig config("KPackageKit");
    KConfigGroup kpackagekitMain(&config, "KpkMainUi");
    restoreDialogSize(kpackagekitMain);

    // Set Apply and Cancel buttons
//     setButtons(KDialog::Apply | KDialog::Help | KDialog::Default /*| KDialog::Reset*/);
//     enableButton(KDialog::Apply, false);
//     enableButton(KDialog::Help, false);
//     enableButton(KDialog::Default, false);
    // Find out how to call load() on the kcm
//     enableButton(KDialog::Reset, false);
}

KpkMainUi::~KpkMainUi()
{
    // save size
    KConfig config("KPackageKit");
    KConfigGroup kpackagekitMain(&config, "KpkMainUi");
    saveDialogSize(kpackagekitMain);
    kDebug();
}

void KpkMainUi::showAll()
{
    // check to see if all are added
    showSettings();
    showUpdates();
    if (!m_addrmPWI) {
        m_addrmPWI = addModule(KCModuleInfo::KCModuleInfo("kpk_addrm.desktop"));
    }
    setCurrentPage(m_addrmPWI);
}

void KpkMainUi::showUpdates()
{
    if (!m_updatePWI) {
        m_updatePWI = addModule(KCModuleInfo::KCModuleInfo("kpk_update.desktop"));
    }
    setCurrentPage(m_updatePWI);
}

void KpkMainUi::showSettings()
{
    if (!m_settingsPWI) {
        m_settingsPWI = addModule(KCModuleInfo::KCModuleInfo("kpk_settings.desktop"));
    }
    setCurrentPage(m_settingsPWI);
}

#include "KpkMainUi.moc"
