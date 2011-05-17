/***************************************************************************
 *   Copyright (C) 2008-2010 by Daniel Nicoletti                           *
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

#include <QTimer>

#include <KpkStrings.h>
#include <KpkIcons.h>
#include <KLocale>

#include <KNotification>
#include <KIcon>
#include <KMessageBox>
#include <KDebug>

#include <Daemon>

Q_DECLARE_METATYPE(PackageKit::Transaction::Error)

KpkTransactionWatcher::KpkTransactionWatcher(QObject *parent)
 : KpkAbstractIsRunning(parent)
{
}

KpkTransactionWatcher::~KpkTransactionWatcher()
{
}

void KpkTransactionWatcher::watchTransaction(const QString &tid, bool interactive)
{
    foreach (const Transaction *trans, m_hiddenTransactions) {
        if (trans->tid() == tid) {
            // Oops we are already watching this one
//             kDebug() << "Oops we are already watching this one" << tid;
            return;
        }
    }

    foreach (const QString &transId, Daemon::getTransactions()) {
        if (transId == tid) {
            // found it let's start watching
            Transaction *trans = new Transaction(transId, this);
//             kDebug() << "found it let's start watching" << tid;
            m_hiddenTransactions.append(trans);
            trans->setProperty("interactive", QVariant(interactive));
            connect(trans, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
                    this, SLOT(finished()));
            connect(trans, SIGNAL(errorCode(PackageKit::Transaction::Error, const QString &)),
                    this, SLOT(errorCode(PackageKit::Transaction::Error, const QString &)));
            break;
        }
    }
}

void KpkTransactionWatcher::removeTransactionWatcher(const QString &tid)
{
    foreach (Transaction *trans, m_hiddenTransactions) {
        if (trans->tid() == tid) {
            // Found it, let's remove
//             kDebug() << "found it let's remove" << tid;
            m_hiddenTransactions.removeOne(trans);
            // disconnect to not show any notification
            disconnect(trans, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
                    this, SLOT(finished()));
            disconnect(trans, SIGNAL(errorCode(PackageKit::Transaction::Error, const QString &)),
                    this, SLOT(errorCode(PackageKit::Transaction::Error, const QString &)));
            break;
        }
    }
}

void KpkTransactionWatcher::finished()
{
    m_hiddenTransactions.removeOne(qobject_cast<Transaction*>(sender()));
    // TODO if the transaction took too long to finish warn the user
}

void KpkTransactionWatcher::errorCode(PackageKit::Transaction::Error err, const QString &details)
{
    increaseRunning();
    KNotification *notify;
    if (sender()->property("interactive").toBool() == true) {
        notify = new KNotification("TransactionError", 0, KNotification::Persistent);
    } else {
        notify = new KNotification("TransactionError");
    }
    notify->setText("<b>"+KpkStrings::error(err)+"</b><br />"+KpkStrings::errorMessage(err));
    notify->setProperty("ErrorType", QVariant::fromValue(err));
    notify->setProperty("Details", details);
    QStringList actions;
    actions << i18n("Details") << i18n("Ignore");
    notify->setActions(actions);
    notify->setPixmap(KIcon("dialog-error").pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
    connect(notify, SIGNAL(activated(uint)),
            this, SLOT(errorActivated(uint)));
    connect(notify, SIGNAL(closed()),
                this, SLOT(decreaseRunning()));
    notify->sendEvent();
}

void KpkTransactionWatcher::errorActivated(uint action)
{
    KNotification *notify = qobject_cast<KNotification*>(sender());

    // if the user clicked "Details"
    if (action == 1) {
        // the notify object gets deleted when not in persistant
        // mode so pass it to another function
        QTimer *t = new QTimer(this);
        connect(t, SIGNAL(timeout()), this, SLOT(showError()));
        t->setProperty("ErrorType",   notify->property("ErrorType"));
        t->setProperty("Details",     notify->property("Details"));
        t->start();
    }

    notify->close();
}

void KpkTransactionWatcher::showError()
{
    increaseRunning();
    Transaction::Error error;
    QString details;
    error = sender()->property("ErrorType").value<Transaction::Error>();
    details = sender()->property("Details").toString();
    KMessageBox::detailedSorry(0,
                               KpkStrings::errorMessage(error),
                               QString(details).replace('\n', "<br />"),
                               KpkStrings::error(error),
                               KMessageBox::Notify);
    decreaseRunning();
    sender()->deleteLater();
}

#include "KpkTransactionWatcher.moc"
