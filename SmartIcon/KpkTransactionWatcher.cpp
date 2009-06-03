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

#include "KpkTransactionWatcher.h"

#include <KpkStrings.h>

#include <KNotification>
#include <KIcon>
#include <KDebug>

KpkTransactionWatcher::KpkTransactionWatcher(QObject *parent)
 : KpkAbstractIsRunning(parent)
{
}

KpkTransactionWatcher::~KpkTransactionWatcher()
{
}

void KpkTransactionWatcher::watchTransaction(const QString &tid)
{
    foreach(Transaction *trans, m_hiddenTransactions) {
        if (trans->tid() == tid) {
            // Oops we are already watching this one
            kDebug() << "Oops we are already watching this one" << tid;
            return;
        }
    }
    foreach(Transaction *trans, Client::instance()->getTransactions()) {
        if (trans->tid() == tid) {
            // found it let's start watching
            kDebug() << "found it let's start watching" << tid;
            m_hiddenTransactions.append(trans);
            connect(trans, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                    this, SLOT(finished(PackageKit::Transaction::ExitStatus, uint)));
            connect(trans, SIGNAL(errorCode(PackageKit::Client::ErrorType, const QString &)),
                    this, SLOT(errorCode(PackageKit::Client::ErrorType, const QString &)));
            break;
        }
    }
}

void KpkTransactionWatcher::removeTransactionWatcher(const QString &tid)
{
    foreach(Transaction *trans, m_hiddenTransactions) {
        if (trans->tid() == tid) {
            // Found it, let's remove
            kDebug() << "found it let's remove" << tid;
            m_hiddenTransactions.removeOne(trans);
            // disconnect to not show any notification
            trans->disconnect();
            break;
        }
    }
}

void KpkTransactionWatcher::finished(PackageKit::Transaction::ExitStatus status, uint time)
{
    Q_UNUSED(status)
    Q_UNUSED(time)
    m_hiddenTransactions.removeOne(qobject_cast<Transaction*>(sender()));
    // TODO if the transaction took too long to finish warn the user
}

void KpkTransactionWatcher::errorCode(PackageKit::Client::ErrorType err, const QString &details)
{
    Q_UNUSED(details)
    // TODO add a details button to show details in a message box
    // do not forget to increase count ^^
    KNotification* errorNotification = new KNotification("TransactionError");
    errorNotification->setFlags(KNotification::Persistent);
    errorNotification->setText("<b>"+KpkStrings::error(err)+"</b><br />"+KpkStrings::errorMessage(err));
    KIcon icon("dialog-error");
    // Use QSize for proper icon
    errorNotification->setPixmap(icon.pixmap(QSize(128, 128)));
    errorNotification->sendEvent();
}

// //FIXME: Implement the proper dbus calls to restart things.
// void KpkTransactionWatcher::showRestartMessage(PackageKit::Client::RestartType type, const QString &details)
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

#include "KpkTransactionWatcher.moc"
