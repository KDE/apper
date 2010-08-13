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

#ifndef KCM_KPK_UPDATE_H
#define KCM_KPK_UPDATE_H

#include <KCModule>

#include "ui_KcmKpkUpdate.h"

using namespace PackageKit;

class KpkPackageModel;
class KpkDelegate;
class KpkCheckableHeader;
class KProgressDialog;
class KcmKpkUpdate : public KCModule, Ui::KcmKpkUpdate
{
Q_OBJECT
public:
    KcmKpkUpdate(QWidget *&parent, const QVariantList &args);

signals:
    void changed(bool);

public slots:
    void load();
    void save();

private slots:
    void on_refreshPB_clicked();
    void on_historyPB_clicked();

    void distroUpgrade(PackageKit::Enum::DistroUpgrade type, const QString &name, const QString &description);

    void getUpdates();
    void getUpdatesFinished(PackageKit::Enum::Exit status);

    void updatePackages();

    void contractAll();
    void showExtendItem(const QModelIndex &index);

    void checkEnableUpdateButton();
    void errorCode(PackageKit::Enum::Error error, const QString &details);

private:
    bool m_selected;
    KpkPackageModel *m_updatesModel;
    KpkDelegate *m_delegate;
    KpkCheckableHeader *m_header;
    Client *m_client;
    Transaction *m_updatesT;
    Enum::Roles m_roles;
};

#endif
