/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
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

#include "KcmKpkUpdate.h"

#include <KGenericFactory>
#include <KAboutData>

#include <version.h>

K_PLUGIN_FACTORY(KPackageKitFactory, registerPlugin<KcmKpkUpdate>(); )
K_EXPORT_PLUGIN(KPackageKitFactory("kcm_kpk_update"))

KcmKpkUpdate::KcmKpkUpdate(QWidget *&parent, const QVariantList &args)
    : KCModule(KPackageKitFactory::componentData(), parent, args)
{
    KGlobal::locale()->insertCatalog("kpackagekit");
    KAboutData *about = new KAboutData("kcm_kpk_update", "kpackagekit", ki18n("KPackageKit Update"), KPK_VERSION);
    setAboutData(about);
    setButtons(Apply);
    m_grid = new QGridLayout(this);
    view = new KpkUpdate(this);
    m_grid->addWidget(view);
    connect(view, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
}

void KcmKpkUpdate::load()
{
    view->load();
}

void KcmKpkUpdate::save()
{
    view->applyUpdates();
}
