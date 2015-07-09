/***************************************************************************
 *   Copyright (C) 2009 by Daniel Nicoletti                                *
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

#ifndef TRANSACTION_HISTORY_H
#define TRANSACTION_HISTORY_H

#include "ui_TransactionHistory.h"

class TransactionFilterModel;
class TransactionModel;
class TransactionHistory : public QWidget, Ui::TransactionHistory
{
    Q_OBJECT
public:
    explicit TransactionHistory(QWidget *parent = 0);
    ~TransactionHistory();

public Q_SLOTS:
    void setFilterRegExp(const QString &regexp);

private Q_SLOTS:
    void on_treeView_customContextMenuRequested(const QPoint &pos);
    void refreshList();

private:
    TransactionModel *m_transactionModel;
    TransactionFilterModel *m_proxyModel;
};

#endif
