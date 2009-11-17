/***************************************************************************
 *   Copyright (C) 2008 by Trever Fischer                                  *
 *   wm161@wm161.net                                                       *
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

#ifndef KPK_UPDATE_ICON_H
#define KPK_UPDATE_ICON_H

#include <KpkAbstractIsRunning.h>

#include <QObject>
#include <KSystemTrayIcon>
#include <KCMultiDialog>
#include <KNotification>
#include <QList>
#include <QPackageKit>

class KpkUpdateIcon : public KpkAbstractIsRunning
{
Q_OBJECT

public:
    KpkUpdateIcon(QObject *parent = 0);
    ~KpkUpdateIcon();

signals:
    void watchTransaction(const QString &tid, bool interactive);

public slots:
    void refreshAndUpdate(bool refresh);

private slots:
    void update();
    void updateListed(PackageKit::Package*);
    void updateCheckFinished(PackageKit::Transaction::ExitStatus, uint runtime);
    void handleUpdateAction(uint action);
    void handleUpdateActionClosed();
    void notifyUpdates();
    void showSettings();
    void showUpdates(QSystemTrayIcon::ActivationReason = KSystemTrayIcon::Unknown);
    void updatesFinished(PackageKit::Transaction::ExitStatus status, uint runtime);

private:
    KSystemTrayIcon* m_icon;
    KNotification *m_updateNotify;
    QList<PackageKit::Package*> m_updateList;

//     int m_inhibitCookie;
//     void suppressSleep(bool enable);
};

#endif
