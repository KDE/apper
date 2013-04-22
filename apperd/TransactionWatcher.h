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

//#include "AbstractIsRunning.h"

#include <kuiserverjobtracker.h>

#include <Transaction>

#include <QAction>

using namespace PackageKit;

class TransactionJob;
class TransactionWatcher : public QObject
{
    Q_OBJECT
public:
    explicit TransactionWatcher(bool packagekitIsRunning, QObject *parent = 0);
    ~TransactionWatcher();

public slots:
    void watchTransaction(const QDBusObjectPath &tid, bool interactive = true);
    void showRebootNotificationApt();

private slots:
    void transactionListChanged(const QStringList &tids);
    void message(PackageKit::Transaction::Message type, const QString &message);
    void errorCode(PackageKit::Transaction::Error, const QString &);
    void errorActivated(uint action);
    void requireRestart(PackageKit::Transaction::Restart type, const QString &packageID);
    void finished(PackageKit::Transaction::Exit exit);
    void transactionChanged(Transaction *transaction = 0, bool interactive = false);

    void logout();

    void watchedCanceled();

private:
    static void suppressSleep(bool enable, int &inhibitCookie, const QString &reason = QString());

    // Hash of transactions we are watching
    QHash<QDBusObjectPath, Transaction*> m_transactions;
    QHash<QDBusObjectPath, TransactionJob*> m_transactionJob;

    // cookie to suppress sleep
    int           m_inhibitCookie;
    KUiServerJobTracker *m_tracker;
};

#endif
