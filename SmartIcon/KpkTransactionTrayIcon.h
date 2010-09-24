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

#ifndef KPK_TRANSACTION_TRAY_ICON_H
#define KPK_TRANSACTION_TRAY_ICON_H

#include <KpkAbstractIsRunning.h>

#include <QPackageKit>
#include <KMenu>
#include <KStatusNotifierItem>

using namespace PackageKit;

class KpkTransaction;
class KpkTransactionTrayIcon : public KpkAbstractIsRunning
{
Q_OBJECT
public:
    KpkTransactionTrayIcon(QObject *parent = 0);
    ~KpkTransactionTrayIcon();

    bool isRunning();

signals:
    void removeTransactionWatcher(const QString &tid);

public slots:
    void checkTransactionList();

private slots:
    void transactionListChanged(const QList<PackageKit::Transaction*> &tids);
//     void activated(QSystemTrayIcon::ActivationReason reason);
    void triggered(QAction *action);
    void createTransactionDialog(Transaction *t);
    void transactionDialogClosed();
    void message(PackageKit::Enum::Message type, const QString &message);
    void requireRestart(PackageKit::Enum::Restart type, QSharedPointer<PackageKit::Package>pkg);
    void finished(PackageKit::Enum::Exit exit);
    void transactionChanged();
    void logout();

    void refreshCache();
    void showMessages();
    void hideIcon();

private:
    void updateMenu(const QList<PackageKit::Transaction*> &tids);
    void setCurrentTransaction(PackageKit::Transaction *transaction);

    Enum::Roles m_act;
    KStatusNotifierItem *m_smartSTI;
    Client *m_client;
    Transaction *m_pkClient_updates;
    KMenu *m_menu;
    QHash<QString, KpkTransaction *> m_transDialogs;

    PackageKit::Transaction *m_currentTransaction;

    // Refresh Cache menu entry
    QAction *m_refreshCacheAction;

    // Message Container
    QList<QPair<Enum::Message, QString> > m_messages;
    QAction *m_messagesAction;

    // Restart menu entry
    Enum::Restart m_restartType;
    QAction *m_restartAction;
    QStringList m_restartPackages;

    // Hide this icon action
    QAction *m_hideAction;
};

#endif
