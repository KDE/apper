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

#include "TransactionTrayIcon.h"

#include <KpkStrings.h>
#include <KpkTransaction.h>
#include <KpkIcons.h>
#include <KpkImportance.h>
#include <KpkEnum.h>

#include <KMenu>
#include <KTextBrowser>
#include <KNotification>
#include <KMessageBox>
#include <KLocale>
#include <KWindowSystem>

#include <Solid/PowerManagement>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>

#include <KDebug>

Q_DECLARE_METATYPE(Enum::Restart)

KpkTransactionTrayIcon::KpkTransactionTrayIcon(QObject *parent)
 : KpkAbstractIsRunning(parent),
   m_trayIcon(0),
   m_refreshCacheAction(0),
   m_messagesCount(0),
   m_inhibitCookie(-1)
{
    // Create a new daemon
    m_client = Client::instance();

    connect(m_client, SIGNAL(transactionListChanged(const QList<PackageKit::Transaction*> &)),
            this, SLOT(transactionListChanged(const QList<PackageKit::Transaction*> &)));

    if (Client::instance()->actions() & Enum::RoleRefreshCache) {
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
            this, SLOT(hideIcon()));

    // initiate the restart type
    m_restartType = Enum::RestartNone;

    m_restartAction = new QAction(this);
    connect(m_restartAction, SIGNAL(triggered(bool)),
            this, SLOT(logout()));
}

KpkTransactionTrayIcon::~KpkTransactionTrayIcon()
{
}

void KpkTransactionTrayIcon::checkTransactionList()
{
    // here the menu can be checked whether or not is visible
    // it's only called here so a just started app can show the current transactions
    transactionListChanged(m_client->getTransactionObjectList());
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

void KpkTransactionTrayIcon::createTransactionDialog(PackageKit::Transaction *t)
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
    KpkTransaction *trans = new KpkTransaction(t, KpkTransaction::CloseOnFinish);
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
        // release any cookie that we might have
        suppressSleep(false);

        if (m_messagesCount == 0 &&
            m_restartType == Enum::RestartNone)
        {
            // TODO check if a menu being shown
            // will hide
            hideIcon();
            emit close();
        } else {
            QString toolTip;
            QString iconName;
            QIcon icon;
            if (m_restartType != Enum::RestartNone) {
                toolTip.append(KpkStrings::restartType(m_restartType) + '\n');
                toolTip.append(i18np("Package: %2",
                                     "Packages: %2",
                                     m_restartPackages.size(),
                                     m_restartPackages.join(", ")));
                iconName = KpkIcons::restartIconName(m_restartType);
            }
            if (m_messagesCount) {
                if (!toolTip.isEmpty()) {
                    toolTip.append('\n');
                } else {
                    // in case the restart icon is not set
                    icon = KpkIcons::getPreloadedIcon("kpk-important");
                }
                toolTip.append(i18np("One message from the package manager",
                                     "%1 messages from the package manager",
                                     m_messagesCount));

            }

            if (iconName.isEmpty()) {
                m_trayIcon->setIconByPixmap(icon);
            } else {
                m_trayIcon->setIconByName(iconName);
            }
            m_trayIcon->setToolTipSubTitle(toolTip);
        }
    }
}

void KpkTransactionTrayIcon::setCurrentTransaction(PackageKit::Transaction *transaction)
{
    m_currentTransaction = transaction;

    // Check if the icon is created
    if (m_trayIcon == 0) {
        // Creates our smart icon
        m_trayIcon = new TransactionTrayIcon(this);
        connect(m_trayIcon->contextMenu(), SIGNAL(aboutToShow()),
                this, SLOT(fillMenu()));
        connect(m_trayIcon, SIGNAL(transactionActivated(PackageKit::Transaction *)),
                this, SLOT(createTransactionDialog(PackageKit::Transaction *)));

        // Clear old data
        m_currentRole     = Enum::UnknownRole;
        m_currentStatus   = Enum::UnknownStatus;
        m_currentProgress = 0;
    }

    connect(m_currentTransaction, SIGNAL(changed()),
            this, SLOT(transactionChanged()));
    // AVOID showing messages and restart requires when
    // the user was just simulating an instalation
    // TODO fix yum backend
    Enum::Role role = transaction->role();
    if (role == Enum::RoleInstallPackages ||
        role == Enum::RoleInstallFiles    ||
        role == Enum::RoleRemovePackages  ||
        role == Enum::RoleUpdatePackages  ||
        role == Enum::RoleUpdateSystem) {
        connect(m_currentTransaction, SIGNAL(message(PackageKit::Enum::Message, const QString &)),
                this, SLOT(message(PackageKit::Enum::Message, const QString &)));
        connect(m_currentTransaction, SIGNAL(requireRestart(PackageKit::Enum::Restart, QSharedPointer<PackageKit::Package>)),
                this, SLOT(requireRestart(PackageKit::Enum::Restart, QSharedPointer<PackageKit::Package>)));

        // Don't let the system sleep while doing some sensible actions
        suppressSleep(true, KpkStrings::action(role));
    }
    connect(m_currentTransaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            this, SLOT(finished(PackageKit::Enum::Exit)));

    // update the icon
    transactionChanged();
}

