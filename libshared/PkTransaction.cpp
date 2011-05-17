/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
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

#include <config.h>

#include "PkTransaction.h"

#include <KLocale>
#include <KMessageBox>
#include <KPushButton>
#include <KService>
#include <KPixmapSequence>
#include <KPixmapSequenceOverlayPainter>

#include <KDebug>

#include <QPropertyAnimation>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtGui/QTreeView>

#include <Daemon>

#include "KpkMacros.h"
#include "KpkEnum.h"
#include "KpkStrings.h"
#include "KpkRepoSig.h"
#include "KpkLicenseAgreement.h"
#include "KpkIcons.h"
#include "ProgressView.h"
#include "ApplicationLauncher.h"
#include "KpkSimulateModel.h"
#include "KpkRequirements.h"

#include "ui_PkTransaction.h"

class PkTransactionPrivate
{
public:
    QString tid;
    bool showDetails;
    bool finished;
    bool allowDeps;
    bool onlyTrusted;
    Transaction::Role role;
    Transaction::Error error;
    QString errorDetails;
    QList<Package> packages;
    QStringList files;
    QVector<KService*> applications;
    KpkSimulateModel *simulateModel;
    KPixmapSequenceOverlayPainter *busySeq;

    void clearApplications()
    {
        while (!applications.isEmpty()) {
            delete applications.at(0);
            applications.remove(0);
        }
    }
};

PkTransaction::PkTransaction(Transaction *trans, QWidget *parent)
 : QWidget(parent),
   m_trans(trans),
   m_handlingActionRequired(false),
   m_showingError(false),
   m_exitStatus(Success),
   m_status(Transaction::UnknownStatus),
   ui(new Ui::PkTransaction),
   d(new PkTransactionPrivate)
{
    ui->setupUi(this);

    d->busySeq = new KPixmapSequenceOverlayPainter(this);
    d->busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->busySeq->setWidget(ui->label);
    ui->label->clear();

    d->finished = true; // for sanity we are finished till some transaction is set
    d->onlyTrusted = true; // for sanity we are trusted till an error is given and the user accepts
    d->simulateModel = 0;

//     setButtons(KDialog::Details | KDialog::User1 | KDialog::Cancel);
//     enableButton(KDialog::Details, false);
//     button(KDialog::Details)->setCheckable(true);
    // Setup HIDE custom button
//     setButtonText(KDialog::User1, i18n("Hide"));
//     setButtonToolTip(KDialog::User1,
//                      i18n("Allows you to hide the window whilst keeping the transaction task running."));
//     setEscapeButton(KDialog::User1);

    KConfig config("KPackageKit");
    KConfigGroup transactionGroup(&config, "Transaction");
    
    connect(ui->cancelButton, SIGNAL(rejected()), this, SLOT(cancel()));

    // We need to track when the user close the dialog using the [X] button
//     connect(this, SIGNAL(finished()), SLOT(finishedDialog()));

    // after ALL set, lets set the transaction
    setTransaction(m_trans);

//     setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
//     setMaximumSize(QWIDGETSIZE_MAX, size().height());

//     KConfigGroup transactionDialog(&config, "TransactionDialog");
//     restoreDialogSize(transactionDialog);
}

PkTransaction::~PkTransaction()
{
//     KConfig config("KPackageKit");
//     if (isButtonEnabled(KDialog::Details)) {
//         KConfigGroup transactionGroup(&config, "Transaction");
//         transactionGroup.writeEntry("ShowDetails", d->showDetails);
//     }
//     KConfigGroup transactionDialog(&config, "TransactionDialog");
//     saveDialogSize(transactionDialog);

    if (!d->finished) {
        // We are going to hide the transaction,
        // which can make the user even close System Settings or KPackageKit
        // so we call the tray icon to keep watching the transaction so if the
        // transaction receives some error we can display them
        QDBusMessage message;
        message = QDBusMessage::createMethodCall("org.kde.KPackageKitSmartIcon",
                                                 "/",
                                                 "org.kde.KPackageKitSmartIcon",
                                                 QLatin1String("WatchTransaction"));
        // Use our own cached tid to avoid crashes
        message << qVariantFromValue(d->tid);
        QDBusMessage reply = QDBusConnection::sessionBus().call(message);
        if (reply.type() != QDBusMessage::ReplyMessage) {
            kWarning() << "Message did not receive a reply";
        }
    }

    // DO NOT disconnect the transaction here,
    // it might not exist when this happen
    d->clearApplications();
    delete d;
}

