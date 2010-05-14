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

#include "KpkTransactionTrayIcon.h"

#include <KpkStrings.h>
#include <KpkTransaction.h>
#include <KpkIcons.h>
#include <KpkImportance.h>
#include <KpkEnum.h>

#include <KMenu>
#include <QTreeView>
#include <QStandardItemModel>
#include <KNotification>
#include <KMessageBox>
#include <KActionCollection>
#include <KLocale>
#include <KWindowSystem>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>

#include <KDebug>

Q_DECLARE_METATYPE(Transaction*)

KpkTransactionTrayIcon::KpkTransactionTrayIcon(QObject *parent)
 : KpkAbstractIsRunning(parent),
   m_smartSTI(0),
   m_refreshCacheAction(0)
{
    // Create a new daemon
    m_client = Client::instance();
    m_act = Client::instance()->actions();

    connect(m_client, SIGNAL(transactionListChanged(const QList<PackageKit::Transaction*> &)),
            this, SLOT(transactionListChanged(const QList<PackageKit::Transaction*> &)));

    if (m_act & Enum::RoleRefreshCache) {
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

    // initiate the restart type
    m_restartType = Enum::RestartNone;

    m_restartAction = new QAction(this);
    connect(m_restartAction, SIGNAL(triggered(bool)),
            this, SLOT(logout()));
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
    Transaction *t = m_client->refreshCache(true);
    if (t->error()) {
        KMessageBox::sorry(0, KpkStrings::daemonError(t->error()));
    } else {
        createTransactionDialog(t);
    }
    decreaseRunning();
}

void KpkTransactionTrayIcon::triggered(QAction *action)
{
    // Check to see if we set a Transaction->tid() in action
    if (!action->data().isNull()) {
        // we need to find if the action clicked has already a dialog
        createTransactionDialog(action->data().value<Transaction *>());
    }
}

void KpkTransactionTrayIcon::createTransactionDialog(Transaction *t)
{
    // if we don't have a dialog already displaying
    // our transaction let's create one
    // BUT first we need to get the transaction pointer,
    // since it might be already deleted if the transaction
    // finished
    if (m_transDialogs.contains(t->tid())) {
        KWindowSystem::forceActiveWindow(m_transDialogs[t->tid()]->winId());
        return;
    }

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
//     kDebug() << tids.size();
    if (tids.size()) {
        setCurrentTransaction(tids.first());
    } else {
        if (m_messages.isEmpty() &&
            m_restartType == Enum::RestartNone)
        {
            removeStatusNotifierItem();
            emit close();
        } else {
            QString toolTip;
            if (m_restartType != Enum::RestartNone) {
                toolTip.append(KpkStrings::restartType(m_restartType) + '\n');
                toolTip.append(i18np("Package: %2",
                                     "Packages: %2",
                                     m_restartPackages.size(),
                                     m_restartPackages.join(", ")));
                m_smartSTI->setIconByPixmap(KpkIcons::restartIcon(m_restartType));
            }
            if (m_messages.size()) {
                if (!toolTip.isEmpty()) {
                    toolTip.append('\n');
                } else {
                    // in case the restart icon is not set
                    m_smartSTI->setIconByPixmap(KpkIcons::getIcon("kpk-important"));
                }
                toolTip.append(i18np("One message from the package manager",
                                     "%1 messages from the package manager",
                                     m_messages.size()));

            }
            m_smartSTI->setToolTip(KpkIcons::restartIcon(m_restartType), toolTip, QString());
        }
    }
}

void KpkTransactionTrayIcon::setCurrentTransaction(PackageKit::Transaction *transaction)
{
    m_currentTransaction = transaction;

    // Creates the smart icon
    if (!m_smartSTI) {
        m_smartSTI = new KStatusNotifierItem(this);
        m_smartSTI->setCategory(KStatusNotifierItem::SystemServices);
        m_smartSTI->setStatus(KStatusNotifierItem::Active);

        // Remove the EXIT button
        KActionCollection *actions = m_smartSTI->actionCollection();
        actions->removeAction(actions->action(KStandardAction::name(KStandardAction::Quit)));

        // Creates our transaction menu
        m_menu = new KMenu;
        m_menu->addTitle(KIcon("applications-other"), i18n("Transactions"));
        connect(m_menu, SIGNAL(triggered(QAction *)),
                this, SLOT(triggered(QAction *)));

        // Setup a menu with some actions
        m_smartSTI->setContextMenu(m_menu);
        connect(m_smartSTI, SIGNAL(activateRequested(bool, const QPoint &)),
                this, SLOT(showUpdates()));
    }

    // update icon
    transactionChanged();
    connect(m_currentTransaction, SIGNAL(changed()),
            this, SLOT(transactionChanged()));
    connect(m_currentTransaction, SIGNAL(message(PackageKit::Enum::Message, const QString &)),
            this, SLOT(message(PackageKit::Enum::Message, const QString &)));
    connect(m_currentTransaction, SIGNAL(requireRestart(PackageKit::Enum::Restart, QSharedPointer<PackageKit::Package>)),
            this, SLOT(requireRestart(PackageKit::Enum::Restart, QSharedPointer<PackageKit::Package>)));
    connect(m_currentTransaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            this, SLOT(finished()));
}

void KpkTransactionTrayIcon::finished()
{
    // check if the transaction emitted any require restart
    if (m_restartType != Enum::RestartNone) {
        increaseRunning();
        Enum::Restart type = m_restartType;

        // Create the notification object
        KNotification *notify = new KNotification("RestartRequired", 0, KNotification::Persistent);
        QString text("<b>" + i18n("The system update has completed") + "</b>");
        text.append("<br />" + KpkStrings::restartType(type));
        m_restartPackages.removeDuplicates();
        m_restartPackages.sort();
        if (m_restartPackages.size()) {
            text.append("<br />Packages: " + m_restartPackages.join(", "));
        }

        QStringList actions;
        switch (type) {
        case Enum::RestartSystem :
        case Enum::RestartSecuritySystem :
            actions << i18nc("Restart the computer", "Restart");
            actions << i18n("Not now");
            m_restartAction->setText(i18nc("Restart the computer", "Restart"));
            break;
        case Enum::RestartSession :
        case Enum::RestartSecuritySession :
            actions << i18n("Logout");
            actions << i18n("Not now");
            m_restartAction->setText(i18n("Logout"));
            break;
        case Enum::RestartApplication :
            // What do we do in restart application?
            // we don't even know what application is
            // SHOULD we check for pkg and see the installed
            // files and try to match some running process?
            break;
        default :
            // We do not set a restart type cause it will probably
            // be already none, if not we will still want that restart type.
            return;
        }

        notify->setPixmap(KpkIcons::restartIcon(type).pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
        m_restartAction->setIcon(KpkIcons::restartIcon(type));
        notify->setText(text);
        notify->setActions(actions);
        connect(notify, SIGNAL(activated(uint)),
                this, SLOT(restartActivated(uint)));
        connect(notify, SIGNAL(closed()),
                this, SLOT(decreaseRunning()));

        notify->sendEvent();

        // Reset the restart type
        m_restartType = Enum::RestartNone;
    }
}

void KpkTransactionTrayIcon::transactionChanged()
{
    uint percentage     = m_currentTransaction->percentage();
    Enum::Status status = m_currentTransaction->status();
    QString toolTip;

    if (percentage && percentage <= 100) {
        toolTip = i18n("%1% - %2", percentage, KpkStrings::status(status));
    } else {
        toolTip = i18n("%1", KpkStrings::status(status));
    }

    m_smartSTI->setToolTip(KpkStrings::action(m_currentTransaction->role()), toolTip, QString());
    m_smartSTI->setIconByPixmap(KpkIcons::statusIcon(status));
}

void KpkTransactionTrayIcon::message(PackageKit::Enum::Message type, const QString &message)
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
        dialog->setWindowIcon(KIcon("view-pim-notes"));

        QStandardItemModel *model = new QStandardItemModel(this);
        tree->setModel(model);

        model->setHorizontalHeaderLabels(QStringList() << i18n("Message") << i18n("Details"));
        while (!m_messages.isEmpty()) {
            QPair<Enum::Message, QString> pair = m_messages.takeFirst();
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
        transactionAction->setData(QVariant::fromValue(t));

        if (t->role() == Enum::RoleRefreshCache) {
            refreshCache = false;
        }

        text = KpkStrings::action(t->role())
               + " (" + KpkStrings::status(t->status()) + ')';
        transactionAction->setText(text);
        transactionAction->setIcon(KpkIcons::statusIcon(t->status()));
        m_menu->addAction(transactionAction);
    }

    m_menu->addSeparator();
    if (refreshCache && m_refreshCacheAction) {
        m_menu->addAction(m_refreshCacheAction);
    }
    if (m_restartType != Enum::RestartNone) {
        m_menu->addAction(m_restartAction);
    }
    if (m_messages.size()) {
        m_menu->addAction(m_messagesAction);
    }
    m_menu->addAction(m_hideAction);
}

void KpkTransactionTrayIcon::activated()
{
    QList<PackageKit::Transaction*> tids(m_client->getTransactions());
    if (tids.size() || isRunning()) {
        updateMenu(tids);
        m_menu->exec(QCursor::pos());
    } else {
        removeStatusNotifierItem();
    }
}

void KpkTransactionTrayIcon::requireRestart(PackageKit::Enum::Restart type, QSharedPointer<PackageKit::Package> pkg)
{
    int old = KpkImportance::restartImportance(m_restartType);
    int newer = KpkImportance::restartImportance(type);
    // Check to see which one is more important
    if (newer > old) {
        m_restartType = type;
    }

    if (!pkg->name().isEmpty()) {
        m_restartPackages << pkg->name();
    }
}

void KpkTransactionTrayIcon::restartActivated(uint action)
{
    if (action == 1) {
        logout();
    }
    // in persistent mode we need to manually close it
    qobject_cast<KNotification*>(sender())->close();
}

void KpkTransactionTrayIcon::logout()
{
    // We call KSM server to restart or logout our system
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.kde.ksmserver",
                                             "/KSMServer",
                                             "org.kde.KSMServerInterface",
                                             QLatin1String("logout"));
    // We are polite
    // KWorkSpace::ShutdownConfirmYes = 1
    message << qVariantFromValue(1);

    if (m_restartType == Enum::RestartSystem) {
        // The restart type was system
        // KWorkSpace::ShutdownTypeReboot = 1
        message << qVariantFromValue(1);
    } else if (m_restartType == Enum::RestartSession) {
        // The restart type was session
        // KWorkSpace::ShutdownTypeLogout = 3
        message << qVariantFromValue(3);
    } else {
        kWarning() << "Unknown restart type:" << m_restartType;
        return;
    }
    // Try now
    // KWorkSpace::ShutdownModeTryNow = 1
    message << qVariantFromValue(1);

    QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        kWarning() << "Message did not receive a reply" << reply;
    }
}

bool KpkTransactionTrayIcon::isRunning()
{
    return KpkAbstractIsRunning::isRunning() ||
           Client::instance()->getTransactions().size() ||
           m_messages.size() ||
           m_restartType != Enum::RestartNone;
}

void KpkTransactionTrayIcon::removeStatusNotifierItem()
{
    if (m_smartSTI) {
        m_smartSTI->deleteLater();
        m_smartSTI = 0;
    }
}

#include "KpkTransactionTrayIcon.moc"