void KpkTransactionTrayIcon::finished(PackageKit::Enum::Exit exit)
{
    // check if the transaction emitted any require restart
    Transaction *transaction = qobject_cast<Transaction*>(sender());
    if (exit == Enum::ExitSuccess && !transaction->property("restartType").isNull()) {
        Enum::Restart type = transaction->property("restartType").value<Enum::Restart>();

        // Create the notification object
        KNotification *notify = new KNotification("RestartRequired");
        QString text("<b>" + i18n("The system update has completed") + "</b>");
        text.append("<br />" + KpkStrings::restartType(type));
        m_restartPackages.removeDuplicates();
        m_restartPackages.sort();
        if (m_restartPackages.size()) {
            text.append("<br />Packages: " + m_restartPackages.join(", "));
        }

        switch (type) {
        case Enum::RestartSystem :
        case Enum::RestartSecuritySystem :
            m_restartAction->setText(i18nc("Restart the computer", "Restart"));
            break;
        case Enum::RestartSession :
        case Enum::RestartSecuritySession :
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

        notify->sendEvent();

        int old = KpkImportance::restartImportance(m_restartType);
        int newer = KpkImportance::restartImportance(type);
        // Check to see which one is more important
        if (newer > old) {
            m_restartType = type;
        }
    }
}

void KpkTransactionTrayIcon::transactionChanged()
{
    if (m_trayIcon == 0) {
        return;
    }

    uint         percentage = m_currentTransaction->percentage();
    Enum::Status status     = m_currentTransaction->status();
    Enum::Role   role       = m_currentTransaction->role();
    QString      toolTip;

    if (m_currentRole != role) {
        m_currentRole = role;
        m_trayIcon->setToolTipTitle(KpkStrings::action(role));
        QString iconName = KpkIcons::actionIconName(role);
        m_trayIcon->setToolTipIconByPixmap(KpkIcons::getPreloadedIcon(iconName));
    }

    if (status != m_currentStatus) {
        // Do not store status here
        // so we can compare on the next 'if'
        QString iconName = KpkIcons::statusIconName(status);
        kDebug() << iconName << KpkIcons::getPreloadedIcon(iconName).isNull() << KpkIcons::getPreloadedIcon(iconName).availableSizes();
        m_trayIcon->setIconByPixmap(KpkIcons::getPreloadedIcon(iconName));
    }

    if (percentage != m_currentProgress || status != m_currentStatus) {
        m_currentProgress = percentage;
        m_currentStatus   = status;
        if (percentage && percentage <= 100) {
            toolTip = i18n("%1% - %2", percentage, KpkStrings::status(status));
        } else {
            toolTip = i18n("%1", KpkStrings::status(status));
        }
        m_trayIcon->setToolTipSubTitle(toolTip);
    }
}

void KpkTransactionTrayIcon::message(PackageKit::Enum::Message type, const QString &message)
{
    m_messagesCount++;
    m_messages.append("<p><h3>");
    m_messages.append(QDateTime::currentDateTime().toString("hh:mm:ss"));
    m_messages.append(" - ");
    m_messages.append(KpkStrings::message(type));
    m_messages.append("</h3>");
    m_messages.append(message);
    m_messages.append("</p>");
}

void KpkTransactionTrayIcon::showMessages()
{
    if (m_messagesCount) {
        increaseRunning();
        KDialog *dialog = new KDialog;
        dialog->setCaption(i18n("Package Manager Messages"));
        dialog->setButtons(KDialog::Close);

        KTextBrowser *browser = new KTextBrowser(dialog);
        browser->setOpenExternalLinks(true);
        browser->setHtml(m_messages);
        dialog->setMainWidget(browser);
        dialog->setWindowIcon(KIcon("view-pim-notes"));
        dialog->setInitialSize(QSize(600, 400));
        m_messagesCount = 0;
        m_messages.clear();

        connect(dialog, SIGNAL(finished()), this, SLOT(decreaseRunning()));
        // We call this so that the icon can hide if there
        // are no more messages and transactions running
        connect(dialog, SIGNAL(finished()), this, SLOT(checkTransactionList()));
        dialog->show();
    }
}

void KpkTransactionTrayIcon::fillMenu()
{
    // this function checks if we need to display the menu
    if (!isRunning()) {
        hideIcon();
        return;
    }

    KMenu *contextMenu = qobject_cast<KMenu*>(sender());
    QString text;
    bool refreshCache = true;

    contextMenu->clear();

    QList<PackageKit::Transaction*> tids = m_client->getTransactionObjectList();

    contextMenu->addTitle(KIcon("applications-other"), i18n("Transactions"));
    foreach (Transaction *t, tids) {
        QAction *transactionAction = new QAction(this);
        // We use the tid since the pointer might get deleted
        // as it was some times
        transactionAction->setData(t->tid());

        if (t->role() == Enum::RoleRefreshCache) {
            refreshCache = false;
        }

        text = KpkStrings::action(t->role())
               + " (" + KpkStrings::status(t->status()) + ')';
        transactionAction->setText(text);
        transactionAction->setIcon(KpkIcons::statusIcon(t->status()));
        contextMenu->addAction(transactionAction);
    }

    contextMenu->addSeparator();
    if (refreshCache && m_refreshCacheAction) {
        contextMenu->addAction(m_refreshCacheAction);
    }
    if (m_restartType != Enum::RestartNone) {
        contextMenu->addAction(m_restartAction);
    }
    if (m_messagesCount) {
        contextMenu->addAction(m_messagesAction);
    }
    contextMenu->addAction(m_hideAction);
}

void KpkTransactionTrayIcon::requireRestart(PackageKit::Enum::Restart type, QSharedPointer<PackageKit::Package> pkg)
{
    Transaction *transaction = qobject_cast<Transaction*>(sender());
    if (transaction->property("restartType").isNull()) {
        transaction->setProperty("restartType", qVariantFromValue(type));
    } else {
        Enum::Restart oldType = transaction->property("restartType").value<Enum::Restart>();
        int old = KpkImportance::restartImportance(oldType);
        int newer = KpkImportance::restartImportance(type);
        // Check to see which one is more important
        if (newer > old) {
            transaction->setProperty("restartType", qVariantFromValue(type));
        }
    }

    if (!pkg->name().isEmpty()) {
        m_restartPackages << pkg->name();
    }
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

void KpkTransactionTrayIcon::hideIcon()
{
    // Reset things as the user don't want to see it
    if (m_trayIcon) {
        m_trayIcon->deleteLater();
    }
    m_trayIcon = 0;
    m_messages.clear();
    m_messagesCount = 0;
    m_restartType = Enum::RestartNone;
}

bool KpkTransactionTrayIcon::isRunning()
{
    return KpkAbstractIsRunning::isRunning() ||
           !Client::instance()->getTransactionList().isEmpty() ||
           m_messagesCount ||
           m_restartType != Enum::RestartNone;
}

    
void KpkTransactionTrayIcon::suppressSleep(bool enable, const QString &reason)
{
    if (enable) {
        if (m_inhibitCookie == -1) {
            kDebug() << "Disabling powermanagement sleep";
            m_inhibitCookie = Solid::PowerManagement::beginSuppressingSleep(reason);
            if (m_inhibitCookie == -1) {
                kDebug() << "Sleep suppression denied!";
            }
        }
    } else {
        kDebug() << "Enabling powermanagement sleep";
        if (m_inhibitCookie == -1) {
            if ( ! Solid::PowerManagement::stopSuppressingSleep(m_inhibitCookie)) {
                kDebug() << "Enable failed: invalid cookie.";
            }
        }
    }
}

#include "KpkTransactionTrayIcon.moc"
