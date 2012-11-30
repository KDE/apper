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

#include <QWidget>
#include <QModelIndex>

#include <KPixmapSequenceOverlayPainter>

#include <Transaction>

using namespace PackageKit;

namespace Ui {
    class Updater;
}

class PackageModel;
class ApplicationsDelegate;
class CheckableHeader;
class Updater : public QWidget
{
    Q_OBJECT
public:
    Updater(Transaction::Roles roles, QWidget *parent);
    ~Updater();

    bool hasChanges() const;
    QStringList packagesToUpdate() const;

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
    void showCurrentVersions(bool enabled);
    void showArchs(bool enabled);
    void showOrigins(bool enabled);
    void showSizes(bool enabled);
    void updatePallete();

private:
    Ui::Updater          *ui;
    Transaction::Roles    m_roles;
    bool                  m_selected;
    PackageModel         *m_updatesModel;
    ApplicationsDelegate *m_delegate;
    CheckableHeader      *m_header;
    QAction              *m_showPackageVersion;
    QAction              *m_showPackageCurrentVersion;
    QAction              *m_showPackageArch;
    QAction              *m_showPackageOrigin;
    QAction              *m_showPackageSize;
    Transaction          *m_updatesT;
    KPixmapSequenceOverlayPainter *m_busySeq;
};

#endif