void PkTransaction::hideCancelButton()
{
    ui->cancelButton->hide();
}

void PkTransaction::installPackages(const QList<Package> &packages)
{
    if (Daemon::actions() & Transaction::RoleInstallPackages) {
        if (Daemon::actions() & Transaction::RoleSimulateInstallPackages/* &&
            !(m_flags & HideConfirmDeps)*/) {
            d->packages      = packages;
            d->simulateModel = new KpkSimulateModel(this, d->packages);

            // Create the depends transaction and it's model
            Transaction *trans = new Transaction(this);
            trans->simulateInstallPackages(d->packages);
            if (trans->error()) {
                KMessageBox::sorry(this,
                                   KpkStrings::daemonError(trans->error()),
                                   i18n("Failed to simulate package install"));
//                 taskDone(Transaction::RoleInstallPackages);
            } else {
                setTransaction(trans);
            }
        } else {
            installPackages();
        }
    } else {
        KMessageBox::error(this, i18n("Current backend does not support installing packages."), i18n("Error"));
//         taskDone(Transaction::RoleInstallPackages);
    }
}

void PkTransaction::removePackages(const QList<Package> &packages)
{
    if (Daemon::actions() & Transaction::RoleRemovePackages) {
        if (Daemon::actions() & Transaction::RoleSimulateRemovePackages/* &&
            !(m_flags & HideConfirmDeps)*/) { //TODO we need admin to lock this down
            d->packages      = packages;
            d->simulateModel = new KpkSimulateModel(this, d->packages);

            // Create the requirements transaction and it's model
            Transaction *trans = new Transaction(this);
            trans->simulateRemovePackages(d->packages, AUTOREMOVE);
            if (trans->error()) {
                KMessageBox::sorry(this,
                                   KpkStrings::daemonError(trans->error()),
                                   i18n("Failed to simulate package removal"));
//                 taskDone(Transaction::RoleRemovePackages);
            } else {
                setTransaction(trans);
            }
        } else {
            // As we can't check for requires don't allow deps removal
            removePackages(false);
        }
    } else {
        KMessageBox::error(this, i18n("The current backend does not support removing packages."), i18n("Error"));
//         taskDone(Transaction::RoleRemovePackages);
    }
}

void PkTransaction::updatePackages(const QList<Package> &packages)
{
    if (Daemon::actions() & Transaction::RoleRemovePackages) {
        if (Daemon::actions() & Transaction::RoleSimulateUpdatePackages) {
            d->packages      = packages;
            d->simulateModel = new KpkSimulateModel(this, d->packages);

            Transaction *trans = new Transaction(this);
            trans->simulateUpdatePackages(d->packages);
            if (trans->error()) {
                KMessageBox::sorry(this, KpkStrings::daemonError(trans->error()),
                                i18n("Failed to simulate package update"));
            } else {
                setTransaction(trans);
            }
        } else {
            updatePackages();
        }
    } else {
        KMessageBox::error(this, i18n("The current backend does not support updating packages."), i18n("Error"));
    }
}

void PkTransaction::refreshCache()
{
    SET_PROXY
    Transaction *trans = new Transaction(this);
    trans->refreshCache(true);
    if (trans->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(trans->error()),
                           i18n("Failed to refresh package cache"));
    } else {
        setTransaction(trans);
    }
}

