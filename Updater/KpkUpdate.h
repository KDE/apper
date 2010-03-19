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

#ifndef KPK_UPDATE_H
#define KPK_UPDATE_H

#include <QPackageKit>

#include <KUrlLabel>

#include <KpkTransaction.h>

#include "ui_KpkUpdate.h"

class KpkPackageModel;
class KpkDelegate;
class KProgressDialog;

using namespace PackageKit;

class KpkUpdate : public QWidget, Ui::KpkUpdate
{
Q_OBJECT
public:
    KpkUpdate(QWidget *parent = 0);

signals:
    void changed(bool);

public slots:
    void load();
    void applyUpdates();

private slots:
    void on_selectAllPB_clicked();
    void on_refreshPB_clicked();
    void on_historyPB_clicked();

    void distroUpgrade(PackageKit::Enum::DistroUpgrade type, const QString &name, const QString &description);

    void getUpdates();
    void getUpdatesFinished(PackageKit::Enum::Exit status);

    void updatePackages();
    void updatePackagesFinished(KpkTransaction::ExitStatus status);

    void updateColumnsWidth(bool force = false);
    void on_packageView_pressed(const QModelIndex &index);

    void checkEnableUpdateButton();
    void errorCode(PackageKit::Enum::Error error, const QString &details);


private:
    KpkPackageModel *m_pkg_model_updates;

    KpkDelegate *pkg_delegate;
    Client *m_client;
    Transaction *m_updatesT;
    Enum::Roles m_roles;

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual bool event(QEvent *event);
};

#endif
