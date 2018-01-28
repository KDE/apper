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

#include <PkStrings.h>
#include <PkIcons.h>
#include <PackageImportance.h>

#include <KNotification>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNotification>
//#include <KComponentData>

//#include <Solid/PowerManagement>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>

#include <kworkspace5/kworkspace.h>
#include <Daemon>

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(APPER_DAEMON)

TransactionWatcher::TransactionWatcher(bool packagekitIsRunning, QObject *parent) :
    QObject(parent),
    m_inhibitCookie(-1)
{
    m_tracker = new KUiServerJobTracker(this);

    // keep track of new transactions
    connect(Daemon::global(), &Daemon::transactionListChanged, this, &TransactionWatcher::transactionListChanged);

    // if PackageKit is running check to see if there are running transactons already
    if (packagekitIsRunning) {
        // here we check whether a transaction job should be created or not
        QStringList tids;
        const QList<QDBusObjectPath> paths = Daemon::global()->getTransactionList();
        for (const QDBusObjectPath &path : paths) {
            tids << path.path();
        }
        transactionListChanged(tids);
    }
}

TransactionWatcher::~TransactionWatcher()
{
    // release any cookie that we might have
    suppressSleep(false, m_inhibitCookie);
}

void TransactionWatcher::watchTransactionInteractive(const QDBusObjectPath &tid)
{
    watchTransaction(tid);
}

void TransactionWatcher::transactionListChanged(const QStringList &tids)
{
    if (tids.isEmpty()) {
        // release any cookie that we might have
        suppressSleep(false, m_inhibitCookie);
    } else {
        for (const QString &tid : tids) {
            watchTransaction(QDBusObjectPath(tid), false);
        }
    }
}

void TransactionWatcher::watchTransaction(const QDBusObjectPath &tid, bool interactive)
{
    Transaction *transaction;
    if (!m_transactions.contains(tid)) {
        // Check if the current transaction is still the same
        transaction = new Transaction(tid);
        connect(transaction, &Transaction::roleChanged, this, &TransactionWatcher::transactionReady);
        connect(transaction, &Transaction::finished, this, &TransactionWatcher::finished);

        // Store the transaction id
        m_transactions[tid] = transaction;
    } else {
        transaction = m_transactions[tid];

        if (transaction->role() != Transaction::RoleUnknown) {
            // force the first changed or create a TransactionJob
            transactionChanged(transaction, interactive);
        }
    }
}

void TransactionWatcher::transactionReady()
{
    auto transaction = qobject_cast<Transaction*>(sender());

    Transaction::Role role = transaction->role();
    Transaction::TransactionFlags flags = transaction->transactionFlags();
    if (!(flags & Transaction::TransactionFlagOnlyDownload || flags & Transaction::TransactionFlagSimulate) &&
            (role == Transaction::RoleInstallPackages ||
             role == Transaction::RoleInstallFiles    ||
             role == Transaction::RoleRemovePackages  ||
             role == Transaction::RoleUpdatePackages)) {
        // AVOID showing messages and restart requires when
        // the user was just simulating an instalation
        connect(transaction, &Transaction::requireRestart, this, &TransactionWatcher::requireRestart);

        // Don't let the system sleep while doing some sensible actions
        suppressSleep(true, m_inhibitCookie, PkStrings::action(role, flags));
    }

    connect(transaction, &Transaction::isCallerActiveChanged, this, [this, transaction] () {
        transactionChanged(transaction);
    });

}