void PkTransaction::installPackages()
{
    SET_PROXY
    QString socket = "/tmp/kpk_debconf_" + QString::number(QCoreApplication::applicationPid());
    Transaction *trans = new Transaction(this);
    trans->setHints("frontend-socket=" + socket);
    trans->installPackages(d->packages, this);
    if (trans->error()) {
        KMessageBox::sorry(this,
                           KpkStrings::daemonError(trans->error()),
                           i18n("Failed to install package"));
//         taskDone(Transaction::RoleInstallPackages);
    } else {
        setTransaction(trans);
        setupDebconfDialog(socket);
//         d->transactionDialog->setPackages(d->addPackages);
    }
}

void PkTransaction::removePackages(bool allow_deps)
{
    SET_PROXY
    QString socket = "/tmp/kpk_debconf_" + QString::number(QCoreApplication::applicationPid());
    Transaction *trans = new Transaction(this);
    trans->setHints("frontend-socket=" + socket);
    trans->removePackages(d->packages, d->allowDeps, AUTOREMOVE);
    if (trans->error()) {
        KMessageBox::sorry(this,
                           KpkStrings::daemonError(trans->error()),
                           i18n("Failed to remove package"));
//         taskDone(Transaction::RoleRemovePackages);
    } else {
        setTransaction(trans);
        setupDebconfDialog(socket);
//         setAllowDeps(allowDeps);
//         d->transactionDialog->setAllowDeps(allowDeps);
    }
}

void PkTransaction::updatePackages()
{
    SET_PROXY
    QString socket = "/tmp/kpk_debconf_" + QString::number(QCoreApplication::applicationPid());
    Transaction *trans = new Transaction(this);
    trans->setHints("frontend-socket=" + socket);
    trans->updatePackages(d->packages, true);
    if (trans->error()) {
        KMessageBox::sorry(this,
                           KpkStrings::daemonError(trans->error()),
                           i18n("Failed to update package"));
//         taskDone(Transaction::RoleRemovePackages);
    } else {
        setTransaction(trans);
        setupDebconfDialog(socket);
//         d->transactionDialog->setAllowDeps(allowDeps);
    }
}

void PkTransaction::cancel()
{
    m_trans->cancel();
//     m_flags |= CloseOnFinish;
}

void PkTransaction::slotButtonClicked(int bt)
{
//     switch(bt) {
//     case KDialog::Cancel :
// //         kDebug() << "KDialog::Cancel";
//         m_trans->cancel();
// //         m_flags |= CloseOnFinish;
//         break;
//     case KDialog::User1 :
// //         kDebug() << "KDialog::User1";
//         // when we're done finishedDialog() is called
//         done(KDialog::User1);
//         break;
//     case KDialog::Close :
// //         kDebug() << "KDialog::Close";
//         // Always disconnect BEFORE emitting finished
//         unsetTransaction();
//         setExitStatus(Cancelled);
//         done(KDialog::Close);
//         break;
//     case KDialog::Details :
//     {
//         d->showDetails = !d->progressView->isVisible();
//         button(KDialog::Details)->setChecked(d->showDetails);
//         if (d->progressView->isVisible()) {
//             QSize windowSize = size();
//             windowSize.rheight() -= d->progressView->height();
//             d->progressView->setVisible(false);
//             setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
//             setMaximumSize(QWIDGETSIZE_MAX, windowSize.height());
//             ui->gridLayout->removeWidget(d->progressView);
//         } else {
//             QSize windowSize = size();
//             windowSize.rheight() += d->progressView->height();
//             setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//             setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
//             ui->gridLayout->addWidget(d->progressView, 1, 0, 1, 2);
//             d->progressView->setVisible(true);
//             resize(windowSize);
//         }
//     }
//         break;
//     default : // Should be only details
//         KDialog::slotButtonClicked(bt);
//     }
}

