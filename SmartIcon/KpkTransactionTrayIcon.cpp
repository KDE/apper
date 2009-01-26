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
#include <KNotification>

#include <KDebug>

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
        KpkTransaction *transaction = new KpkTransaction(m_transAction[action]);
        transaction->show();
    }
    else {
        if ( Transaction *t = m_client->refreshCache(true) ) {
            KpkTransaction *frm = new KpkTransaction(t);
            frm->show();
        }
        else
            KMessageBox::error( m_menu, i18n("Authentication failed"), i18n("KPackageKit"));
    }
}

void KpkTransactionTrayIcon::showTransactionError(PackageKit::Client::ErrorType err, const QString &details)
{
    kDebug() << "Transaction Error:" << err << details;
    KNotification* errorNotification = new KNotification("TransactionError");
    errorNotification->setFlags(KNotification::Persistent);
    errorNotification->setText("<b>"+KpkStrings::error(err)+"</b><br />"+KpkStrings::errorMessage(err));
    KIcon icon("dialog-error");
    // Use QSize for proper icon
    errorNotification->setPixmap(icon.pixmap(QSize(128, 128)));
    errorNotification->sendEvent();
}

//FIXME: Implement the proper dbus calls to restart things.
void KpkTransactionTrayIcon::showRestartMessage(PackageKit::Client::RestartType type, const QString &details)
{
    if (type==Client::RestartNone)
        return;
    KNotification* notify = new KNotification("RestartRequired");
    QString text;
    QStringList events;
    KIcon icon;
    switch(type) {
        case Client::RestartApplication:
            text = i18n("Restart the application for system changes to take effect.");
            //FIXME: Need a way to detect which program it is
            icon = KIcon("window-close");
            break;
        case Client::RestartSession:
            text = i18n("You must logout and log back in for system changes to take effect.");
            icon = KIcon("system-restart"); //FIXME: find the logout icon
            break;
        case Client::RestartSystem:
            text = i18n("Please restart your computer for system changes to take effect.");
            icon = KIcon("system-restart");
            break;
        case Client::RestartNone:
        case Client::UnknownRestartType:
            return;
    }
    // Use QSize for proper icon
    notify->setPixmap(icon.pixmap(QSize(128, 128)));
    notify->setText("<b>"+text+"</b><br />"+details);
    notify->sendEvent();
}

void KpkTransactionTrayIcon::transactionListChanged(const QList<PackageKit::Transaction*> &tids)
{
    if (tids.size()) {
        m_smartSTI->setIcon( KpkIcons::statusIcon( tids.first()->status() ) );
        m_smartSTI->show();
        connect( tids.first(), SIGNAL( statusChanged(PackageKit::Transaction::Status) ),
                  this, SLOT( currentStatusChanged(PackageKit::Transaction::Status) ) );
        connect( tids.first(), SIGNAL( errorCode( PackageKit::Client::ErrorType, const QString) ),
                  this, SLOT( showTransactionError(PackageKit::Client::ErrorType, const QString) ));
        connect( tids.first(), SIGNAL( requireRestart(PackageKit::Client::RestartType, const QString) ),
                  this, SLOT( showRestartMessage(PackageKit::Client::RestartType, const QString) ));
        updateMenu(tids);
    } else {
        m_menu->hide();
        m_menu->clear();
        QTimer::singleShot(0, m_smartSTI, SLOT( hide() ) );
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
            //QTimer::singleShot(1, m_smartSTI, SLOT( hide() ) );
        }
    }
}

#include "KpkTransactionTrayIcon.moc"
