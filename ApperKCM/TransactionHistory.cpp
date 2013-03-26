/***************************************************************************
 *   Copyright (C) 2009-2010 by Daniel Nicoletti                           *
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

#include "TransactionHistory.h"

#include "TransactionFilterModel.h"
#include "TransactionModel.h"

#include <PkIcons.h>
#include <PkStrings.h>

#include <Daemon>

#include <KMenu>
#include <KMessageBox>

#include <KDebug>

TransactionHistory::TransactionHistory(QWidget *parent)
 : QWidget(parent)
{
    setupUi(this);

    m_transactionModel = new TransactionModel(this);
    m_proxyModel = new TransactionFilterModel(this);
    m_proxyModel->setSourceModel(m_transactionModel);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1);
    treeView->setModel(m_proxyModel);
    treeView->header()->setResizeMode(QHeaderView::ResizeToContents);

    // Get the data
    refreshList();
}

TransactionHistory::~TransactionHistory()
{
}

void TransactionHistory::setFilterRegExp(const QString &regexp)
{
    m_proxyModel->setFilterRegExp(regexp);
}

void TransactionHistory::on_treeView_customContextMenuRequested(const QPoint &pos)
{
    KMenu *menu = new KMenu(this);
    QAction *action;
    action = menu->addAction(i18n("Refresh transactions list"));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(refreshList()));
    menu->exec(treeView->viewport()->mapToGlobal(pos));
    delete menu;
}

void TransactionHistory::refreshList()
{
    // Refresh transaction list
    m_transactionModel->clear();
    Transaction *transaction = new Transaction(this);
    connect(transaction, SIGNAL(transaction(PackageKit::Transaction*)),
            m_transactionModel, SLOT(addTransaction(PackageKit::Transaction*)));
    transaction->getOldTransactions(0);
    if (transaction->internalError()) {
        KMessageBox::sorry(this, PkStrings::daemonError(transaction->internalError()));
    }

    // Refresh time
    QString text;
    uint time = Daemon::global()->getTimeSinceAction(Transaction::RoleRefreshCache) * 1000;
    text = i18n("Time since last cache refresh: %1", KGlobal::locale()->prettyFormatDuration(time));
    timeCacheLabel->setText(text);
}

#include "TransactionHistory.moc"
