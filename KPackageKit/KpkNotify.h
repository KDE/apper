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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef KPKNOTIFY_H
#define KPKNOTIFY_H

#include <QPackageKit>
#include <KNotification>

using namespace PackageKit;

namespace kpackagekit {

class KpkNotify : public QObject
{
Q_OBJECT
public:
    KpkNotify( QObject *parent=0 );
    ~KpkNotify();

    bool canClose() { return !(m_showingUpdates || m_showingAutoUpdate || m_showingRestartReq); };

signals:
    void appClose();
    void showUpdatesUi();
    void showUpdatesTrayIcon();

public slots:
    void showUpdates();
    void rebootRequired(bool required = false);
    void installingAutoUpdates(Transaction *transaction);

private slots:
    // Show updates
    void getUpdatesFinished(PackageKit::Transaction::ExitStatus status, uint runtime);
    void gotPackage(PackageKit::Package *);
    void updatesActions(uint action);
    void showUpdatesClosed();

    // reboot is required
    void rebootRequiredAction();
    void rebootRequiredClosed();

    // auto Installing Updates
    void autoUpdatesActions(uint action);
    void autoUpdatesClosed();

private:
    // reboot is required
    KNotification *m_notifyRestartReq;
    bool m_showingRestartReq;

    // auto Installing Updates
    KNotification *m_notifyAutoUpdate;
    Transaction *m_transactionAutoUpdates;
    bool m_showingAutoUpdate;

    // updates notification
    KNotification *m_showUpdatesNotify;
    QList<Package *> packages;
    bool m_showingUpdates;
};

}

#endif
