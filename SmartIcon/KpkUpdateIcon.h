/**************************************************************************
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
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#ifndef KPKUPDATEICON_H
#define KPKUPDATEICON_H

#include <QObject>
#include <KSystemTrayIcon>
#include <KCMultiDialog>
#include <KNotification>
#include <QList>
#include <QPackageKit>

class KpkUpdateIcon : public QObject {

    Q_OBJECT

    public:
        KpkUpdateIcon(QObject* parent=0);
        ~KpkUpdateIcon();

    public slots:
        void checkUpdates();
//         void checkDistroUpgrades();

    private slots:
        void updateListed(PackageKit::Package*);
        void updateCheckFinished(PackageKit::Transaction::ExitStatus, uint runtime);
        void handleUpdateAction(uint action);
        void handleUpdateActionClosed();
        void notifyUpdates();
//         void hideUpdates();
        void showSettings();
//         void updaterClosed(int);
        void showUpdates( QSystemTrayIcon::ActivationReason = QSystemTrayIcon::Unknown);
        void updatesFinished(PackageKit::Transaction::ExitStatus status, uint runtime);
//         void distroUpgrade(PackageKit::Client::UpgradeType, const QString&, const QString&);
//         void handleDistroUpgradeAction(uint action);
//         void distroUpgradeError( QProcess::ProcessError error );
//         void distroUpgradeFinished( int exitCode, QProcess::ExitStatus exitStatus );

    private:
        KSystemTrayIcon* m_icon;
        KNotification *m_updateNotify;
//         KCMultiDialog* m_updateView;
        QList<PackageKit::Package*> m_updateList;
        bool m_checkingUpdates;
        
        int m_inhibitCookie;
        void suppressSleep(bool enable);
};

#endif
