/***************************************************************************
 *   Copyright (C) 2008-2010 by Daniel Nicoletti                           *
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

#ifndef SETTINGS_KCM_H
#define SETTINGS_KCM_H

#include <KCModule>
#include <KPixmapSequenceOverlayPainter>

#include <QPackageKit>

#include "ui_SettingsKCM.h"

using namespace PackageKit;

class KpkModelOrigin;
class SettingsKCM : public KCModule, public Ui::SettingsKCM
{
Q_OBJECT

public:
    SettingsKCM(QWidget *parent, const QVariantList &args);

public slots:
    void load();
    void save();
    void defaults();

signals:
    void changed(bool state);

private slots:
    void on_showOriginsCB_stateChanged(int state);
    void on_editOriginsPB_clicked();
    void checkChanges();

private:
    KPixmapSequenceOverlayPainter *m_busySeq;
    KpkModelOrigin *m_originModel;
    Enum::Roles     m_roles;
};

#endif
