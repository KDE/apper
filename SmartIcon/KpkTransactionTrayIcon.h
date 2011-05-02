/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
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

#include <kuiserverjobtracker.h>
#include <kwidgetjobtracker.h>

#include <QPackageKit>

#include <QAction>

using namespace PackageKit;

class PkTransactionDialog;
class TransactionTrayIcon;
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
    void createTransactionDialog(PackageKit::Transaction *t);
    void transactionDialogClosed();
    void message(PackageKit::Enum::Message type, const QString &message);
    void requireRestart(PackageKit::Enum::Restart type, QSharedPointer<PackageKit::Package>pkg);
    void finished(PackageKit::Enum::Exit exit);
    void transactionChanged();
    void logout();

    void refreshCache();
    void showMessages();
    void hideIcon();
    void fillMenu();

private:
    void suppressSleep(bool enable, const QString &reason = QString());
    void updateMenu(const QList<PackageKit::Transaction*> &tids);
    void setCurrentTransaction(PackageKit::Transaction *transaction);

    TransactionTrayIcon *m_trayIcon;
    Client              *m_client;
    QHash<QString, PkTransactionDialog *> m_transDialogs;

    Transaction *m_currentTransaction;

    // Refresh Cache menu entry
    QAction *m_refreshCacheAction;

    // Hide this icon action
    QAction *m_hideAction;

    // Message Container
    int      m_messagesCount;
    QString  m_messages;
    QAction *m_messagesAction;

    // Restart menu entry
    Enum::Restart m_restartType;
    QAction      *m_restartAction;
    QStringList   m_restartPackages;

    // Cache data for tooltip
    Enum::Status  m_currentStatus;
    Enum::Role    m_currentRole;
    uint          m_currentProgress;
    
    // cookie to suppress sleep
    int           m_inhibitCookie;
    QStringList   m_tids;
    KUiServerJobTracker *m_tracker;
};

#endif