void PkTransaction::setTransaction(Transaction *trans)
{
    if (!trans) {
        // 0 pointer passed
        return;
    }

    m_trans = trans;
    Transaction::Role role = trans->role();
    if (role != Transaction::RoleInstallSignature &&
        role != Transaction::RoleAcceptEula &&
        role != Transaction::RoleGetFiles) {
        // We need to keep the original role for requeuing
        d->role = role;
    }
    d->tid = trans->tid();
    d->finished = false;
    d->error = Transaction::UnknownError;
    d->errorDetails.clear();
    ui->progressView->clear();
    d->clearApplications();

    KConfig config("KPackageKit");
    KConfigGroup transactionGroup(&config, "Transaction");
    // enable the Details button just on these roles
    if (role == Transaction::RoleInstallPackages ||
        role == Transaction::RoleInstallFiles ||
        role == Transaction::RoleRemovePackages ||
        role == Transaction::RoleUpdatePackages ||
        role == Transaction::RoleUpdateSystem ||
        role == Transaction::RoleRefreshCache) {
        // DISCONNECT THIS SIGNAL BEFORE SETTING A NEW ONE
        if (role == Transaction::RoleRefreshCache) {
            connect(m_trans, SIGNAL(repoDetail(const QString &, const QString &, bool)),
                    ui->progressView, SLOT(currentRepo(const QString &, const QString &)));
            ui->progressView->handleRepo(true);
        } else {
            connect(m_trans, SIGNAL(package(const PackageKit::Package &)),
                ui->progressView, SLOT(currentPackage(const PackageKit::Package &)));
            ui->progressView->handleRepo(false);
        }
        d->showDetails = transactionGroup.readEntry("ShowDetails", false);
//         enableButton(KDialog::Details, true);
        if (d->showDetails != ui->progressView->isVisible()) {
            slotButtonClicked(KDialog::Details);
        }

        d->simulateModel->deleteLater();
        d->simulateModel = 0;
    } else {
        if (role == Transaction::RoleSimulateInstallPackages ||
            role == Transaction::RoleSimulateInstallFiles ||
            role == Transaction::RoleSimulateRemovePackages ||
            role == Transaction::RoleSimulateUpdatePackages) {
            // DISCONNECT THIS SIGNAL BEFORE SETTING A NEW ONE
            if (!d->simulateModel) {
                d->simulateModel = new KpkSimulateModel(this, d->packages);
            }
            d->simulateModel->clear();
            connect(m_trans, SIGNAL(package(const PackageKit::Package &)),
                    d->simulateModel, SLOT(addPackage(const PackageKit::Package &)));
        }

        if (ui->progressView->isVisible()) {
//             slotButtonClicked(KDialog::Details);
        }
//         ui->gridLayout->removeWidget(d->progressView);
//         enableButton(KDialog::Details, false);
    }

    // sets the action icon to be the window icon
    setWindowIcon(KpkIcons::actionIcon(role));

    // Sets the kind of transaction
    emit titleChanged(KpkStrings::action(role));
//     setCaption(KpkStrings::action(role));

    // Now sets the last package
    ui->progressView->currentPackage(m_trans->lastPackage());

    // sets ui
    updateUi();

    // DISCONNECT ALL THESE SIGNALS BEFORE SETTING A NEW ONE
    connect(m_trans, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
            this, SLOT(transactionFinished(PackageKit::Transaction::Exit)));
    connect(m_trans, SIGNAL(errorCode(PackageKit::Transaction::Error, const QString &)),
            this, SLOT(errorCode(PackageKit::Transaction::Error, const QString &)));
    connect(m_trans, SIGNAL(changed()),
            this, SLOT(updateUi()));
    connect(m_trans, SIGNAL(eulaRequired(PackageKit::Eula)),
            this, SLOT(eulaRequired(PackageKit::Eula)));
    connect(m_trans, SIGNAL(mediaChangeRequired(PackageKit::Transaction::MediaType, const QString &, const QString &)),
            this, SLOT(mediaChangeRequired(PackageKit::Transaction::MediaType, const QString &, const QString &)));
    connect(m_trans, SIGNAL(repoSignatureRequired(PackageKit::Signature)),
            this, SLOT(repoSignatureRequired(PackageKit::Signature)));
    // DISCONNECT ALL THESE SIGNALS BEFORE SETTING A NEW ONE
}

