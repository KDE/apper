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

#include "MainUi.h"

#include <QLayout>
#include <KDebug>
#include <KConfig>
#include <KLocale>
#include <KMessageBox>
#include <KCModuleProxy>

using namespace PackageKit;

MainUi::MainUi(QWidget *parent)
  : KCMultiDialog(parent),
    m_addrmPWI(0),
    m_updatePWI(0),
    m_settingsPWI(0)
{
    setCaption(QString());
    setWindowIcon(KIcon("applications-other"));

    KConfig config("KPackageKit");
    KConfigGroup kpackagekitMain(&config, "MainUi");
    restoreDialogSize(kpackagekitMain);

    // Set Apply and Cancel buttons
    setButtons(KDialog::Apply | KDialog::Help | KDialog::Default | KDialog::Reset);
}

MainUi::~MainUi()
{
    // save size
    KConfig config("KPackageKit");
    KConfigGroup kpackagekitMain(&config, "MainUi");
    saveDialogSize(kpackagekitMain);
}

void MainUi::showAll()
{
    // check to see if all are added
    showSettings(false);
    showUpdates(false, false);
    if (!m_addrmPWI) {
        m_addrmPWI = addModule("kpk_addrm.desktop");
    }

    if (currentPage() != m_addrmPWI) {
        setCurrentPage(m_addrmPWI);
    }
}

void MainUi::showUpdates(bool selected, bool forceCurrentPage)
{
    if (!m_updatePWI) {
        // the selected boolean is used to automatically select all updates
        QStringList args;
        if (selected) {
            args << "selected";
        }
        m_updatePWI = addModule("kpk_update.desktop", args);
    }

    if (forceCurrentPage && currentPage() != m_updatePWI) {
        setCurrentPage(m_updatePWI);
    }
}

void MainUi::showSettings(bool forceCurrentPage)
{
    if (!m_settingsPWI) {
        m_settingsPWI = addModule("kpk_settings.desktop");
    }

    if (forceCurrentPage && currentPage() != m_settingsPWI) {
        setCurrentPage(m_settingsPWI);
    }
}

#include "MainUi.moc"
