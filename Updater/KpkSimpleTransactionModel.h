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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef KPK_SIMPLE_TRANSACTION_MODEL_H
#define KPK_SIMPLE_TRANSACTION_MODEL_H

#include <QStandardItemModel>
#include <KIcon>

#include <QPackageKit>

using namespace PackageKit;

class KpkSimpleTransactionModel : public QStandardItemModel
{
Q_OBJECT

public:
    KpkSimpleTransactionModel(QObject *parent = 0);

    void clear();

public slots:
    void addTransaction(PackageKit::Transaction *trans);

private:
    QString getDetailsLocalized(const QString &data) const;
    QString getTypeLine(const QStringList &lines, Transaction::Status status) const;

    QList<PackageKit::Transaction *> m_transactions;
};

#endif