void PkTransaction::unsetTransaction()
{
    disconnect(m_trans, SIGNAL(package(const PackageKit::Package &)),
               d->simulateModel, SLOT(addPackage(const PackageKit::Package &)));
    disconnect(m_trans, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
               this, SLOT(transactionFinished(PackageKit::Transaction::Exit)));
    disconnect(m_trans, SIGNAL(errorCode(PackageKit::Transaction::Error, const QString &)),
               this, SLOT(errorCode(PackageKit::Transaction::Error, const QString &)));
    disconnect(m_trans, SIGNAL(changed()),
               this, SLOT(updateUi()));
    disconnect(m_trans, SIGNAL(eulaRequired(PackageKit::Eula)),
               this, SLOT(eulaRequired(PackageKit::Eula)));
    disconnect(m_trans, SIGNAL(mediaChangeRequired(PackageKit::Transaction::MediaType, const QString &, const QString &)),
               this, SLOT(mediaChangeRequired(PackageKit::Transaction::MediaType, const QString &, const QString &)));
    disconnect(m_trans, SIGNAL(repoSignatureRequired(PackageKit::Signature)),
               this, SLOT(repoSignatureRequired(PackageKit::Signature)));
}

void PkTransaction::requeueTransaction()
{
    SET_PROXY
    Transaction *trans = new Transaction(this);
    QString socket;
    socket = "/tmp/kpk_debconf_" + QString::number(QCoreApplication::applicationPid());
    trans->setHints("frontend-socket=" + socket);
    switch (d->role) {
    case Transaction::RoleRemovePackages :
        trans->removePackages(d->packages, d->allowDeps, AUTOREMOVE);
        break;
    case Transaction::RoleInstallPackages :
        trans->installPackages(d->packages, d->onlyTrusted);
        break;
    case Transaction::RoleInstallFiles :
        trans->installFiles(d->files, d->onlyTrusted);
        break;
    case Transaction::RoleUpdatePackages :
        trans->updatePackages(d->packages, d->onlyTrusted);
        break;
    default :
        setExitStatus(Failed);
        return;
    }

    if (trans->error()) {
        KMessageBox::sorry(this,
                           KpkStrings::daemonError(trans->error()),
                           KpkStrings::action(trans->role()));
        setExitStatus(Failed);
    } else {
        setTransaction(trans);
    }
}

void PkTransaction::updateUi()
{
    uint percentage = m_trans->percentage();
    if (percentage <= 100) {
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(percentage);
    } else if (ui->progressBar->maximum() != 0) {
        ui->progressBar->setMaximum(0);
        ui->progressBar->reset();
    }

    ui->progressView->setSubProgress(m_trans->subpercentage());
    ui->progressBar->setRemaining(m_trans->remainingTime());

    // Status & Speed
    Transaction::Status status = m_trans->status();
    if (m_status != status) {
        m_status = status;
        ui->currentL->setText(KpkStrings::status(status));

//         kDebug() << KIconLoader::global()->iconPath(KpkIcons::statusAnimation(status), -KIconLoader::SizeLarge);
        KPixmapSequence sequence = KPixmapSequence(KpkIcons::statusAnimation(status),
                                                   KIconLoader::SizeLarge);
        if (sequence.isValid()) {
            d->busySeq->setSequence(sequence);
            d->busySeq->start();
        }
    } else if (status == Transaction::StatusDownload && m_trans->speed() != 0) {
        uint speed = m_trans->speed();
        if (speed) {
            ui->currentL->setText(i18n("Downloading packages at %1/s",
                                         KGlobal::locale()->formatByteSize(speed)));
        }
    }

    // check to see if we can cancel
    bool cancel = m_trans->allowCancel();
    emit allowCancel(cancel);
    ui->cancelButton->setEnabled(cancel);
}

