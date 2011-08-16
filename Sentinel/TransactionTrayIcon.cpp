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

#include "TransactionTrayIcon.h"

#include <KMenu>
#include <KLocale>
#include <KActionCollection>
#include <KIcon>
#include <KDebug>

#include <PkStrings.h>
#include <PkIcons.h>

#include <Transaction>

using namespace PackageKit;

TransactionTrayIcon::TransactionTrayIcon(PackageKit::Transaction *transaction, QObject *parent)
 : KStatusNotifierItem(parent)
{
    setCategory(KStatusNotifierItem::SystemServices);
    setStatus(KStatusNotifierItem::Active);

    // Remove standard quit action, as it would quit app
    KActionCollection *actions = actionCollection();
    actions->removeAction(actions->action(KStandardAction::name(KStandardAction::Quit)));
    contextMenu()->addTitle(KIcon("applications-other"), i18n("Transactions"));
    connect(contextMenu(), SIGNAL(triggered(QAction *)),
            this, SLOT(actionActivated(QAction *)));
    connect(contextMenu(), SIGNAL(aboutToShow()),
                this, SLOT(fillMenu()));
    setAssociatedWidget(contextMenu());

    connect(transaction, SIGNAL(transactionChanged()),
            this, SLOT(transactionChanged()));
    m_currentRole     = Transaction::UnknownRole;
    m_currentStatus   = Transaction::UnknownStatus;
    m_currentProgress = 0;
}

TransactionTrayIcon::~TransactionTrayIcon()
{
}

void TransactionTrayIcon::transactionChanged()
{
    PackageKit::Transaction *transaction = static_cast<PackageKit::Transaction*>(sender());

    QString            toolTip;
    uint            percentage = transaction->percentage();
    Transaction::Status status = transaction->status();
    Transaction::Role     role = transaction->role();

    if (m_currentRole != role) {
        m_currentRole = role;
        QString iconName = PkIcons::actionIconName(role);
        setToolTipTitle(PkStrings::action(role));
        setToolTipIconByPixmap(PkIcons::getPreloadedIcon(iconName));
    }

    if (status != m_currentStatus) {
        // Do not store status here
        // so we can compare on the next 'if'
        QString iconName = PkIcons::statusIconName(status);
        setIconByPixmap(PkIcons::getPreloadedIcon(iconName));
    }

    if (percentage != m_currentProgress || status != m_currentStatus) {
        m_currentProgress = percentage;
        m_currentStatus   = status;
        if (percentage && percentage <= 100) {
            toolTip = i18n("%1% - %2", percentage, PkStrings::status(status));
        } else {
            toolTip = i18n("%1", PkStrings::status(status));
        }
        setToolTipSubTitle(toolTip);
    }
}

void TransactionTrayIcon::actionActivated(QAction *action)
{
    kDebug();
    // Check to see if there is transaction id in action
    if (!action->data().isNull()) {
        // we need to find if the action clicked has already a dialog
        Transaction *t = new Transaction(action->data().toString(), this);
        if (!t->error()) {
            emit transactionActivated(t);
        }
    }
}

// void KpkTransactionTrayIcon::createTransactionDialog(PackageKit::Transaction *t)
// {
//     // if we don't have a dialog already displaying
//     // our transaction let's create one
//     // BUT first we need to get the transaction pointer,
//     // since it might be already deleted if the transaction
//     // finished
//     if (m_transDialogs.contains(t->tid())) {
//         KWindowSystem::forceActiveWindow(m_transDialogs[t->tid()]->winId());
//         return;
//     }
//
//     increaseRunning();
//     // we need to close on finish otherwise smart-icon will timeout
//     PkTransactionDialog *trans = new PkTransactionDialog(t, PkTransactionDialog::CloseOnFinish);
//     // Connect to finished since the transaction may fail
//     // due to GPG or EULA and we can't handle this here..
//     connect(trans, SIGNAL(finished()),
//             this, SLOT(transactionDialogClosed()));
//     emit removeTransactionWatcher(t->tid());
//     m_transDialogs[t->tid()] = trans;
//     trans->show();
// }

// void KpkTransactionTrayIcon::transactionDialogClosed()
// {
//     PkTransactionDialog *trans = qobject_cast<PkTransactionDialog*>(sender());
//     m_transDialogs.remove(trans->transaction()->tid());
//     // DO NOT delete the kpkTransaction it might have errors to print
//     decreaseRunning();
// }
