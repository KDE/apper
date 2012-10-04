/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
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

#include "TransactionWatcher.h"

#include "TransactionJob.h"
#include "StatusNotifierItem.h"

#include <PkStrings.h>
#include <PkIcons.h>
#include <PackageImportance.h>

#include <KMenu>
#include <KTextBrowser>
#include <KNotification>
#include <KLocale>
#include <KDialog>
#include <KWindowSystem>
#include <KMessageBox>

#include <kworkspace/kworkspace.h>

#include <Solid/PowerManagement>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>

#include <KDebug>

#include <Daemon>

Q_DECLARE_METATYPE(PackageUpdateDetails::Restart)
Q_DECLARE_METATYPE(Transaction::Error)

TransactionWatcher::TransactionWatcher(QObject *parent) :
    AbstractIsRunning(parent),
    m_messagesSNI(0),
    m_restartSNI(0),
    m_inhibitCookie(-1)
{
    m_tracker = new KUiServerJobTracker(this);

    // keep track of new transactions
    connect(Daemon::global(), SIGNAL(transactionListChanged(QStringList)),
            this, SLOT(transactionListChanged(QStringList)));

    m_hideAction = new QAction(this);
    m_hideAction->setText(i18n("Hide this icon"));
    connect(m_hideAction, SIGNAL(triggered(bool)),
            this, SLOT(hideMessageIcon()));

    // initiate the restart type
    m_restartType = PackageUpdateDetails::RestartNone;

    // here we check whether a transaction job should be created or not
    QList<QDBusObjectPath> paths = Daemon::getTransactionList();
    QStringList tids;
    foreach (const QDBusObjectPath &path, paths) {
        tids << path.path();
    }
    transactionListChanged(tids);
}

TransactionWatcher::~TransactionWatcher()
{
    // release any cookie that we might have
    suppressSleep(false);
}

void TransactionWatcher::transactionListChanged(const QStringList &tids)
{
    kDebug() << tids.size();
    if (!tids.isEmpty()) {
        foreach (const QString &tid, tids) {
            watchTransaction(QDBusObjectPath(tid), false);
        }
    } else {
        // There is no current transaction
        m_transactions.clear();
        m_transactionJob.clear();

        // release any cookie that we might have
        suppressSleep(false);

        if (m_messages.isEmpty() && m_restartType == PackageUpdateDetails::RestartNone) {
            // the app can close now
            emit close();
        }
    }
}

void TransactionWatcher::watchTransaction(const QDBusObjectPath &tid, bool interactive)
{
    Transaction *transaction;
    if (!m_transactions.contains(tid)) {
        // Check if the current transaction is still the same
        transaction = new Transaction(tid, this);
        if (transaction->error()) {
            qWarning() << "Could not create a transaction for the path:" << tid.path();
            delete transaction;
            return;
        }

        // Store the transaction id
        m_transactions[tid] = transaction;

        Transaction::Role role = transaction->role();
        if (role == Transaction::RoleInstallPackages ||
                role == Transaction::RoleInstallFiles    ||
                role == Transaction::RoleRemovePackages  ||
                role == Transaction::RoleUpdatePackages  ||
                role == Transaction::RoleUpgradeSystem) {
            // AVOID showing messages and restart requires when
            // the user was just simulating an instalation
            // TODO fix yum backend
            connect(transaction, SIGNAL(message(PackageKit::Transaction::Message,QString)),
                    this, SLOT(message(PackageKit::Transaction::Message,QString)));
            connect(transaction, SIGNAL(requireRestart(PackageKit::PackageUpdateDetails::Restart,PackageKit::Package)),
                    this, SLOT(requireRestart(PackageKit::PackageUpdateDetails::Restart,PackageKit::Package)));

            // Don't let the system sleep while doing some sensible actions
            suppressSleep(true, PkStrings::action(role));
        }
        connect(transaction, SIGNAL(changed()), this, SLOT(transactionChanged()));
        connect(transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                this, SLOT(finished(PackageKit::Transaction::Exit)));
    } else {
        transaction = m_transactions[tid];
    }

    // force the firs changed or create a TransactionJob
    transactionChanged(transaction, interactive);
}