// Return value: if the error code suggests to try with only_trusted %FALSE
static bool untrustedIsNeed(Transaction::Error error)
{
    switch (error) {
    case Transaction::ErrorGpgFailure:
    case Transaction::ErrorBadGpgSignature:
    case Transaction::ErrorMissingGpgSignature:
    case Transaction::ErrorCannotInstallRepoUnsigned:
    case Transaction::ErrorCannotUpdateRepoUnsigned:
        return true;
    default:
        return false;
    }
}

void PkTransaction::errorCode(Transaction::Error error, const QString &details)
{
//     kDebug() << "errorCode: " << error << details;
    d->error = error;
    d->errorDetails = details;
    // obvious message, don't tell the user
    if (m_handlingActionRequired ||
        error == Transaction::ErrorTransactionCancelled ||
        error == Transaction::ErrorProcessKill) {
        return;
    }

    if (untrustedIsNeed(error)) {
        m_handlingActionRequired = true;
        int ret = KMessageBox::warningYesNo(this,
                                            i18n("You are about to install unsigned packages that can compromise your system, "
                                            "as it is impossible to verify if the software came from a trusted "
                                            "source.\n\nAre you sure you want to proceed with the installation?"),
                                            i18n("Installing unsigned software"));
        if (ret == KMessageBox::Yes) {
            // Set only trusted to false, to do as the user asked
            d->onlyTrusted = false;
            requeueTransaction();
        } else {
            setExitStatus(Cancelled);
//             if (m_flags & CloseOnFinish) {
//                 done(QDialog::Rejected);
//             }
        }
        m_handlingActionRequired = false;
        return;
    }

    // check to see if we are already handlying these errors
    if (error == Transaction::ErrorNoLicenseAgreement ||
        error == Transaction::ErrorMediaChangeRequired)
    {
        if (m_handlingActionRequired) {
            return;
        }
    }

    m_showingError = true;
    KMessageBox::detailedSorry(this,
                               KpkStrings::errorMessage(error),
                               QString(details).replace('\n', "<br />"),
                               KpkStrings::error(error),
                               KMessageBox::Notify);
    m_showingError = false;

    // when we receive an error we are done
    setExitStatus(Failed);
    // TODO maybe this should go in the above method
//     if (m_flags & CloseOnFinish) {
//         done(QDialog::Rejected);
//     }
}

void PkTransaction::eulaRequired(PackageKit::Eula info)
{
    if (m_handlingActionRequired) {
        // if its true means that we alread passed here
        m_handlingActionRequired = false;
        return;
    } else {
        m_handlingActionRequired = true;
    }

    QPointer<KpkLicenseAgreement> frm = new KpkLicenseAgreement(info, true, this);
    if (frm->exec() == KDialog::Yes) {
        m_handlingActionRequired = false;
        Transaction *trans = new Transaction(this);
        trans->acceptEula(info.id);
        if (trans->error()) {
            KMessageBox::sorry(this,
                               KpkStrings::daemonError(trans->error()),
                               i18n("Failed to accept EULA"));
        } else {
            setTransaction(trans);
        }
    } else {
        setExitStatus(Cancelled);
        m_handlingActionRequired = false;
    }
    delete frm;
}

void PkTransaction::mediaChangeRequired(Transaction::MediaType type, const QString &id, const QString &text)
{
    Q_UNUSED(id)

    m_handlingActionRequired = true;
    int ret = KMessageBox::questionYesNo(this,
                                         KpkStrings::mediaMessage(type, text),
                                         i18n("A media change is required"),
                                         KStandardGuiItem::cont(),
                                         KStandardGuiItem::cancel());
    m_handlingActionRequired = false;

    // if the user clicked continue we got yes
    if (ret == KMessageBox::Yes) {
        requeueTransaction();
    } else {
        setExitStatus(Cancelled);
    }
}

