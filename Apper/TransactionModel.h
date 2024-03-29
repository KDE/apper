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

#ifndef TRANSACTION_MODEL_H
#define TRANSACTION_MODEL_H

#include <QStandardItemModel>
#include <Transaction>

using namespace PackageKit;

class TransactionModel : public QStandardItemModel
{
Q_OBJECT

public:
    explicit TransactionModel(QObject *parent = nullptr);

    void clear();

public Q_SLOTS:
    void addTransaction(PackageKit::Transaction *trans);

private:
    QString getDetailsLocalized(const QString &data) const;
    QString getTypeLine(const QStringList &lines, Transaction::Status status) const;
};

#endif