void TransactionWatcher::finished(PackageKit::Transaction::Exit exit)
{
    // check if the transaction emitted any require restart
    Transaction *transaction = qobject_cast<Transaction*>(sender());
    QDBusObjectPath tid = transaction->tid();
    disconnect(transaction, SIGNAL(changed()), this, SLOT(transactionChanged()));
    m_transactions.remove(tid);
    m_transactionJob.remove(tid);

    if (exit == Transaction::ExitSuccess && !transaction->property("restartType").isNull()) {
        PackageUpdateDetails::Restart type = transaction->property("restartType").value<PackageUpdateDetails::Restart>();
        QStringList restartPackages = transaction->property("restartPackages").toStringList();

        // Create the notification about this transaction
        KNotification *notify = new KNotification("RestartRequired");
        QString text("<b>" + i18n("The system update has completed") + "</b>");
        text.append("<br>" + PkStrings::restartType(type));
        restartPackages.removeDuplicates();
        restartPackages.sort();
        if (!restartPackages.isEmpty()) {
            text.append("<br>");
            text.append(i18n("Packages: %1", restartPackages.join(QLatin1String(", "))));
        }
        notify->setPixmap(PkIcons::restartIcon(type).pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
        notify->setText(text);
        notify->sendEvent();

        if (m_restartSNI == 0) {
            m_restartSNI = new StatusNotifierItem(this);
            connect(m_restartSNI, SIGNAL(activateRequested(bool,QPoint)),
                    this, SLOT(logout()));
            // Right click shows HIDE action
            QAction *action;
            action = m_restartSNI->contextMenu()->addAction(i18n("Hide"));
            connect(action, SIGNAL(triggered(bool)),
                    this, SLOT(hideRestartIcon()));
        }

        // Now check if old transactions had a higher restart importance
        int old = PackageImportance::restartImportance(m_restartType);
        int newer = PackageImportance::restartImportance(type);
        // Check to see which one is more important
        if (newer > old) {
            m_restartType = type;
            // The restart type changed let's update the Icon
            QString iconName;
            QString subtitle;
            if (!restartPackages.isEmpty()) {
                subtitle = i18np("Package: %2",
                                 "Packages: %2",
                                 restartPackages.size(),
                                 restartPackages.join(QLatin1String(", ")));
            }
            iconName = PkIcons::restartIconName(m_restartType);
            m_restartSNI->setToolTip(iconName,
                                     PkStrings::restartType(m_restartType),
                                     subtitle);
            m_restartSNI->setIconByName(iconName);
        }
    }
}

void TransactionWatcher::transactionChanged(Transaction *transaction, bool interactive)
{
    if (!transaction) {
        transaction = qobject_cast<Transaction*>(sender());
    }

    QDBusObjectPath tid = transaction->tid();
    if (!interactive) {
        interactive = !transaction->isCallerActive();
    }

    // If the
    if (!m_transactionJob.contains(tid) && interactive) {
        TransactionJob *job = new TransactionJob(transaction, this);
        connect(transaction, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
                this, SLOT(errorCode(PackageKit::Transaction::Error,QString)));
        job->start();
        m_tracker->registerJob(job);
        m_transactionJob[tid] = job;
    }
}

void TransactionWatcher::message(PackageKit::Transaction::Message type, const QString &message)
{
    QString html;
    html.append("<p><h3>");
    html.append(QDateTime::currentDateTime().toString("hh:mm:ss"));
    html.append(" - ");
    html.append(PkStrings::message(type));
    html.append("</h3>");
    html.append(message);
    html.append("</p>");
    m_messages << html;

    if (m_messagesSNI == 0) {
        QIcon icon = PkIcons::getPreloadedIcon("kpk-important");
        m_messagesSNI = new StatusNotifierItem(this);
        m_messagesSNI->setIconByPixmap(icon);
        // Seems that this break in the restart icon
        m_messagesSNI->setAssociatedWidget(m_messagesSNI->contextMenu());
        m_messagesSNI->setToolTip(icon,
                                  i18n("Software Manager Messages"),
                                  i18np("One message from the software manager",
                                        "%1 messages from the software manager",
                                        m_messages.size()));
        connect(m_messagesSNI, SIGNAL(activateRequested(bool,QPoint)),
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

void TransactionWatcher::showMessages()
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

void TransactionWatcher::errorCode(PackageKit::Transaction::Error err, const QString &details)
{
    increaseRunning();

    KNotification *notify;
    notify = new KNotification("TransactionError", 0, KNotification::Persistent);
    notify->setText("<b>"+PkStrings::error(err)+"</b><br>"+PkStrings::errorMessage(err));
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

void TransactionWatcher::errorActivated(uint action)
{
    KNotification *notify = qobject_cast<KNotification*>(sender());

    // if the user clicked "Details"
    if (action == 1) {
        Transaction::Error error = notify->property("ErrorType").value<Transaction::Error>();
        QString details = notify->property("Details").toString();
        KMessageBox::detailedSorry(0,
                                   PkStrings::errorMessage(error),
                                   details.replace('\n', "<br />"),
                                   PkStrings::error(error),
                                   KMessageBox::Notify);
    }

    notify->close();
}

void TransactionWatcher::requireRestart(PackageKit::PackageUpdateDetails::Restart type, const Package &pkg)
{
    Transaction *transaction = qobject_cast<Transaction*>(sender());
    if (transaction->property("restartType").isNull()) {
        transaction->setProperty("restartType", qVariantFromValue(type));
    } else {
        PackageUpdateDetails::Restart oldType;
        oldType = transaction->property("restartType").value<PackageUpdateDetails::Restart>();
        int old = PackageImportance::restartImportance(oldType);
        int newer = PackageImportance::restartImportance(type);
        // Check to see which one is more important
        if (newer > old) {
            transaction->setProperty("restartType", qVariantFromValue(type));
        }
    }

    if (!pkg.name().isEmpty()) {
        QStringList restartPackages = transaction->property("restartPackages").toStringList();
        restartPackages << pkg.name();
        transaction->setProperty("restartPackages", restartPackages);
    }
}

void TransactionWatcher::logout()
{
    KWorkSpace::ShutdownType shutdownType;
    if (m_restartType == PackageUpdateDetails::RestartSystem) {
        // The restart type was system
        shutdownType = KWorkSpace::ShutdownTypeReboot;
    } else if (m_restartType == PackageUpdateDetails::RestartSession) {
        // The restart type was session
        shutdownType = KWorkSpace::ShutdownTypeLogout;
    } else {
        kWarning() << "Unknown restart type:" << m_restartType;
        return;
    }

    // We call KSM server to restart or logout our system
    KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmYes,
                                shutdownType,
                                KWorkSpace::ShutdownModeInteractive);
}

void TransactionWatcher::hideMessageIcon()
{
    // Reset things as the user don't want to see it
    if (m_messagesSNI) {
        m_messagesSNI->deleteLater();
        m_messagesSNI = 0;
    }
    m_messages.clear();
    emit close();
}

void TransactionWatcher::hideRestartIcon()
{
    // Reset things as the user don't want to see it
    if (m_restartSNI) {
        m_restartSNI->deleteLater();
        m_restartSNI = 0;
    }
    m_restartType = PackageUpdateDetails::RestartNone;
    emit close();
}

bool TransactionWatcher::isRunning()
{
    return AbstractIsRunning::isRunning() ||
            !m_transactions.isEmpty() ||
            !m_messages.isEmpty() ||
            m_restartType != PackageUpdateDetails::RestartNone;
}

void TransactionWatcher::suppressSleep(bool enable, const QString &reason)
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
