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

#ifndef PK_TRANSACTION_DIALOG_H
#define PK_TRANSACTION_DIALOG_H

#include <KDialog>
#include "PkTransaction.h"

using namespace PackageKit;

class KpkSimulateModel;
class PkTransactionDialogPrivate;
class KDE_EXPORT PkTransactionDialog : public KDialog
{
    Q_OBJECT
    Q_ENUMS(ExitStatus)
public:
    enum BehaviorFlag {
        Modal = 1,
        CloseOnFinish = 2
    };
    Q_DECLARE_FLAGS(Behaviors, BehaviorFlag)

    explicit PkTransactionDialog(Transaction *trans, Behaviors flags = 0, QWidget *parent = 0);
    ~PkTransactionDialog();

    void setTransaction(Transaction *trans);
    PkTransaction* transaction() const;

    KpkSimulateModel* simulateModel() const;

    void setFiles(const QStringList &files);

    PkTransaction::ExitStatus exitStatus() const;

signals:
    void finished(PkTransaction::ExitStatus status);

private slots:
    void finishedDialog(PkTransaction::ExitStatus status);

private:
    PkTransaction *m_ui;
    Behaviors m_flags;
    PkTransactionDialogPrivate *d;

protected slots:
    virtual void slotButtonClicked(int button);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(PkTransactionDialog::Behaviors)

#endif
