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

#ifndef TRANSACTION_JOB_H
#define TRANSACTION_JOB_H

#include <KJob>

#include <Transaction>

#include <QAction>

using namespace PackageKit;

class PkTransactionDialog;
class TransactionTrayIcon;
class TransactionJob : public KJob
{
    Q_OBJECT
public:
    explicit TransactionJob(Transaction *transaction, QObject *parent = 0);
    ~TransactionJob();

    virtual void start();

private slots:
    void transactionDestroyed();
    void finished(PackageKit::Transaction::Exit exit = Transaction::ExitSuccess);
    void package(const PackageKit::Package &package);
    void repoDetail(const QString &repoId, const QString &repoDescription);
    void updateJob();

protected:
    virtual bool doKill();
    void emitDescription();

private:
    Transaction  *m_transaction;
    Transaction::Status  m_status;
    Transaction::Role m_role;
    uint          m_percentage;
    uint          m_speed;
    QString       m_details;
    QStringList   m_packages;
};

#endif
