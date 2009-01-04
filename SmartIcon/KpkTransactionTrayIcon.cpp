/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
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

#include <KDebug>

#define SEVEN_MINUTES 420000

KpkTransactionTrayIcon::KpkTransactionTrayIcon( QObject *parent ) : QObject( parent )
{
    Client::instance()->setLocale(KGlobal::locale()->language() + "." + KGlobal::locale()->encoding());

    m_smartSTI = new KSystemTrayIcon("applications-other");
    connect(m_smartSTI, SIGNAL( activated(QSystemTrayIcon::ActivationReason) ), this, SLOT( activated(QSystemTrayIcon::ActivationReason) ) );

    // Create a new daemon
    m_client = Client::instance();
    m_act = Client::instance()->getActions();

    connect(m_client, SIGNAL( transactionListChanged(const QList<PackageKit::Transaction*> &) ), this, SLOT( transactionListChanged(const QList<PackageKit::Transaction*> &) ) );

    m_menu = new QMenu(i18n("Transactions"));
    connect(m_menu, SIGNAL( triggered(QAction *) ), this, SLOT( triggered(QAction *) ) );
}

KpkTransactionTrayIcon::~KpkTransactionTrayIcon()
{
}

void KpkTransactionTrayIcon::checkTransactionList()
{
    // here the menu can be checked whether or not is visible
    // it's only called here so a just started app can show the current transactions
    transactionListChanged( m_client->getTransactions() );
}

void KpkTransactionTrayIcon::triggered(QAction *action)
{
    if ( m_transAction.contains(action) ) {
        KpkTransaction *transaction = new KpkTransaction( m_transAction[action], false);
        transaction->show();
    }
    else {
        if ( Transaction *t = m_client->refreshCache(true) ) {
            KpkTransaction *frm = new KpkTransaction(t, false);
            frm->show();
        }
        else
            KMessageBox::error( m_menu, i18n("Authentication failed"), i18n("KPackageKit") );
    }
}

void KpkTransactionTrayIcon::transactionListChanged(const QList<PackageKit::Transaction*> &tids)
{
    if ( m_menu->isVisible() && tids.size() ) {
        m_smartSTI->setIcon( KpkIcons::statusIcon( tids.first()->status() ) );
        connect( tids.first(), SIGNAL( statusChanged(PackageKit::Transaction::Status) ),
            this, SLOT( currentStatusChanged(PackageKit::Transaction::Status) ) );
        m_smartSTI->show();
        updateMenu(tids);
        emit cancelClose();
    }
    else if ( tids.size() == 0){
        kDebug() << "No more transactions";
        m_menu->hide();
        // to avoid warning that the object was deleted in it's event handler
        QTimer::singleShot(1, m_smartSTI, SLOT( hide() ) );
        // this will start a timer to close the app
        m_menu->clear();
        emit appClose(SEVEN_MINUTES);
    }
    else {
        m_smartSTI->setIcon( KpkIcons::statusIcon( tids.first()->status() ) );
        connect( tids.first(), SIGNAL( statusChanged(PackageKit::Transaction::Status) ),
            this, SLOT( currentStatusChanged(PackageKit::Transaction::Status) ) );
        m_smartSTI->show();
    }
}

void KpkTransactionTrayIcon::currentStatusChanged(PackageKit::Transaction::Status status)
{
    m_smartSTI->setIcon( KpkIcons::statusIcon(status) );
}

void KpkTransactionTrayIcon::updateMenu(const QList<PackageKit::Transaction*> &tids)
{
    m_menu->clear();
    m_transAction.clear();

    QString text;
    bool refreshCache = true;
//     Transaction *t;
//     for (int i = tids.size() - 1; i >= 0; i--) {
    foreach (Transaction *t, tids ) {
// 	t = tids.at(i);
        QAction *transactionAction = new QAction(this);
        m_transAction[transactionAction] = t;
        if ( t->role().action == Client::ActionRefreshCache )
            refreshCache = false;
        text = KpkStrings::action( t->role().action ) + " " + t->role().terms.join(", ") + " (" + KpkStrings::status( t->status() ) + ")";
        transactionAction->setText(text);
        transactionAction->setIcon( KpkIcons::statusIcon( t->status() ) );
        m_menu->addAction(transactionAction);
    }

    QAction *refreshCacheAction = new QAction(this);
    if (refreshCache && m_act.contains(Client::ActionRefreshCache) ) {
        m_menu->addSeparator();
        refreshCacheAction->setText(i18n("Refresh Packages List"));
        m_menu->addAction( refreshCacheAction );
    }
}

void KpkTransactionTrayIcon::activated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason != QSystemTrayIcon::Context) {
        QList<PackageKit::Transaction*> tids( m_client->getTransactions() );
        if ( tids.size() ) {
            updateMenu( tids );
            m_menu->exec( QCursor::pos() );
        }
        else{
            m_menu->hide();
            m_menu->clear();
            // to avoid warning that the object was deleted in it's event handler
            QTimer::singleShot(1, m_smartSTI, SLOT( hide() ) );
        }
    }
}

#include "KpkTransactionTrayIcon.moc"
