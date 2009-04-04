/***************************************************************************
 *   Copyright (C) 2008-2009 by Daniel Nicoletti                           *
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

#include "KpkTransactionTrayIcon.h"
#include <KpkStrings.h>
#include <KpkTransaction.h>
#include <KpkIcons.h>

#include <KMessageBox>
#include <KLocale>
#include <KIcon>
#include <QSize>
#include <QMenu>
#include <KRun>
#include <QTimer>
#include <KNotification>

Q_DECLARE_METATYPE(Transaction*)

#include <KDebug>

KpkTransactionTrayIcon::KpkTransactionTrayIcon(QObject *parent)
 : KpkAbstractSmartIcon(parent)
{
    Client::instance()->setLocale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());

    // Creates our smart icon
    m_smartSTI = new KSystemTrayIcon("applications-other");
    connect(m_smartSTI, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(activated(QSystemTrayIcon::ActivationReason)));

    // Creates our transaction menu
    m_menu = new QMenu(i18n("Transactions"));
    connect(m_menu, SIGNAL(triggered(QAction *)),
            this, SLOT(triggered(QAction *)));

    // sets the contextMenu to our menu so we overwrite the KSystemTrayIcon one
    m_smartSTI->setContextMenu(m_menu);

    // Create a new daemon
    m_client = Client::instance();
    m_act = Client::instance()->getActions();

    connect(m_client, SIGNAL(transactionListChanged(const QList<PackageKit::Transaction*> &)),
            this, SLOT(transactionListChanged(const QList<PackageKit::Transaction*> &)));
}

KpkTransactionTrayIcon::~KpkTransactionTrayIcon()
{
    // this is a QObject that want's a QWidget as parent!?
    m_smartSTI->deleteLater();
    // This is a QWidget
    m_menu->deleteLater();
}

void KpkTransactionTrayIcon::checkTransactionList()
{
    // here the menu can be checked whether or not is visible
    // it's only called here so a just started app can show the current transactions
    transactionListChanged(m_client->getTransactions());
}

void KpkTransactionTrayIcon::triggered(QAction *action)
{
    // Check to see if we set a Transaction* in action
    if (action->data().isNull()) {
        // in this case refreshCache action must be clicked
        if (Transaction *t = m_client->refreshCache(true)) {
            createTransactionDialog(t);
        } else {
            KMessageBox::sorry(m_menu,
                               i18n("You do not have the necessary privileges to perform this action."),
                               i18n("Failed to refresh package lists"));
        }
    } else {
        // we need to find if the action clicked has already a dialog
        QList<Transaction *> values = m_transDialogs.keys();
        Transaction *trans = action->data().value<Transaction*>();
        QString tid = trans->tid();
        for (int i = 0; i < values.size(); ++i) {
            // We have to compare the tids since the Transaction
            // pointer changes.
            if (tid == values.at(i)->tid()) {
                // lets raise the dialog
                m_transDialogs[values.at(i)]->raise();
                return;
            }
        }
        // here will happen when we don't find a dialog
        // for the transaction set in action.
        createTransactionDialog(trans);
    }
}

void KpkTransactionTrayIcon::createTransactionDialog(Transaction *t)
{
    kDebug();
    increaseRunning();
    // we need to close on finish otherwise smart-icon will timeout
    KpkTransaction *trans = new KpkTransaction(t, KpkTransaction::CloseOnFinish, m_menu);
    // Connect to finished since the transaction may fail
    // due to GPG or EULA and we can't handle this here..
    connect(trans, SIGNAL(finished()),
            this, SLOT(transactionDialogClosed()));
    m_transDialogs[t] = trans;
    trans->show();
}

void KpkTransactionTrayIcon::transactionDialogClosed()
{
    KpkTransaction *trans = qobject_cast<KpkTransaction*>(sender());
    m_transDialogs.remove(trans->transaction());
    // DO NOT delete the kpkTransaction it might have errors to print
    decreaseRunning();
}

// void KpkTransactionTrayIcon::showTransactionError(PackageKit::Client::ErrorType err, const QString &details)
// {
// // TODO this needs a DBus interface to track only HIDDEN transactions
// //     KNotification* errorNotification = new KNotification("TransactionError");
// //     errorNotification->setFlags(KNotification::Persistent);
// //     errorNotification->setText("<b>"+KpkStrings::error(err)+"</b><br />"+KpkStrings::errorMessage(err));
// //     KIcon icon("dialog-error");
// //     // Use QSize for proper icon
// //     errorNotification->setPixmap(icon.pixmap(QSize(128, 128)));
// //     errorNotification->sendEvent();
// }

// //FIXME: Implement the proper dbus calls to restart things.
// void KpkTransactionTrayIcon::showRestartMessage(PackageKit::Client::RestartType type, const QString &details)
// {
//     if (type==Client::RestartNone) {
//         return;
//     }
//     KNotification *notify = new KNotification("RestartRequired");
//     QString text;
//     QStringList events;
//     KIcon icon;
//     switch(type) {
//         case Client::RestartApplication:
//             text = i18n("Restart the application for system changes to take effect.");
//             //FIXME: Need a way to detect which program it is
//             icon = KIcon("window-close");
//             break;
//         case Client::RestartSession:
//             text = i18n("You must logout and log back in for system changes to take effect.");
//             icon = KIcon("system-restart"); //FIXME: find the logout icon
//             break;
//         case Client::RestartSystem:
//             text = i18n("Please restart your computer for system changes to take effect.");
//             icon = KIcon("system-restart");
//             break;
//         case Client::RestartNone:
//         case Client::UnknownRestartType:
//             return;
//     }
//     // Use QSize for proper icon
//     notify->setPixmap(icon.pixmap(QSize(128, 128)));
//     notify->setText("<b>"+text+"</b><br />"+details);
//     notify->sendEvent();
// }

void KpkTransactionTrayIcon::transactionListChanged(const QList<PackageKit::Transaction*> &tids)
{
    if (tids.size()) {
        setCurrentTransaction(tids.first());
        updateMenu(tids);
    } else {
        emit close();
        m_menu->hide();
        m_menu->clear();
        QTimer::singleShot(0, m_smartSTI, SLOT(hide()));
    }
}

void KpkTransactionTrayIcon::setCurrentTransaction(PackageKit::Transaction *transaction)
{
    m_currentTransaction = transaction;
    m_smartSTI->setIcon(KpkIcons::statusIcon(m_currentTransaction->status()));
    currentProgressChanged(m_currentTransaction->progress());
    connect(m_currentTransaction, SIGNAL(statusChanged(PackageKit::Transaction::Status)),
            this, SLOT(currentStatusChanged(PackageKit::Transaction::Status)));
    connect(m_currentTransaction, SIGNAL(progressChanged(PackageKit::Transaction::ProgressInfo)),
            this, SLOT(currentProgressChanged(PackageKit::Transaction::ProgressInfo)));
    // TODO we don't want do display all transactions informatiom
    // There will certainly be more than one user so this will never work.
//     connect(tids.first(), SIGNAL(errorCode( PackageKit::Client::ErrorType, const QString)),
//             this, SLOT(showTransactionError(PackageKit::Client::ErrorType, const QString)));
//     connect(tids.first(), SIGNAL(requireRestart(PackageKit::Client::RestartType, const QString)),
//             this, SLOT(showRestartMessage(PackageKit::Client::RestartType, const QString)));
    m_smartSTI->show();
}

void KpkTransactionTrayIcon::currentProgressChanged(PackageKit::Transaction::ProgressInfo info)
{
    QString toolTip;

    if (info.percentage && info.percentage <= 100) {
        toolTip = i18n("%1% - %2", info.percentage, KpkStrings::status(m_currentTransaction->status()));
    } else {
        toolTip = i18n("%1", KpkStrings::status(m_currentTransaction->status()));
    }

    m_smartSTI->setToolTip(toolTip);
}

void KpkTransactionTrayIcon::currentStatusChanged(PackageKit::Transaction::Status status)
{
    QString toolTip;
    PackageKit::Transaction::ProgressInfo info = m_currentTransaction->progress();

    if (info.percentage && info.percentage <= 100) {
        toolTip = i18n("%1% - %2", info.percentage, KpkStrings::status(status));
    } else {
        toolTip = i18n("%1", KpkStrings::status(status));
    }

    m_smartSTI->setToolTip(toolTip);
    m_smartSTI->setIcon(KpkIcons::statusIcon(status));
}

void KpkTransactionTrayIcon::updateMenu(const QList<PackageKit::Transaction*> &tids)
{
    QString text;
    bool refreshCache = true;

    m_menu->clear();

    foreach (Transaction *t, tids) {
        QAction *transactionAction = new QAction(this);
        transactionAction->setData(qVariantFromValue(t));

        if (t->role().action == Client::ActionRefreshCache) {
            refreshCache = false;
        }

        text = KpkStrings::action(t->role().action)
               + ' ' + t->role().terms.join(", ")
               + " (" + KpkStrings::status(t->status()) + ')';
        transactionAction->setText(text);
        transactionAction->setIcon(KpkIcons::statusIcon(t->status()));
        m_menu->addAction(transactionAction);
    }

    QAction *refreshCacheAction = new QAction(this);
    if (refreshCache && m_act.contains(Client::ActionRefreshCache)) {
        m_menu->addSeparator();
        refreshCacheAction->setText(i18n("Refresh Packages List"));
        m_menu->addAction(refreshCacheAction);
    }
}

void KpkTransactionTrayIcon::activated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason != QSystemTrayIcon::Context) {
        QList<PackageKit::Transaction*> tids(m_client->getTransactions());
        if (tids.size()) {
            updateMenu(tids);
            m_menu->exec(QCursor::pos());
        } else {
            m_menu->hide();
            m_menu->clear();
            // to avoid warning that the object was deleted in it's event handler
            QTimer::singleShot(0, m_smartSTI, SLOT(hide()));
        }
    }
}

bool KpkTransactionTrayIcon::isRunning()
{
    return KpkAbstractSmartIcon::isRunning() || m_client->getTransactions().size();
}

#include "KpkTransactionTrayIcon.moc"
