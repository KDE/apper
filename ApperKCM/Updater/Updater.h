/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
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

#ifndef UPDATER_H
#define UPDATER_H

#include "ui_Updater.h"

#include <PkTransactionDialog.h>

#include <Transaction>

using namespace PackageKit;

class PackageModel;
class ApplicationsDelegate;
class CheckableHeader;
class Updater : public QWidget, Ui::Updater
{
    Q_OBJECT
public:
    Updater(Transaction::Roles roles, QWidget *parent);
    ~Updater();

    bool hasChanges() const;
    void setSelected(bool selected);
    QList<Package> packagesToUpdate() const;

signals:
    void changed(bool);
    void refreshCache();
    void downloadSize(const QString &message);

public slots:
    void load();

    void getUpdates();

private slots:
    void on_packageView_customContextMenuRequested(const QPoint &pos);

    void distroUpgrade(PackageKit::Transaction::DistroUpgrade type, const QString &name, const QString &description);

    void getUpdatesFinished();

    void on_packageView_clicked(const QModelIndex &index);

    void checkEnableUpdateButton();
    void errorCode(PackageKit::Transaction::Error error, const QString &details);

    void showVersions(bool enabled);
    void showArchs(bool enabled);
    void showSizes(bool enabled);
    void updatePallete();

private:
    Transaction::Roles    m_roles;
    bool                  m_selected;
    PackageModel         *m_updatesModel;
    ApplicationsDelegate *m_delegate;
    CheckableHeader      *m_header;
    QAction              *m_showPackageVersion;
    QAction              *m_showPackageArch;
    QAction              *m_showPackageSize;
    PkTransactionDialog  *m_transDialog;
    Transaction          *m_updatesT;
    KPixmapSequenceOverlayPainter *m_busySeq;
};

#endif