void TransactionWatcher::showRebootNotificationApt() {
    // Create the notification about this transaction
    auto notify = new KNotification(QLatin1String("RestartRequired"), 0, KNotification::Persistent);
    connect(notify, QOverload<uint>::of(&KNotification::activated), this, &TransactionWatcher::logout);
    notify->setComponentName(QLatin1String("apperd"));

    QString text(QLatin1String("<b>") + i18n("The system update has completed") + QLatin1String("</b>"));
    text.append(QLatin1String("<br>") + PkStrings::restartType(Transaction::RestartSystem));
    notify->setPixmap(PkIcons::restartIcon(Transaction::RestartSystem).pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
    notify->setText(text);

    // TODO RestartApplication should be handled differently
    QStringList actions;
    actions << i18n("Restart");
    notify->setActions(actions);

    notify->sendEvent();
}

void TransactionWatcher::finished(PackageKit::Transaction::Exit exit)
{
    // check if the transaction emitted any require restart
    auto transaction = qobject_cast<Transaction*>(sender());
    QDBusObjectPath tid = transaction->tid();
    transaction->disconnect(this);
    m_transactions.remove(tid);
    m_transactionJob.remove(tid);

    if (exit == Transaction::ExitSuccess && !transaction->property("restartType").isNull()) {
        Transaction::Restart type = transaction->property("restartType").value<Transaction::Restart>();
        QStringList restartPackages = transaction->property("restartPackages").toStringList();

        // Create the notification about this transaction
        auto notify = new KNotification(QLatin1String("RestartRequired"), 0, KNotification::Persistent);
        connect(notify, QOverload<uint>::of(&KNotification::activated), this, &TransactionWatcher::logout);
        notify->setComponentName(QLatin1String("apperd"));
        notify->setProperty("restartType", qVariantFromValue(type));
        notify->setPixmap(PkIcons::restartIcon(type).pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
        notify->setTitle(PkStrings::restartType(type));

        // Create a readable text with package names that required the restart
        if (!restartPackages.isEmpty()) {
            restartPackages.removeDuplicates();
            restartPackages.sort();

            QString text;
            text = i18np("Package: %2",
                         "Packages: %2",
                         restartPackages.size(),
                         restartPackages.join(QLatin1String(", ")));
            notify->setText(text);
        }

        // TODO RestartApplication should be handled differently
        QStringList actions;
        actions << i18n("Restart");
        notify->setActions(actions);

        notify->sendEvent();
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
        auto job = new TransactionJob(transaction, this);
        connect(transaction, &Transaction::errorCode, this, &TransactionWatcher::errorCode);
        connect(job, &TransactionJob::canceled, this, &TransactionWatcher::watchedCanceled);
        m_tracker->registerJob(job);
        m_transactionJob[tid] = job;
        job->start();
    }
}

void TransactionWatcher::errorCode(PackageKit::Transaction::Error err, const QString &details)
{
    auto notify = new KNotification(QLatin1String("TransactionError"), 0, KNotification::Persistent);
    notify->setComponentName(QLatin1String("apperd"));
    notify->setTitle(PkStrings::error(err));
    notify->setText(PkStrings::errorMessage(err));
    notify->setProperty("ErrorType", QVariant::fromValue(err));
    notify->setProperty("Details", details);

    QStringList actions;
    actions << i18n("Details");
    notify->setActions(actions);
    notify->setPixmap(QIcon::fromTheme(QLatin1String("dialog-error")).pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
    connect(notify, QOverload<uint>::of(&KNotification::activated), this, &TransactionWatcher::errorActivated);
    notify->sendEvent();
}

void TransactionWatcher::errorActivated(uint action)
{
    auto notify = qobject_cast<KNotification*>(sender());

    // if the user clicked "Details"
    if (action == 1) {
        Transaction::Error error = notify->property("ErrorType").value<Transaction::Error>();
        QString details = notify->property("Details").toString();
        KMessageBox::detailedSorry(0,
                                   PkStrings::errorMessage(error),
                                   details.replace(QLatin1Char('\n'), QLatin1String("<br>")),
                                   PkStrings::error(error),
                                   KMessageBox::Notify);
    }

    notify->close();
}

void TransactionWatcher::requireRestart(PackageKit::Transaction::Restart type, const QString &packageID)
{
    auto transaction = qobject_cast<Transaction*>(sender());
    if (transaction->property("restartType").isNull()) {
        transaction->setProperty("restartType", qVariantFromValue(type));
    } else {
        Transaction::Restart oldType;
        oldType = transaction->property("restartType").value<Transaction::Restart>();
        int old = PackageImportance::restartImportance(oldType);
        int newer = PackageImportance::restartImportance(type);
        // Check to see which one is more important
        if (newer > old) {
            transaction->setProperty("restartType", qVariantFromValue(type));
        }
    }

    if (!Transaction::packageName(packageID).isEmpty()) {
        QStringList restartPackages = transaction->property("restartPackages").toStringList();
        restartPackages << Transaction::packageName(packageID);
        transaction->setProperty("restartPackages", restartPackages);
    }
}

void TransactionWatcher::logout()
{
    auto notify = qobject_cast<KNotification*>(sender());
    Transaction::Restart restartType;
    restartType = notify->property("restartType").value<Transaction::Restart>();

    KWorkSpace::ShutdownType shutdownType;
    switch (restartType) {
    case Transaction::RestartSystem:
    case Transaction::RestartSecuritySystem:
        // The restart type was system
        shutdownType = KWorkSpace::ShutdownTypeReboot;
        break;
    case Transaction::RestartSession:
    case Transaction::RestartSecuritySession:
        // The restart type was session
        shutdownType = KWorkSpace::ShutdownTypeLogout;
        break;
    default:
        qCWarning(APPER_DAEMON) << "Unknown restart type:" << restartType;
        return;
    }

    // We call KSM server to restart or logout our system
    KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmYes,
                                shutdownType,
                                KWorkSpace::ShutdownModeInteractive);
}

void TransactionWatcher::watchedCanceled()
{
    auto job = qobject_cast<TransactionJob*>(sender());
    if (job->isFinished()) {
        job->deleteLater();
        return;
    }

    Transaction::Role role = job->transaction()->role();
    if (role != Transaction::RoleCancel &&
            role != Transaction::RoleUnknown) {
        m_tracker->unregisterJob(job);
        m_tracker->registerJob(job);
        job->start();
    }
}

void TransactionWatcher::suppressSleep(bool enable, int &inhibitCookie, const QString &reason)
{
    if (inhibitCookie == -1) {
        return;
    }

    if (enable) {
        qCDebug(APPER_DAEMON) << "Begin Suppressing Sleep";
//        inhibitCookie = Solid::PowerManagement::beginSuppressingSleep(reason);
        if (inhibitCookie == -1) {
            qCDebug(APPER_DAEMON) << "Sleep suppression denied!";
        }
    } else {
        qCDebug(APPER_DAEMON) << "Stop Suppressing Sleep";
//        if (!Solid::PowerManagement::stopSuppressingSleep(inhibitCookie)) {
            qCDebug(APPER_DAEMON) << "Stop failed: invalid cookie.";
//        }
        inhibitCookie = -1;
    }
}

#include "moc_TransactionWatcher.cpp"
