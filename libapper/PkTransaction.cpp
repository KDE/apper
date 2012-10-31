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

#include <config.h>

#include "PkTransaction.h"
//#include "ui_PkTransaction.h"

#include <KLocale>
#include <KMessageBox>
#include <KPushButton>
#include <KService>
#include <KPixmapSequence>
#include <KPixmapSequenceOverlayPainter>

#include <KDebug>

#include <QStringBuilder>
#include <QPropertyAnimation>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtGui/QTreeView>

#include <Daemon>

#include "Enum.h"
#include "PkStrings.h"
#include "RepoSig.h"
#include "LicenseAgreement.h"
#include "PkIcons.h"
#include "ApplicationLauncher.h"
#include "PackageModel.h"
#include "Requirements.h"
#include "PkTransactionProgressModel.h"

class PkTransactionPrivate
{
public:
    bool finished;
    bool allowDeps;
    Transaction::TransactionFlags flags;
    Transaction::Role role;
    Transaction::Role originalRole;
    Transaction::Error error;
    QStringList packages;
    QStringList packagesToResolve;
    ApplicationLauncher *launcher;
    QStringList files;
    PackageModel *simulateModel;
    PkTransactionProgressModel *progressModel;
};

PkTransaction::PkTransaction(QWidget *parent) :
    Transaction(parent),
    m_trans(0),
    m_handlingActionRequired(false),
    m_showingError(false),
    m_exitStatus(Success),
    m_status(Transaction::StatusUnknown),
    d(new PkTransactionPrivate)
{
    kDebug() << status() << role();
    // for sanity we are finished till some transaction is set
    d->finished = true;
    d->simulateModel = new PackageModel(this);
    d->launcher = 0;
    d->originalRole = Transaction::RoleUnknown;
    d->role = Transaction::RoleUnknown;
    // for sanity we are trusted till an error is given and the user accepts
    d->flags = Transaction::TransactionFlagOnlyTrusted;
    d->progressModel = new PkTransactionProgressModel(this);
    connect(this, SIGNAL(repoDetail(QString,QString,bool)),
            d->progressModel, SLOT(currentRepo(QString,QString,bool)));
    connect(this, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            d->progressModel, SLOT(currentPackage(PackageKit::Transaction::Info,QString)));
    connect(this, SIGNAL(itemProgress(QString,PackageKit::Transaction::Status,uint)),
            d->progressModel, SLOT(itemProgress(QString,PackageKit::Transaction::Status,uint)));

    connect(this, SIGNAL(changed()), SLOT(updateUi()));
}

PkTransaction::~PkTransaction()
{
    // DO NOT disconnect the transaction here,
    // it might not exist when this happen
    delete d;
}

void PkTransaction::installFiles(const QStringList &files)
{
    if (Daemon::global()->actions() & Transaction::RoleInstallFiles) {
        d->originalRole = Transaction::RoleInstallFiles;
        d->files = files;
        d->flags = Transaction::TransactionFlagOnlyTrusted | Transaction::TransactionFlagSimulate;

        // Create the simulate transaction and it's model
        Transaction *trans = new Transaction(this);
        setTransaction(trans, Transaction::RoleInstallFiles);
        trans->installFiles(files, d->flags);
        if (trans->error()) {
            showSorry(i18n("Failed to simulate file install"),
                      PkStrings::daemonError(trans->error()));
        }
    } else {
        showError(i18n("Current backend does not support installing files."), i18n("Error"));
    }
}

void PkTransaction::installPackages(const QStringList &packages)
{
    if (Daemon::global()->actions() & Transaction::RoleInstallPackages) {
        d->originalRole = Transaction::RoleInstallPackages;
        d->packages = packages;
        d->flags = Transaction::TransactionFlagOnlyTrusted | Transaction::TransactionFlagSimulate;

        // Create the depends transaction and it's model
        Transaction *trans = new Transaction(this);
        setTransaction(trans, Transaction::RoleInstallPackages);
        trans->installPackages(d->packages, d->flags);
        if (trans->error()) {
            showSorry(i18n("Failed to simulate package install"),
                      PkStrings::daemonError(trans->error()));
        }
    } else {
        showError(i18n("Current backend does not support installing packages."), i18n("Error"));
    }
}

