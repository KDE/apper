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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include <KPixmapSequenceOverlayPainter>

#include <Transaction>

using namespace PackageKit;

namespace Ui {
    class Settings;
}

class OriginModel;
class Settings : public QWidget
{
    Q_OBJECT
public:
    Settings(Transaction::Roles roles, QWidget *parent);
    ~Settings();

    bool hasChanges() const;

public slots:
    void load();
    void save();
    void defaults();
    void showGeneralSettings();
    void showRepoSettings();

signals:
    void changed(bool state);
    void refreshCache();

private slots:
    void refreshRepoModel();
    void on_showOriginsCB_stateChanged(int state);
    void on_editOriginsPB_clicked();
    void checkChanges();

private:
    Ui::Settings *ui;
    KPixmapSequenceOverlayPainter *m_busySeq;
    OriginModel *m_originModel;
    Transaction::Roles  m_roles;
};

#endif
