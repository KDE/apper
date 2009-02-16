/***************************************************************************
 *   Copyright (C) 2008-2009 by Daniel Nicoletti                           *
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

#ifndef KPK_TRANSACTION_TRAY_ICON_H
#define KPK_TRANSACTION_TRAY_ICON_H

#include "KpkAbstractSmartIcon.h"

#include <QPackageKit>
#include <KSystemTrayIcon>

using namespace PackageKit;

class KpkTransactionTrayIcon : public KpkAbstractSmartIcon
{
Q_OBJECT
public:
    KpkTransactionTrayIcon(QObject *parent = 0);
    ~KpkTransactionTrayIcon();

    bool isRunning();
    void checkTransactionList();

private slots:
    void transactionListChanged(const QList<PackageKit::Transaction*> &tids);
    void activated(QSystemTrayIcon::ActivationReason reason);
    void triggered(QAction *action);
    void currentStatusChanged(PackageKit::Transaction::Status status);
    void currentProgressChanged(PackageKit::Transaction::ProgressInfo);
//     void showTransactionError(PackageKit::Client::ErrorType, const QString&);
//     void showRestartMessage(PackageKit::Client::RestartType, const QString&);

private:
    void updateMenu(const QList<PackageKit::Transaction*> &tids);
    void setCurrentTransaction(PackageKit::Transaction *transaction);

    Client::Actions m_act;
    KSystemTrayIcon *m_smartSTI;
    Client *m_client;
    Transaction *m_pkClient_updates;
    QMenu *m_menu;
    QHash<QAction *,Transaction*> m_transAction;

    PackageKit::Transaction *m_currentTransaction;
};

#endif
