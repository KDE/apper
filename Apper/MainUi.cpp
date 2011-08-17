/***************************************************************************
 *   Copyright (C) 2009-2011 by Daniel Nicoletti                           *
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

#include "MainUi.h"

#include <QLayout>
#include <KDebug>
#include <KConfig>
#include <KCModuleProxy>

MainUi::MainUi(QWidget *parent)
  : KCMultiDialog(parent),
    m_apperModule(0)
{
    setCaption(QString());
    setWindowIcon(KIcon("applications-other"));

    KConfig config("apper");
    KConfigGroup configGroup(&config, "MainUi");
    restoreDialogSize(configGroup);

    // Set Apply and Cancel buttons
    setButtons(KDialog::Apply /*| KDialog::Help*/ | KDialog::Default | KDialog::Reset);

    KPageWidgetItem *page = addModule("kcm_apper.desktop");
    if (page) {
        KCModuleProxy *proxy = static_cast<KCModuleProxy*>(page->widget());
        if (proxy) {
            m_apperModule = proxy->realModule();
        }
    } else {
        kWarning() << "Could not load kcm_apper.desktop!";
    }
}

MainUi::~MainUi()
{
    // save size
    KConfig config("apper");
    KConfigGroup configGroup(&config, "MainUi");
    saveDialogSize(configGroup);
}

void MainUi::showAll()
{
    if (m_apperModule) {
        m_apperModule->setProperty("page", "home");
    }
}

void MainUi::showUpdates(bool selected)
{
    if (m_apperModule) {
        if (selected) {
            m_apperModule->setProperty("page", "updatesSelected");
        } else {
            m_apperModule->setProperty("page", "updates");
        }
    }
}

void MainUi::showSettings()
{
    if (m_apperModule) {
        m_apperModule->setProperty("page", "settings");
    }
}

#include "MainUi.moc"
