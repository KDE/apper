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

#ifndef TRANSACTION_WATCHER_H
#define TRANSACTION_WATCHER_H

#include "AbstractIsRunning.h"

#include <kuiserverjobtracker.h>

#include <Transaction>

#include <QAction>

using namespace PackageKit;

class StatusNotifierItem;
class TransactionWatcher : public AbstractIsRunning
{
    Q_OBJECT
public:
    TransactionWatcher(QObject *parent = 0);
    ~TransactionWatcher();

    bool isRunning();

private slots:
    void transactionListChanged(const QStringList &tids);
    void message(PackageKit::Transaction::Message type, const QString &message);
    void errorCode(PackageKit::Transaction::Error, const QString &);
    void errorActivated(uint action);
    void requireRestart(PackageKit::Package::Restart type, const PackageKit::Package &pkg);
    void finished(PackageKit::Transaction::Exit exit);
    void transactionChanged();

    void logout();
    void showMessages();
    void hideMessageIcon();
    void hideRestartIcon();

private:
    void suppressSleep(bool enable, const QString &reason = QString());
    void setCurrentTransaction(const QString &tid);

    Transaction *m_currentTransaction;
    QString m_currentTid;
    bool m_transHasJob;

    // Hide this icon action
    QAction *m_hideAction;

    // Message Container
    QStringList  m_messages;
    QAction     *m_messagesAction;
    StatusNotifierItem *m_messagesSNI;

    // Restart menu entry
    Package::Restart m_restartType;
    QStringList      m_restartPackages;
    StatusNotifierItem *m_restartSNI;

    // cookie to suppress sleep
    int           m_inhibitCookie;
    KUiServerJobTracker *m_tracker;
};

#endif
