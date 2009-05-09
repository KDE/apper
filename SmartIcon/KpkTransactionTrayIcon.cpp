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
#include <QMenu>
#include <QTimer>
#include <QTreeView>
#include <QStandardItemModel>
#include <KNotification>

#include <KDebug>

Q_DECLARE_METATYPE(Transaction*)

KpkTransactionTrayIcon::KpkTransactionTrayIcon(QObject *parent)
 : KpkAbstractIsRunning(parent),
   m_refreshCacheAction(0)
{
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

    if (m_act & Client::ActionRefreshCache) {
        m_refreshCacheAction = new QAction(this);
        m_refreshCacheAction->setText(i18n("Refresh package list"));
        m_refreshCacheAction->setIcon(KIcon("view-refresh"));
        connect(m_refreshCacheAction, SIGNAL(triggered(bool)),
            this, SLOT(refreshCache()));
    }

    m_messagesAction = new QAction(this);
    m_messagesAction->setText(i18n("Show messages"));
    m_messagesAction->setIcon(KIcon("view-pim-notes"));
    connect(m_messagesAction, SIGNAL(triggered(bool)),
            this, SLOT(showMessages()));

    m_hideAction = new QAction(this);
    m_hideAction->setText(i18n("Hide this icon"));
    connect(m_hideAction, SIGNAL(triggered(bool)),
            m_smartSTI, SLOT(hide()));
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

void KpkTransactionTrayIcon::refreshCache()
{
    increaseRunning();
    // in this case refreshCache action must be clicked
    if (Transaction *t = m_client->refreshCache(true)) {
        createTransactionDialog(t);
    } else {
        KMessageBox::sorry(m_menu,
                            i18n("You do not have the necessary privileges to perform this action."),
                            i18n("Failed to refresh package lists"));
    }
    decreaseRunning();
}

void KpkTransactionTrayIcon::triggered(QAction *action)
{
    // Check to see if we set a Transaction->tid() in action
    if (!action->data().isNull()) {
        // we need to find if the action clicked has already a dialog
        QString tid = action->data().toString();
        if (m_transDialogs.contains(tid)) {
            m_transDialogs[tid]->raise();
        } else {
            // if we don't have a dialog already displaying
            // our transaction let's create one
            // BUT first we need to get the transaction pointer,
            // since it might be already deleted if the transaction
            // finished
            foreach(Transaction *trans, Client::instance()->getTransactions()) {
                if (trans->tid() == tid) {
                    // found it let's create a dialog
                    createTransactionDialog(trans);
                    break;
                }
            }
            // if we don't find just don't do anything
        }
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
    emit removeTransactionWatcher(t->tid());
    m_transDialogs[t->tid()] = trans;
    trans->show();
}

void KpkTransactionTrayIcon::transactionDialogClosed()
{
    KpkTransaction *trans = qobject_cast<KpkTransaction*>(sender());
    m_transDialogs.remove(trans->tid());
    // DO NOT delete the kpkTransaction it might have errors to print
    decreaseRunning();
}

void KpkTransactionTrayIcon::transactionListChanged(const QList<PackageKit::Transaction*> &tids)
{
    kDebug() << tids.size();
    if (tids.size()) {
        setCurrentTransaction(tids.first());
    } else {
        m_menu->hide();
        if (m_messages.isEmpty()) {
            m_smartSTI->hide(); // EXPERIMENTAL
//             QTimer::singleShot(0, m_smartSTI, SLOT(hide()));
            emit close();
        }
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
    connect(m_currentTransaction, SIGNAL(message(PackageKit::Client::MessageType, const QString &)),
            this, SLOT(message(PackageKit::Client::MessageType, const QString &)));
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

void KpkTransactionTrayIcon::message(PackageKit::Client::MessageType type, const QString &message)
{
    m_messages.append(qMakePair(type, message));
}

void KpkTransactionTrayIcon::showMessages()
{
    if (!m_messages.isEmpty()) {
        increaseRunning();
        KDialog *dialog = new KDialog;
        dialog->setCaption(i18n("Package Manager Messages"));
        dialog->setButtons(KDialog::Close);

        QTreeView *tree = new QTreeView(dialog);
        tree->setRootIsDecorated(false);
        tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tree->setTextElideMode(Qt::ElideNone);
        tree->setAlternatingRowColors(true);
        dialog->setMainWidget(tree);

        QStandardItemModel *model = new QStandardItemModel(this);
        tree->setModel(model);

        model->setHorizontalHeaderLabels(QStringList() << i18n("Message") << i18n("Details"));
        while (!m_messages.isEmpty()) {
            QPair<Client::MessageType, QString> pair = m_messages.takeFirst();
            model->appendRow(QList<QStandardItem *>()
                             << new QStandardItem(KpkStrings::message(pair.first))
                             << new QStandardItem(pair.second)
                            );
        }
        tree->resizeColumnToContents(0);
        tree->resizeColumnToContents(1);
        connect(dialog, SIGNAL(finished()), this, SLOT(decreaseRunning()));
        // We call this so that the icon can hide if there
        // are no more messages and transactions running
        connect(dialog, SIGNAL(finished()), this, SLOT(checkTransactionList()));
        dialog->show();
    }
}

void KpkTransactionTrayIcon::updateMenu(const QList<PackageKit::Transaction*> &tids)
{
    QString text;
    bool refreshCache = true;

    m_menu->clear();

    foreach (Transaction *t, tids) {
        QAction *transactionAction = new QAction(this);
        // We use the tid since the pointer might get deleted
        // as it was some times
        transactionAction->setData(t->tid());

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

    if (refreshCache && m_refreshCacheAction) {
        m_menu->addSeparator();
        m_menu->addAction(m_refreshCacheAction);
        if (m_messages.size()) {
            m_menu->addAction(m_messagesAction);
        }
        m_menu->addAction(m_hideAction);
    } else if (m_messages.size()) {
        m_menu->addSeparator();
        m_menu->addAction(m_messagesAction);
        m_menu->addAction(m_hideAction);
    } else {
        m_menu->addSeparator();
        m_menu->addAction(m_hideAction);
    }
}

void KpkTransactionTrayIcon::activated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason != QSystemTrayIcon::Context) {
        QList<PackageKit::Transaction*> tids(m_client->getTransactions());
        if (tids.size() || isRunning()) {
            updateMenu(tids);
            m_menu->exec(QCursor::pos());
        } else {
            m_menu->hide();
            m_smartSTI->hide(); // EXPERIMENTAL
            // to avoid warning that the object was deleted in it's event handler
//             QTimer::singleShot(0, m_smartSTI, SLOT(hide()));
        }
    }
}

bool KpkTransactionTrayIcon::isRunning()
{
    return KpkAbstractIsRunning::isRunning() ||
           Client::instance()->getTransactions().size() ||
           m_messages.size();
}

#include "KpkTransactionTrayIcon.moc"
