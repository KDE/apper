/***************************************************************************
 *   Copyright (C) 2009 by Daniel Nicoletti                                *
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

#ifndef KPK_HISTORY_H
#define KPK_HISTORY_H

#include <KDialog>
#include <QSortFilterProxyModel>

#include "KpkTransactionFilterModel.h"
#include "KpkSimpleTransactionModel.h"
#include "ui_KpkHistory.h"

using namespace PackageKit;

class KpkHistory : public KDialog, Ui::KpkHistory
{
    Q_OBJECT
public:
    KpkHistory(QWidget *parent = 0);
    ~KpkHistory();

private slots:
    void finished();

private:
    KpkSimpleTransactionModel *m_transactionModel;
    KpkTransactionFilterModel *m_proxyModel;

protected slots:
    virtual void slotButtonClicked(int button);
};

#endif
