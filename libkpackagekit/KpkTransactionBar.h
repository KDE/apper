/***************************************************************************
 *   Copyright (C) 2008 by Trever Fischer                                  *
 *   wm161@wm161.net                                                       *
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

#ifndef KPKTRANSACTIONBAR_H
#define KPKTRANSACTIONBAR_H

#include <QPackageKit>
#include <QWidget>
#include <KPushButton>
#include <QTimer>
#include <QProgressBar>
#include <QLabel>
#include <QHBoxLayout>

using namespace PackageKit;

/**
 * @class KpkTransactionBar
 * @short A simple widget to show non-interactive transactions
 *
 * This widget is /only/ meant to be used for simple things like showing
 * feedback during a search operation, or downloading repository information.
 *
 * For more interactive things, such as requiring EULA acceptance or allowing
 * unsigned packages, use KpkTransaction.
 */

class KDE_EXPORT KpkTransactionBar : public QWidget {
    Q_OBJECT

public:
    KpkTransactionBar(QWidget *parent = 0);
    ~KpkTransactionBar();

    enum BehaviorFlag {
        None = 1,
        AutoHide = 2,
        HideCancel = 4,
    };

    Q_DECLARE_FLAGS(Behaviors, BehaviorFlag)

    /**
     * @short Set the currently handled transaction
     *
     * While a transaction is running, the widget is shown if the AutoHide behavior is set.
     */
    void addTransaction(Transaction *trans);

    void setBehaviors(Behaviors);
    Behaviors behaviors() const;

private:
    QList<Transaction*> m_trans;
    QLabel *m_label;
    QProgressBar *m_progress;
    KPushButton *m_cancel;
    QTimer *m_timer;
    QHBoxLayout *m_layout;
    Behaviors m_flags;
    Transaction *m_currTrans;

private slots:
    void finished(PackageKit::Transaction::ExitStatus status, uint runtime);
    void errorCode(PackageKit::Client::ErrorType error, const QString &details);
    void updateUi();
    void nextTransaction();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KpkTransactionBar::Behaviors)

#endif
