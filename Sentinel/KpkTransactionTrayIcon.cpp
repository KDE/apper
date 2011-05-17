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

#include "TransactionJob.h"
#include "StatusNotifierItem.h"

#include <KpkStrings.h>
#include <KpkIcons.h>
#include <KpkImportance.h>
#include <KpkEnum.h>

#include <KMenu>
#include <KTextBrowser>
#include <KNotification>
#include <KLocale>
#include <KDialog>
#include <KWindowSystem>

#include <Solid/PowerManagement>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>

#include <KDebug>

#include <Daemon>

Q_DECLARE_METATYPE(Package::Restart)

KpkTransactionTrayIcon::KpkTransactionTrayIcon(QObject *parent)
 : KpkAbstractIsRunning(parent),
   m_messagesSNI(0),
   m_restartSNI(0),
   m_inhibitCookie(-1)
{
    m_transHasJob = false;
    m_tracker = new KUiServerJobTracker(this);

    // keep track of new transactions
    connect(Daemon::global(), SIGNAL(transactionListChanged(const QStringList &)),
            this, SLOT(transactionListChanged(const QStringList &)));

    m_hideAction = new QAction(this);
    m_hideAction->setText(i18n("Hide this icon"));
    connect(m_hideAction, SIGNAL(triggered(bool)),
            this, SLOT(hideMessageIcon()));

    // initiate the restart type
    m_restartType = Package::RestartNone;

    // here we check whether a transaction job should be created or not
    transactionListChanged(Daemon::getTransactions());
}

KpkTransactionTrayIcon::~KpkTransactionTrayIcon()
{
    // release any cookie that we might have
    suppressSleep(false);
}

void KpkTransactionTrayIcon::transactionListChanged(const QStringList &tids)
{
    kDebug() << tids.size();
    if (!tids.isEmpty()) {
        // the first tid is always the tid of a running transaction
        setCurrentTransaction(tids.first());
    } else {
        // There is no current transaction
        m_currentTransaction = 0;

        // release any cookie that we might have
        suppressSleep(true);

        if (m_messages.isEmpty() && m_restartType == Package::RestartNone) {
            // the app can close now
            emit close();
        }
    }
}

void KpkTransactionTrayIcon::setCurrentTransaction(const QString &tid)
{
    // Check if the current transaction is still the same
    if (m_currentTid == tid) {
        if (m_currentTransaction->isCallerActive() || m_transHasJob) {
            // if the caller is active we don't need to check if the transaction
            // has a job watcher Or if the caller is not active but already
            // has a transaction job return
            return;
        }
    }

    // saves the current transaction
    m_currentTid = tid;
    m_restartPackages.clear();
    m_currentTransaction = new Transaction(tid, this);

    Transaction::Role role = m_currentTransaction->role();
    if (role == Transaction::RoleInstallPackages ||
        role == Transaction::RoleInstallFiles    ||
        role == Transaction::RoleRemovePackages  ||
        role == Transaction::RoleUpdatePackages  ||
        role == Transaction::RoleUpdateSystem    ||
        role == Transaction::RoleUpgradeSystem) {
        // AVOID showing messages and restart requires when
        // the user was just simulating an instalation
        // TODO fix yum backend
        connect(m_currentTransaction, SIGNAL(message(PackageKit::Transaction::Message, const QString &)),
                this, SLOT(message(PackageKit::Transaction::Message, const QString &)));
        connect(m_currentTransaction, SIGNAL(requireRestart(PackageKit::Package::Restart, const PackageKit::Package &)),
                this, SLOT(requireRestart(PackageKit::Package::Restart, const PackageKit::Package &)));

        // Don't let the system sleep while doing some sensible actions
        suppressSleep(true, KpkStrings::action(role));
    }
    connect(m_currentTransaction, SIGNAL(changed()),
            this, SLOT(transactionChanged()));
    connect(m_currentTransaction, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
            this, SLOT(finished(PackageKit::Transaction::Exit)));

    m_transHasJob = !m_currentTransaction->isCallerActive();
    if (m_transHasJob) {
        TransactionJob *job = new TransactionJob(m_currentTransaction, this);
        job->start();
        m_tracker->registerJob(job);
    }
}

