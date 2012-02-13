/***************************************************************************
 *   Copyright (C) 2010-2011 by Daniel Nicoletti                           *
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

#ifndef TRANSACTION_TRAY_ICON_H
#define TRANSACTION_TRAY_ICON_H

#include <KStatusNotifierItem>
#include <QAction>

#include <Transaction>

using namespace PackageKit;

class TransactionTrayIcon : public KStatusNotifierItem
{
Q_OBJECT
public:
    explicit TransactionTrayIcon(PackageKit::Transaction *transaction, QObject *parent = 0);
    ~TransactionTrayIcon();

signals:
    void transactionActivated(PackageKit::Transaction *transaction);

private slots:
    void actionActivated(QAction *action);
    void transactionChanged();

private:
    // Cache data for tooltip
    Transaction::Status m_currentStatus;
    Transaction::Role   m_currentRole;
    uint                m_currentProgress;
};

#endif
