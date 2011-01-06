/***************************************************************************
 *   Copyright (C) 2009-2011 by Daniel Nicoletti                           *
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

#include <KDebug>
#include <KConfig>
#include <KCModuleProxy>

using namespace PackageKit;

MainUi::MainUi(QWidget *parent)
  : KCMultiDialog(parent),
    m_apperModule(0)
{
    setCaption(QString());
    setWindowIcon(KIcon("applications-other"));

    KConfig config("KPackageKit");
    KConfigGroup kpackagekitMain(&config, "MainUi");
    restoreDialogSize(kpackagekitMain);

    // Set Apply and Cancel buttons
    setButtons(KDialog::Apply /*| KDialog::Help*/ | KDialog::Default | KDialog::Reset);

    KPageWidgetItem *page = addModule("kcm_apper.desktop");
    KCModuleProxy *proxy = static_cast<KCModuleProxy*>(page->widget());
    if (proxy) {
        m_apperModule = proxy->realModule();
    }
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
    m_apperModule->setProperty("page", "home");
}

void MainUi::showUpdates(bool selected)
{
    if (selected) {   
        m_apperModule->setProperty("page", "updatesSelected");
    } else {
        m_apperModule->setProperty("page", "updates");
    }
}

void MainUi::showSettings()
{
    m_apperModule->setProperty("page", "settings");
}

#include "MainUi.moc"