void PkTransaction::removePackages(const QStringList &packages)
{
    if (Daemon::global()->actions() & Transaction::RoleRemovePackages) {
        d->originalRole = Transaction::RoleRemovePackages;
        d->allowDeps = false; // Default to avoid dependencies removal unless simulate says so
        d->packages = packages;
        d->flags = Transaction::TransactionFlagOnlyTrusted | Transaction::TransactionFlagSimulate;

        // Create the requirements transaction and it's model
        Transaction *trans = new Transaction(this);
        setTransaction(trans, Transaction::RoleRemovePackages);
        trans->removePackages(d->packages, d->allowDeps, AUTOREMOVE, d->flags);
        if (trans->error()) {
            showSorry(i18n("Failed to simulate package removal"),
                      PkStrings::daemonError(trans->error()));
        }
    } else {
        showError(i18n("The current backend does not support removing packages."), i18n("Error"));
    }
}

void PkTransaction::updatePackages(const QStringList &packages)
{
    if (Daemon::global()->actions() & Transaction::RoleUpdatePackages) {
        d->originalRole = Transaction::RoleUpdatePackages;
        d->packages = packages;
        d->flags = Transaction::TransactionFlagOnlyTrusted | Transaction::TransactionFlagSimulate;

        Transaction *trans = new Transaction(this);
        setTransaction(trans, Transaction::RoleUpdatePackages);
        trans->updatePackages(d->packages, d->flags);
        if (trans->error()) {
            showSorry(i18n("Failed to simulate package update"),
                      PkStrings::daemonError(trans->error()));
        }
    } else {
        showError(i18n("The current backend does not support updating packages."), i18n("Error"));
    }
}

//void PkTransaction::refreshCache(bool force)
//{
//    kDebug() << force;
//    Transaction::refreshCache(force);
//    if (error()) {
//        showSorry(i18n("Failed to refresh package cache"),
//                  PkStrings::daemonError(error()));
//    } else {
//        d->finished = false;
//    }
//}

void PkTransaction::setupTransaction(PackageKit::Transaction *transaction)
{
#ifdef HAVE_DEBCONFKDE
    QString tid;
    QString socket;
    tid = transaction->tid();
    // Build a socket path like /tmp/1761_edeceabd_data_debconf
    socket = QLatin1String("/tmp") % tid % QLatin1String("_debconf");
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.kde.ApperSentinel"),
                                             QLatin1String("/"),
                                             QLatin1String("org.kde.ApperSentinel"),
                                             QLatin1String("SetupDebconfDialog"));
    // Use our own cached tid to avoid crashes
    message << qVariantFromValue(tid);
    message << qVariantFromValue(socket);
    message << qVariantFromValue(static_cast<uint>(effectiveWinId()));
    QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        kWarning() << "Message did not receive a reply";
    }

    transaction->setHints(QLatin1String("frontend-socket=") % socket);
#else
     Q_UNUSED(transaction)
#endif //HAVE_DEBCONFKDE
}

void PkTransaction::installPackages()
{
    Transaction *trans = new Transaction(this);
    setupTransaction(trans);
    setTransaction(trans, Transaction::RoleInstallPackages);
    trans->installPackages(d->packages, d->flags);
    if (trans->error()) {
        showSorry(i18n("Failed to install package"),
                  PkStrings::daemonError(trans->error()));
    }
}

void PkTransaction::installFiles()
{
    Transaction *trans = new Transaction(this);
    setupTransaction(trans);
    setTransaction(trans, Transaction::RoleInstallFiles);
    trans->installFiles(d->files, d->flags);
    if (trans->error()) {
        showSorry(i18np("Failed to install file",
                        "Failed to install files", d->files.size()),
                  PkStrings::daemonError(trans->error()));
    }
}