void PkTransaction::repoSignatureRequired(PackageKit::Signature info)
{
    if (m_handlingActionRequired) {
        // if its true means that we alread passed here
        m_handlingActionRequired = false;
        return;
    } else {
        m_handlingActionRequired = true;
    }

    QPointer<KpkRepoSig> frm = new KpkRepoSig(info, true, this);
    frm->exec();
    if (frm && frm->result() == KDialog::Yes) {
        m_handlingActionRequired = false;
        Transaction *trans = new Transaction(this);
        trans->installSignature(info);
        if (trans->error()) {
            KMessageBox::sorry(this,
                               KpkStrings::daemonError(trans->error()),
                               i18n("Failed to install signature"));
        } else {
            setTransaction(trans);
        }
    } else {
        setExitStatus(Cancelled);
        m_handlingActionRequired = false;
    }
    delete frm;
}

void PkTransaction::files(const PackageKit::Package &package, const QStringList &files)
{
    Q_UNUSED(package)
    foreach (const QString &desktop, files.filter(".desktop")) {
        // we create a new KService because findByDestopPath
        // might fail because the Sycoca database is not up to date yet.
        KService *service = new KService(desktop);
        if (service->isApplication() &&
           !service->noDisplay() &&
           !service->exec().isEmpty())
        {
            d->applications << service;
        }
    }
}

void PkTransaction::transactionFinished(Transaction::Exit status)
{
    Transaction *trans = qobject_cast<Transaction*>(sender());
    Transaction::Role role = trans->role();
    KpkRequirements *requires = 0;

//     kDebug() << status;
    d->finished = true;
    switch(status) {
    case Transaction::ExitSuccess :
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(100);

        // If the simulate model exists we were simulating
        if (d->simulateModel) {
            if (d->simulateModel->rowCount() > 0) {
                requires = new KpkRequirements(d->simulateModel, this);
                connect(requires, SIGNAL(rejected()), this, SLOT(reject()));
                requires->show();
            }

            switch (role) {
            case Transaction::RoleSimulateInstallPackages:
                if (requires) {
                    connect(requires, SIGNAL(accepted()), this, SLOT(installPackages()));
                } else {
                    installPackages();
                }
                return;
            case Transaction::RoleSimulateRemovePackages:
                if (requires) {
                    connect(requires, SIGNAL(accepted()), this, SLOT(removePackages()));
                } else {
                    // As there was no requires don't allow deps removal
                    removePackages(false);
                }
                return;
            case Transaction::RoleSimulateUpdatePackages:
                if (requires) {
                    connect(requires, SIGNAL(accepted()), this, SLOT(updatePackages()));
                } else {
                    updatePackages();
                }
                return;
            case Transaction::RoleSimulateInstallFiles:
                // TODO
                return;
            default:
                break;
            }
        }

        if (role != Transaction::RoleInstallSignature &&
            role != Transaction::RoleAcceptEula &&
            role != Transaction::RoleGetFiles) {
            KConfig config("KPackageKit");
            KConfigGroup transactionGroup(&config, "Transaction");
            if ((role == Transaction::RoleInstallPackages ||
                 role == Transaction::RoleInstallFiles) &&
                transactionGroup.readEntry("ShowApplicationLauncher", true) &&
                Daemon::actions() & Transaction::RoleGetFiles) {
                // Let's try to find some desktop files'
                Transaction *transaction = new Transaction(this);
                transaction->getFiles(d->packages);
                if (!transaction->error()) {
                    setTransaction(transaction);
                    connect(transaction, SIGNAL(files(const PackageKit::Package &, const QStringList &)),
                            this, SLOT(files(const PackageKit::Package &, const QStringList &)));
                    return; // avoid the exit code
                }
            }
            setExitStatus(Success);
        } else if (role == Transaction::RoleGetFiles) {
            if (!d->applications.isEmpty()) {
                ApplicationLauncher *launcher = new ApplicationLauncher(d->applications, this);
                launcher->exec();
            }
            setExitStatus(Success);
        } else {
            d->finished = false;
            requeueTransaction();
        }
        break;
    case Transaction::ExitCancelled :
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(100);
        // Avoid crash in case we are showing an error
        if (!m_showingError) {
            setExitStatus(Cancelled);
        }
        break;
    case Transaction::ExitFailed :
        kDebug() << "Failed.";
        if (!m_handlingActionRequired && !m_showingError) {
            ui->progressBar->setMaximum(0);
            ui->progressBar->reset();
            kDebug() << "Yep, we failed.";
            setExitStatus(Failed);
        }
        break;
    case Transaction::ExitKeyRequired :
    case Transaction::ExitEulaRequired :
    case Transaction::ExitMediaChangeRequired :
    case Transaction::ExitNeedUntrusted :
        kDebug() << "finished KeyRequired or EulaRequired: " << status;
        ui->currentL->setText(KpkStrings::status(Transaction::StatusSetup));
        if (!m_handlingActionRequired) {
            setExitStatus(Failed);
        }
        break;
    default :
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(100);
        kDebug() << "finished default" << status;
//         KDialog::slotButtonClicked(KDialog::Close);
        setExitStatus(Failed);
        break;
    }
    // if we're not showing an error or don't have the
    // CloseOnFinish flag don't close the dialog
    //TODO !!!!!!!!!
//     if (m_flags & CloseOnFinish && !m_handlingActionRequired && !m_showingError) {
// //         kDebug() << "CloseOnFinish && !m_handlingActionRequired && !m_showingError";
//         done(QDialog::Rejected);
//         deleteLater();
//     }
}