void KpkTransactionTrayIcon::finished(PackageKit::Transaction::Exit exit)
{
    // check if the transaction emitted any require restart
    Transaction *transaction = qobject_cast<Transaction*>(sender());
    if (exit == Transaction::ExitSuccess && !transaction->property("restartType").isNull()) {
        Package::Restart type = transaction->property("restartType").value<Package::Restart>();

        // Create the notification object
        KNotification *notify = new KNotification("RestartRequired");
        QString text("<b>" + i18n("The system update has completed") + "</b>");
        text.append("<br>" + KpkStrings::restartType(type));
        m_restartPackages.removeDuplicates();
        m_restartPackages.sort();
        if (!m_restartPackages.isEmpty()) {
            text.append("<br>");
            text.append(i18n("Packages: %1", m_restartPackages.join(", ")));
        }

        notify->setPixmap(KpkIcons::restartIcon(type).pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
        notify->setText(text);
        notify->sendEvent();

        if (m_restartSNI == 0) {
            QString iconName = KpkIcons::restartIconName(m_restartType);
            m_restartSNI = new StatusNotifierItem(this);
            m_restartSNI->setToolTip(iconName,
                                     KpkStrings::restartType(m_restartType),
                                     i18np("Package: %2",
                                           "Packages: %2",
                                           m_restartPackages.size(),
                                           m_restartPackages.join(", ")));
            connect(m_messagesSNI, SIGNAL(activateRequested(bool, const QPoint &)),
                    this, SLOT(logout()));
        }

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
    if (!m_transHasJob && !m_currentTransaction->isCallerActive()) {
        TransactionJob *job = new TransactionJob(m_currentTransaction, this);
        job->start();
        m_tracker->registerJob(job);
        m_transHasJob = true;
    }
}

void KpkTransactionTrayIcon::message(PackageKit::Transaction::Message type, const QString &message)
{
    QString html;
    html.append("<p><h3>");
    html.append(QDateTime::currentDateTime().toString("hh:mm:ss"));
    html.append(" - ");
    html.append(KpkStrings::message(type));
    html.append("</h3>");
    html.append(message);
    html.append("</p>");
    m_messages << html;

    if (m_messagesSNI == 0) {
        QIcon icon = KpkIcons::getPreloadedIcon("kpk-important");
        m_messagesSNI = new StatusNotifierItem(this);
        m_messagesSNI->setIconByPixmap(icon);
        m_messagesSNI->setToolTip(icon,
                                  i18n("Software Manager Messages"),
                                  i18np("One message from the software manager",
                                        "%1 messages from the software manager",
                                        m_messages.size()));
        connect(m_messagesSNI, SIGNAL(activateRequested(bool, const QPoint &)),
                this, SLOT(showMessages()));

        // Action for message handling
        QAction *messagesAction = new QAction(this);
        messagesAction->setText(i18n("Show messages"));
        messagesAction->setIcon(KIcon("view-pim-notes"));
        connect(messagesAction, SIGNAL(triggered(bool)),
                this, SLOT(showMessages()));
        m_messagesSNI->contextMenu()->addAction(messagesAction);

        m_messagesSNI->contextMenu()->addSeparator();

        QAction *hideAction = new QAction(this);
        hideAction->setText(i18n("Hide this icon"));
        connect(hideAction, SIGNAL(triggered(bool)),
                m_messagesSNI, SLOT(deleteLater()));
        m_messagesSNI->contextMenu()->addAction(hideAction);
    }
}

void KpkTransactionTrayIcon::showMessages()
{
    if (!m_messages.isEmpty()) {
        increaseRunning();

        KDialog *dialog = new KDialog;

        QString html = m_messages.join("");
        KTextBrowser *browser = new KTextBrowser(dialog);
        browser->setOpenExternalLinks(true);
        browser->setHtml(html);

        dialog->setMainWidget(browser);
        dialog->setWindowIcon(KIcon("view-pim-notes"));
        dialog->setInitialSize(QSize(600, 400));
        dialog->setCaption(i18n("Software Manager Messages"));
        dialog->setButtons(KDialog::Close);
        connect(dialog, SIGNAL(finished()), this, SLOT(decreaseRunning()));
        connect(dialog, SIGNAL(finished()), SLOT(deleteLater()));

        hideMessageIcon();
        dialog->show();
    }
}

void KpkTransactionTrayIcon::fillMenu()
{
    // this function checks if we need to display the menu
    if (!isRunning()) {
        hideMessageIcon();
        return;
    }

    KMenu *contextMenu = qobject_cast<KMenu*>(sender());
    QString text;
    bool refreshCache = true;

    contextMenu->clear();

    QList<PackageKit::Transaction*> tids = Daemon::getTransactionsObj(this);

    contextMenu->addTitle(KIcon("applications-other"), i18n("Transactions"));
    foreach (Transaction *t, tids) {
        QAction *transactionAction = new QAction(this);
        // We use the tid since the pointer might get deleted
        // as it was some times
        transactionAction->setData(t->tid());

        if (t->role() == Transaction::RoleRefreshCache) {
            refreshCache = false;
        }

        text = KpkStrings::action(t->role())
               + " (" + KpkStrings::status(t->status()) + ')';
        transactionAction->setText(text);
        transactionAction->setIcon(KpkIcons::statusIcon(t->status()));
        contextMenu->addAction(transactionAction);
    }

    contextMenu->addSeparator();
//     if (refreshCache && m_refreshCacheAction) {
//         contextMenu->addAction(m_refreshCacheAction);
//     }
    if (m_restartType != Package::RestartNone) {
//         contextMenu->addAction(m_restartAction);
    }
    if (!m_messages.isEmpty()) {
        contextMenu->addAction(m_messagesAction);
    }
    contextMenu->addAction(m_hideAction);
}

void KpkTransactionTrayIcon::requireRestart(PackageKit::Package::Restart type, const Package &pkg)
{
    if (m_currentTransaction->property("restartType").isNull()) {
        m_currentTransaction->setProperty("restartType", qVariantFromValue(type));
    } else {
        Package::Restart oldType = m_currentTransaction->property("restartType").value<Package::Restart>();
        int old = KpkImportance::restartImportance(oldType);
        int newer = KpkImportance::restartImportance(type);
        // Check to see which one is more important
        if (newer > old) {
            m_currentTransaction->setProperty("restartType", qVariantFromValue(type));
        }
    }

    if (!pkg.name().isEmpty()) {
        m_restartPackages << pkg.name();
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

    if (m_restartType == Package::RestartSystem) {
        // The restart type was system
        // KWorkSpace::ShutdownTypeReboot = 1
        message << qVariantFromValue(1);
    } else if (m_restartType == Package::RestartSession) {
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

void KpkTransactionTrayIcon::hideMessageIcon()
{
    // Reset things as the user don't want to see it
    if (m_messagesSNI) {
        m_messagesSNI->deleteLater();
        m_messagesSNI = 0;
    }
    m_messages.clear();
    emit close();
}

bool KpkTransactionTrayIcon::isRunning()
{
    return KpkAbstractIsRunning::isRunning() ||
           m_currentTransaction ||
          !m_messages.isEmpty() ||
           m_restartType != Package::RestartNone;
}

void KpkTransactionTrayIcon::suppressSleep(bool enable, const QString &reason)
{
    if (enable) {
        if (m_inhibitCookie == -1) {
            kDebug() << "Begin Suppressing Sleep";
            m_inhibitCookie = Solid::PowerManagement::beginSuppressingSleep(reason);
            if (m_inhibitCookie == -1) {
                kDebug() << "Sleep suppression denied!";
            }
        }
    } else if (m_inhibitCookie != -1) {
        kDebug() << "Stop Suppressing Sleep";
        if (!Solid::PowerManagement::stopSuppressingSleep(m_inhibitCookie)) {
            kDebug() << "Stop failed: invalid cookie.";
        }
        m_inhibitCookie = -1;
    }
}

#include "KpkTransactionTrayIcon.moc"