void PkTransaction::removePackages()
{
    Transaction *trans = new Transaction(this);
    setupTransaction(trans);
    setTransaction(trans, Transaction::RoleRemovePackages);
    trans->removePackages(d->packages, d->allowDeps, AUTOREMOVE, d->flags);
    if (trans->error()) {
        showSorry(i18n("Failed to remove package"),
                  PkStrings::daemonError(trans->error()));
    }
}

void PkTransaction::updatePackages()
{
    Transaction *trans = new Transaction(this);
    setupTransaction(trans);
    setTransaction(trans, Transaction::RoleUpdatePackages);
    trans->updatePackages(d->packages, d->flags);
    if (trans->error()) {
        showSorry(i18n("Failed to update package"),
                  PkStrings::daemonError(trans->error()));
    }
}

void PkTransaction::cancel()
{
    if (m_trans) {
        m_trans->cancel();
    }
}

void PkTransaction::setTransaction(Transaction *trans, Transaction::Role role)
{
    Q_ASSERT(trans);

    m_trans = trans;
    d->role = role;
    d->finished = false;
    m_handlingActionRequired = false;
    m_showingError = false;
    d->error = Transaction::ErrorUnknown;

    if (role != Transaction::RoleUnknown) {
//        setWindowTitle(PkStrings::action(role));
        emit titleChanged(PkStrings::action(role));
    }



    // sets the action icon to be the window icon
//    setWindowIcon(PkIcons::actionIcon(role));

    // sets ui
    updateUi();

    // DISCONNECT ALL THESE SIGNALS BEFORE SETTING A NEW ONE
//    connect(m_trans, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
//            this, SLOT(transactionFinished(PackageKit::Transaction::Exit)));
//    connect(m_trans, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
//            this, SLOT(errorCode(PackageKit::Transaction::Error,QString)));
//    connect(m_trans, SIGNAL(changed()),
//            this, SLOT(updateUi()));
//    connect(m_trans, SIGNAL(eulaRequired(QString,QString,QString,QString)),
//            this, SLOT(eulaRequired(QString,QString,QString,QString)));
//    connect(m_trans, SIGNAL(mediaChangeRequired(PackageKit::Transaction::MediaType,QString,QString)),
//            this, SLOT(mediaChangeRequired(PackageKit::Transaction::MediaType,QString,QString)));
//    connect(m_trans, SIGNAL(repoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,PackageKit::Transaction::SigType)),
//            this, SLOT(repoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,PackageKit::Transaction::SigType)));
    // DISCONNECT ALL THESE SIGNALS BEFORE SETTING A NEW ONE
}

void PkTransaction::unsetTransaction()
{
    if (m_trans == 0) {
        return;
    }

//    disconnect(m_trans, SIGNAL(ItemProgress(QString,uint)),
//               ui->progressView, SLOT(itemProgress(QString,uint)));
//    disconnect(m_trans, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
//               d->simulateModel, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
//    disconnect(m_trans, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
//               this, SLOT(transactionFinished(PackageKit::Transaction::Exit)));
//    disconnect(m_trans, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
//               this, SLOT(errorCode(PackageKit::Transaction::Error,QString)));
//    disconnect(m_trans, SIGNAL(changed()),
//               this, SLOT(updateUi()));
//    disconnect(m_trans, SIGNAL(eulaRequired(QString,QString,QString,QString)),
//               this, SLOT(eulaRequired(QString,QString,QString,QString)));
//    disconnect(m_trans, SIGNAL(mediaChangeRequired(PackageKit::Transaction::MediaType,QString,QString)),
//               this, SLOT(mediaChangeRequired(PackageKit::Transaction::MediaType,QString,QString)));
//    disconnect(m_trans, SIGNAL(repoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,PackageKit::Transaction::SigType)),
//               this, SLOT(repoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,PackageKit::Transaction::SigType)));
}