PkTransaction::ExitStatus PkTransaction::exitStatus() const
{
    return m_exitStatus;
}

bool PkTransaction::isFinished() const
{
    return d->finished;
}

void PkTransaction::setExitStatus(PkTransaction::ExitStatus status)
{
    kDebug() << status;
    m_exitStatus = status;
    emit finished(status);
}

void PkTransaction::reject()
{
    d->finished = true;
    setExitStatus(Cancelled);
}

QString PkTransaction::tid() const
{
    return d->tid;
}

bool PkTransaction::allowDeps() const
{
    return d->allowDeps;
}

bool PkTransaction::onlyTrusted() const
{
    return d->onlyTrusted;
}

Transaction::Role PkTransaction::role() const
{
    return d->role;
}

Transaction::Error PkTransaction::error() const
{
    return d->error;
}

QString PkTransaction::errorDetails() const
{
    return d->errorDetails;
}

QList<Package> PkTransaction::packages() const
{
    return d->packages;
}

QStringList PkTransaction::files() const
{
    return d->files;
}

KpkSimulateModel* PkTransaction::simulateModel() const
{
    return d->simulateModel;
}

void PkTransaction::setAllowDeps(bool allowDeps)
{
    d->allowDeps = allowDeps;
}

void PkTransaction::setPackages(const QList<Package> &packages)
{
    d->packages = packages;
}

void PkTransaction::setFiles(const QStringList &files)
{
    d->files = files;
}

void PkTransaction::setupDebconfDialog(const QString &tid)
{
#ifdef HAVE_DEBCONFKDE
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.kde.KPackageKitSmartIcon",
                                             "/",
                                             "org.kde.KPackageKitSmartIcon",
                                             QLatin1String("SetupDebconfDialog"));
    // Use our own cached tid to avoid crashes
    message << qVariantFromValue(tid);
    message << qVariantFromValue(static_cast<uint>(effectiveWinId()));
    QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        kWarning() << "Message did not receive a reply";
    }
#else
    Q_UNUSED(tid)
#endif //HAVE_DEBCONFKDE
}

#include "PkTransaction.moc"