void PkTransaction::requeueTransaction()
{
    switch (d->originalRole) {
    case Transaction::RoleRemovePackages :
        removePackages();
        break;
    case Transaction::RoleInstallPackages :
        installPackages();
        break;
    case Transaction::RoleInstallFiles :
        installFiles();
        break;
    case Transaction::RoleUpdatePackages :
        updatePackages();
        break;
    default :
        setExitStatus(Failed);
        return;
    }
}

void PkTransaction::updateUi()
{
    Transaction *transaction = qobject_cast<Transaction*>(sender());
    if (transaction == 0 && (transaction = m_trans) == 0) {
        kWarning() << "no transaction object";
        return;
    }

    uint percentage = transaction->percentage();
    kDebug() << percentage;
    if (percentage <= 100) {
//        ui->progressBar->setMaximum(100);
//        ui->progressBar->setValue(percentage);
//    } else if (ui->progressBar->maximum() != 0) {
//        ui->progressBar->setMaximum(0);
//        ui->progressBar->reset();
    }

//    ui->progressBar->setRemaining(transaction->remainingTime());

    // Status & Speed
    Transaction::Status status = transaction->status();
    if (m_status != status) {
        m_status = status;
//        ui->currentL->setText(PkStrings::status(status));

//        KPixmapSequence sequence = KPixmapSequence(PkIcons::statusAnimation(status),
//                                                   KIconLoader::SizeLarge);
//        if (sequence.isValid()) {
//            d->busySeq->setSequence(sequence);
//            d->busySeq->start();
//        }
    } else if (status == Transaction::StatusDownload) {
        uint speed = transaction->speed();
        qulonglong downloadRemaining = transaction->downloadSizeRemaining();
//        if (speed != 0 && downloadRemaining != 0) {
//            ui->currentL->setText(i18n("Downloading packages at %1/s, %2 remaining",
//                                       KGlobal::locale()->formatByteSize(speed),
//                                       KGlobal::locale()->formatByteSize(downloadRemaining)));
//        } else if (speed != 0 && downloadRemaining == 0) {
//            ui->currentL->setText(i18n("Downloading packages at %1/s",
//                                         KGlobal::locale()->formatByteSize(speed)));
//        } else if (speed == 0 && downloadRemaining != 0) {
//            ui->currentL->setText(i18n("Downloading packages, %1 remaining",
//                                         KGlobal::locale()->formatByteSize(downloadRemaining)));
//        }
    }

    Transaction::Role role = transaction->role();
    if (d->role != role &&
        role != Transaction::RoleUnknown) {
        d->role = role;
//        setWindowTitle(PkStrings::action(role));
        emit titleChanged(PkStrings::action(role));

        // The role changed let's change the connections too
        unsetTransaction();
        // Setup the simulate model if we are simulating
        if (d->flags & Transaction::TransactionFlagSimulate) {
            // DISCONNECT THIS SIGNAL BEFORE SETTING A NEW ONE
            d->simulateModel->clear();
    //        d->simulateModel->setSkipPackages(d->packages);
            connect(m_trans, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                    d->simulateModel, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
        } else if (role == Transaction::RoleInstallPackages ||
                   role == Transaction::RoleInstallFiles ||
                   role == Transaction::RoleRemovePackages ||
                   role == Transaction::RoleUpdatePackages ||
                   role == Transaction::RoleRefreshCache) {
//            connect(m_trans, SIGNAL(repoDetail(QString,QString,bool)),
//                    d->progressModel, SLOT(currentRepo(QString,QString,bool)));
//            connect(m_trans, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
//                    d->progressModel, SLOT(currentPackage(PackageKit::Transaction::Info,QString,QString)));
//            connect(m_trans, SIGNAL(itemProgress(QString,PackageKit::Transaction::Status,uint)),
//                    d->progressModel, SLOT(itemProgress(QString,PackageKit::Transaction::Status,uint)));
        }
    }

    // check to see if we can cancel
    bool cancel = transaction->allowCancel();
    emit allowCancel(cancel);
//    ui->cancelButton->setEnabled(cancel);
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
    // obvious message, don't tell the user
    if (m_handlingActionRequired ||
        error == Transaction::ErrorTransactionCancelled ||
        error == Transaction::ErrorProcessKill) {
        return;
    }

    if (untrustedIsNeed(error)) {
        m_handlingActionRequired = true;
//        int ret = KMessageBox::warningYesNo(this,
//                                            i18n("You are about to install unsigned packages that can compromise your system, "
//                                            "as it is impossible to verify if the software came from a trusted "
//                                            "source.\n\nAre you sure you want to proceed with the installation?"),
//                                            i18n("Installing unsigned software"));
//        if (ret == KMessageBox::Yes) {
//            // Set only trusted to false, to do as the user asked
//            d->flags ^= Transaction::TransactionFlagOnlyTrusted;
//            requeueTransaction();
//        } else {
//            setExitStatus(Cancelled);
//        }
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
    showSorry(PkStrings::error(error), PkStrings::errorMessage(error), QString(details).replace('\n', "<br>"));

    // when we receive an error we are done
    setExitStatus(Failed);
}

void PkTransaction::eulaRequired(const QString &eulaID, const QString &packageID, const QString &vendor, const QString &licenseAgreement)
{
    if (m_handlingActionRequired) {
        // if its true means that we alread passed here
        m_handlingActionRequired = false;
        return;
    } else {
        m_handlingActionRequired = true;
    }

//    LicenseAgreement *eula = new LicenseAgreement(eulaID, packageID, vendor, licenseAgreement, this);
//    connect(eula, SIGNAL(yesClicked()), this, SLOT(acceptEula()));
//    connect(eula, SIGNAL(rejected()), this, SLOT(reject()));
//    showDialog(eula);
}

void PkTransaction::acceptEula()
{
    LicenseAgreement *eula = qobject_cast<LicenseAgreement*>(sender());

    if (eula) {
        kDebug() << "Accepting EULA" << eula->id();
        Transaction *trans = new Transaction(this);
        setTransaction(trans, Transaction::RoleAcceptEula);
        trans->acceptEula(eula->id());
        if (trans->error()) {
            showSorry(i18n("Failed to install signature"),
                      PkStrings::daemonError(trans->error()));
        }
    } else {
        kWarning() << "something is broken, slot is bound to LicenseAgreement but signalled from elsewhere.";
    }
}

void PkTransaction::mediaChangeRequired(Transaction::MediaType type, const QString &id, const QString &text)
{
    Q_UNUSED(id)

    m_handlingActionRequired = true;
//    int ret = KMessageBox::questionYesNo(this,
//                                         PkStrings::mediaMessage(type, text),
//                                         i18n("A media change is required"),
//                                         KStandardGuiItem::cont(),
//                                         KStandardGuiItem::cancel());
    m_handlingActionRequired = false;

    // if the user clicked continue we got yes
//    if (ret == KMessageBox::Yes) {
//        requeueTransaction();
//    } else {
//        setExitStatus(Cancelled);
//    }
}

void PkTransaction::repoSignatureRequired(const QString &packageID, const QString &repoName, const QString &keyUrl, const QString &keyUserid, const QString &keyId, const QString &keyFingerprint, const QString &keyTimestamp, Transaction::SigType type)
{
    if (m_handlingActionRequired) {
        // if its true means that we alread passed here
        m_handlingActionRequired = false;
        return;
    } else {
        m_handlingActionRequired = true;
    }

//    RepoSig *repoSig = new RepoSig(packageID, repoName, keyUrl, keyUserid, keyId, keyFingerprint, keyTimestamp, type, this);
//    connect(repoSig, SIGNAL(yesClicked()), this, SLOT(installSignature()));
//    connect(repoSig, SIGNAL(rejected()), this, SLOT(reject()));
//    showDialog(repoSig);
}

void PkTransaction::installSignature()
{
    RepoSig *repoSig = qobject_cast<RepoSig*>(sender());

    if (repoSig)  {
        kDebug() << "Installing Signature" << repoSig->keyID();
        Transaction *trans = new Transaction(this);
        setTransaction(trans, Transaction::RoleInstallSignature);
        trans->installSignature(repoSig->sigType(), repoSig->keyID(), repoSig->packageID());
        if (trans->error()) {
            showSorry(i18n("Failed to install signature"),
                      PkStrings::daemonError(trans->error()));
        }
    } else {
        kWarning() << "something is broken, slot is bound to RepoSig but signalled from elsewhere.";
    }
}

void PkTransaction::transactionFinished(Transaction::Exit status)
{
    Transaction *trans = qobject_cast<Transaction*>(sender());
    Requirements *requires = 0;
    m_trans = 0;

    Transaction::Role role = trans->role();
    kDebug() << status << role;
    d->finished = true;
    switch(status) {
    case Transaction::ExitSuccess :
//        ui->progressBar->setMaximum(100);
//        ui->progressBar->setValue(100);

        // Check if we are just simulating
        if (d->flags & Transaction::TransactionFlagSimulate) {
            // Disable the simulate flag
            d->flags ^= Transaction::TransactionFlagSimulate;
            d->simulateModel->finished();

            kDebug() << "Simulate Finished" << d->simulateModel->rowCount();
            // Remove the transaction packages
            foreach (const QString &packageID, d->packages) {
                kDebug() << "Simulate Finished packageID" << packageID;
                d->simulateModel->removePackage(packageID);
            }
            kDebug() << "Simulate Finished removed" << d->simulateModel->rowCount();

            d->packagesToResolve.append(d->simulateModel->selectedPackagesToInstall());
//            requires = new Requirements(d->simulateModel, this);
//            connect(requires, SIGNAL(rejected()), this, SLOT(reject()));
//            if (requires->shouldShow()) {
//                showDialog(requires);
//            } else {
//                requires->deleteLater();
//                requires = 0;
//            }

            switch (role) {
            case Transaction::RoleInstallPackages:
                if (requires) {
                    connect(requires, SIGNAL(accepted()), this, SLOT(installPackages()));
                } else {
                    installPackages();
                }
                return;
            case Transaction::RoleRemovePackages:
                if (requires) {
                    // As we have requires allow deps removal
                    d->allowDeps = true;
                    connect(requires, SIGNAL(accepted()), this, SLOT(removePackages()));
                } else {
                    removePackages();
                }
                return;
            case Transaction::RoleUpdatePackages:
                if (requires) {
                    connect(requires, SIGNAL(accepted()), this, SLOT(updatePackages()));
                } else {
                    updatePackages();
                }
                return;
            case Transaction::RoleInstallFiles:
                if (requires) {
                    connect(requires, SIGNAL(accepted()), this, SLOT(installFiles()));
                } else {
                    installFiles();
                }
                return;
            default:
                break;
            }
        } else {
            KConfig config("apper");
            KConfigGroup transactionGroup(&config, "Transaction");
            bool showApp = transactionGroup.readEntry("ShowApplicationLauncher", true);
            if (showApp &&
                    !d->packagesToResolve.isEmpty() &&
                    (role == Transaction::RoleInstallPackages ||
                     role == Transaction::RoleInstallFiles ||
                     role == Transaction::RoleRemovePackages ||
                     role == Transaction::RoleUpdatePackages)) {
                // When installing files or updates that involves new packages
                // try to resolve the available packages at simulation time
                // to maybe show the user the new applications that where installed
                if (d->launcher == 0) {
//                    d->launcher = new ApplicationLauncher(this);
                }

                Transaction *transaction = new Transaction(this);
                connect(transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                        d->launcher, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
                setTransaction(transaction, Transaction::RoleResolve);
                transaction->resolve(d->packagesToResolve, Transaction::FilterInstalled);
                if (!transaction->error()) {
                    return; // avoid the exit code
                }
            } else if (showApp &&
                       d->launcher &&
                       !d->launcher->packages().isEmpty() &&
                       role == Transaction::RoleResolve &&
                       d->originalRole != Transaction::RoleUnknown) {
                // Let's try to find some desktop files
                Transaction *transaction = new Transaction(this);
                connect(transaction, SIGNAL(files(QString,QStringList)),
                        d->launcher, SLOT(files(QString,QStringList)));
                setTransaction(transaction, Transaction::RoleGetFiles);
                transaction->getFiles(d->launcher->packages());
                if (!transaction->error()) {
                    return; // avoid the exit code
                }
            } else if (showApp &&
                       d->launcher &&
                       d->launcher->hasApplications() &&
                       role == Transaction::RoleGetFiles &&
                       d->originalRole != Transaction::RoleUnknown) {
//                showDialog(d->launcher);
                connect(d->launcher, SIGNAL(accepted()),
                        this, SLOT(setExitStatus()));
                return;
            } else if (role == Transaction::RoleAcceptEula ||
                       role == Transaction::RoleInstallSignature) {
                kDebug() << "EULA or Signature accepted";
                d->finished = false;
                requeueTransaction();
                return;
            }
            setExitStatus(Success);
        }
        break;
    case Transaction::ExitCancelled :
//        ui->progressBar->setMaximum(100);
//        ui->progressBar->setValue(100);
        // Avoid crash in case we are showing an error
        if (!m_showingError) {
            setExitStatus(Cancelled);
        }
        break;
    case Transaction::ExitFailed :
        if (!m_handlingActionRequired && !m_showingError) {
//            ui->progressBar->setMaximum(0);
//            ui->progressBar->reset();
            kDebug() << "Yep, we failed.";
            setExitStatus(Failed);
        }
        break;
    case Transaction::ExitKeyRequired :
    case Transaction::ExitEulaRequired :
    case Transaction::ExitMediaChangeRequired :
    case Transaction::ExitNeedUntrusted :
        kDebug() << "finished KeyRequired or EulaRequired: " << status;
//        ui->currentL->setText(PkStrings::status(Transaction::StatusSetup));
        if (!m_handlingActionRequired) {
            kDebug() << "Not Handling Required Action";
            setExitStatus(Failed);
        }
        break;
    default :
//        ui->progressBar->setMaximum(100);
//        ui->progressBar->setValue(100);
        kDebug() << "finished default" << status;
        setExitStatus(Failed);
        break;
    }
}

PkTransaction::ExitStatus PkTransaction::exitStatus() const
{
    return m_exitStatus;
}

bool PkTransaction::isFinished() const
{
    kDebug() << status() << role();
    return status() == Transaction::StatusFinished;
//    return d->finished;
}

void PkTransaction::setExitStatus(PkTransaction::ExitStatus status)
{
    kDebug() << status;
    m_exitStatus = status;
    if (!m_handlingActionRequired || !m_showingError) {
        emit finished(status);
    }
}

void PkTransaction::reject()
{
    d->finished = true;
    setExitStatus(Cancelled);
}

void PkTransaction::showError(const QString &title, const QString &description, const QString &details)
{
//    if (ui->cancelButton->isVisible()) {
//        if (details.isEmpty()) {
//            KMessageBox::error(this, description, title);
//        } else {
//            KMessageBox::detailedError(this, description, details, title);
//        }
//    } else {
//        emit error(title, description, details);
//    }
}

void PkTransaction::showSorry(const QString &title, const QString &description, const QString &details)
{
//    if (ui->cancelButton->isVisible()) {
//        if (details.isEmpty()) {
//            KMessageBox::sorry(this, description, title);
//        } else {
//            KMessageBox::detailedSorry(this, description, details, title);
//        }
//    } else {
//        emit sorry(title, description, details);
//    }
}

QString PkTransaction::title() const
{
    return PkStrings::action(d->role);
}

Transaction::Role PkTransaction::role() const
{
    return d->role;
}

Transaction::TransactionFlags PkTransaction::flags() const
{
    return d->flags;
}

PkTransactionProgressModel *PkTransaction::progressModel() const
{
    return d->progressModel;
}

#include "PkTransaction.moc"
