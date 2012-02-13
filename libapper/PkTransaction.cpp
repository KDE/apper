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
#include "ui_PkTransaction.h"

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

#include "Macros.h"
#include "Enum.h"
#include "PkStrings.h"
#include "RepoSig.h"
#include "LicenseAgreement.h"
#include "PkIcons.h"
#include "ProgressView.h"
#include "ApplicationLauncher.h"
#include "SimulateModel.h"
#include "Requirements.h"

class PkTransactionPrivate
{
public:
    bool finished;
    bool allowDeps;
    bool onlyTrusted;
    Transaction::Role role;
    Transaction::Role originalRole;
    Transaction::Error error;
    QList<Package> packages;
    QStringList packagesToResolve;
    ApplicationLauncher *launcher;
    QStringList files;
    SimulateModel *simulateModel;
    KPixmapSequenceOverlayPainter *busySeq;
};

PkTransaction::PkTransaction(QWidget *parent)
 : QWidget(parent),
   m_trans(0),
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
    d->launcher = 0;
    d->originalRole = Transaction::UnknownRole;
    d->role = Transaction::UnknownRole;
    
    connect(ui->cancelButton, SIGNAL(rejected()), this, SLOT(cancel()));
}

PkTransaction::~PkTransaction()
{
    // DO NOT disconnect the transaction here,
    // it might not exist when this happen
    delete d;
}

void PkTransaction::hideCancelButton()
{
    ui->cancelButton->hide();
}

void PkTransaction::installFiles(const QStringList &files)
{
    if (Daemon::actions() & Transaction::RoleInstallFiles) {
        d->originalRole = Transaction::RoleInstallFiles;
        if (Daemon::actions() & Transaction::RoleSimulateInstallFiles) {
            d->files         = files;
            d->simulateModel = new SimulateModel(this, d->packages);

            // Create the simulate transaction and it's model
            Transaction *trans = new Transaction(this);
            setTransaction(trans, Transaction::RoleSimulateInstallFiles);
            trans->simulateInstallFiles(files);
            if (trans->error()) {
                showSorry(i18n("Failed to simulate file install"),
                          PkStrings::daemonError(trans->error()));
            }
        } else {
            installFiles();
        }
    } else {
        showError(i18n("Current backend does not support installing files."), i18n("Error"));
    }
}

void PkTransaction::installPackages(const QList<Package> &packages)
{
    if (Daemon::actions() & Transaction::RoleInstallPackages) {
        d->originalRole = Transaction::RoleInstallPackages;
        if (Daemon::actions() & Transaction::RoleSimulateInstallPackages) {
            d->packages      = packages;
            d->simulateModel = new SimulateModel(this, d->packages);

            // Create the depends transaction and it's model
            Transaction *trans = new Transaction(this);
            setTransaction(trans, Transaction::RoleSimulateInstallPackages);
            trans->simulateInstallPackages(d->packages);
            if (trans->error()) {
                showSorry(i18n("Failed to simulate package install"),
                          PkStrings::daemonError(trans->error()));
            }
        } else {
            installPackages();
        }
    } else {
        showError(i18n("Current backend does not support installing packages."), i18n("Error"));
    }
}

void PkTransaction::removePackages(const QList<Package> &packages)
{
    if (Daemon::actions() & Transaction::RoleRemovePackages) {
        d->originalRole = Transaction::RoleRemovePackages;
        d->allowDeps = false; // Default to avoid dependencies removal unless simulate says so
        if (Daemon::actions() & Transaction::RoleSimulateRemovePackages) {
            d->packages      = packages;
            d->simulateModel = new SimulateModel(this, d->packages);

            // Create the requirements transaction and it's model
            Transaction *trans = new Transaction(this);
            setTransaction(trans, Transaction::RoleSimulateRemovePackages);
            trans->simulateRemovePackages(d->packages, AUTOREMOVE);
            if (trans->error()) {
                showSorry(i18n("Failed to simulate package removal"),
                          PkStrings::daemonError(trans->error()));
            }
        } else {
            // As we can't check for requires don't allow deps removal
            removePackages();
        }
    } else {
        showError(i18n("The current backend does not support removing packages."), i18n("Error"));
    }
}

void PkTransaction::updatePackages(const QList<Package> &packages)
{
    if (Daemon::actions() & Transaction::RoleUpdatePackages) {
        d->originalRole = Transaction::RoleUpdatePackages;
        if (Daemon::actions() & Transaction::RoleSimulateUpdatePackages) {
            d->packages      = packages;
            d->simulateModel = new SimulateModel(this, d->packages);

            Transaction *trans = new Transaction(this);
            setTransaction(trans, Transaction::RoleSimulateUpdatePackages);
            trans->simulateUpdatePackages(d->packages);
            if (trans->error()) {
                showSorry(i18n("Failed to simulate package update"),
                          PkStrings::daemonError(trans->error()));
            }
        } else {
            updatePackages();
        }
    } else {
        showError(i18n("The current backend does not support updating packages."), i18n("Error"));
    }
}

void PkTransaction::refreshCache()
{
    SET_PROXY
    Transaction *trans = new Transaction(this);
    setTransaction(trans, Transaction::RoleRefreshCache);
    trans->refreshCache(true);
    if (trans->error()) {
        showSorry(i18n("Failed to refresh package cache"),
                  PkStrings::daemonError(trans->error()));
    }
}

void PkTransaction::setupTransaction(PackageKit::Transaction *transaction)
{
    SET_PROXY;

#ifdef HAVE_DEBCONFKDE
    QString socket;
    socket = QLatin1String("/tmp/debconf_") + transaction->tid().remove('/');
    QDBusMessage message;
    message = QDBusMessage::createMethodCall("org.kde.ApperSentinel",
                                             "/",
                                             "org.kde.ApperSentinel",
                                             QLatin1String("SetupDebconfDialog"));
    // Use our own cached tid to avoid crashes
    message << qVariantFromValue(socket);
    message << qVariantFromValue(static_cast<uint>(effectiveWinId()));
    QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        kWarning() << "Message did not receive a reply";
    }

    transaction->setHints("frontend-socket=" + socket);
#else
     Q_UNUSED(transaction)
#endif //HAVE_DEBCONFKDE
}

void PkTransaction::installPackages()
{
    Transaction *trans = new Transaction(this);
    setupTransaction(trans);
    setTransaction(trans, Transaction::RoleInstallPackages);
    trans->installPackages(d->packages, d->onlyTrusted);
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
    trans->installFiles(d->files, d->onlyTrusted);
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
    trans->removePackages(d->packages, d->allowDeps, AUTOREMOVE);
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
    trans->updatePackages(d->packages, true);
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
    if (!trans) {
        // 0 pointer passed
        return;
    }

    m_trans = trans;
    d->role = role;
    d->finished = false;
    m_handlingActionRequired = false;
    m_showingError = false;
    d->error = Transaction::UnknownError;
    ui->progressView->clear();

    if (role != Transaction::UnknownRole) {
        setWindowTitle(PkStrings::action(role));
        emit titleChanged(PkStrings::action(role));
    }

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

        if (d->simulateModel) {
            d->packagesToResolve.append(d->simulateModel->newPackages());
            d->simulateModel->deleteLater();
            d->simulateModel = 0;
        }
    } else if (role == Transaction::RoleSimulateInstallPackages ||
               role == Transaction::RoleSimulateInstallFiles ||
               role == Transaction::RoleSimulateRemovePackages ||
               role == Transaction::RoleSimulateUpdatePackages) {
        // DISCONNECT THIS SIGNAL BEFORE SETTING A NEW ONE
        if (d->simulateModel == 0) {
            d->simulateModel = new SimulateModel(this, d->packages);
        }
        d->simulateModel->clear();
        connect(m_trans, SIGNAL(package(PackageKit::Package)),
                d->simulateModel, SLOT(addPackage(PackageKit::Package)));
    }

    // sets the action icon to be the window icon
    setWindowIcon(PkIcons::actionIcon(role));

    // Now sets the last package
    ui->progressView->currentPackage(m_trans->lastPackage());

    // sets ui
    updateUi();

    // DISCONNECT ALL THESE SIGNALS BEFORE SETTING A NEW ONE
    connect(m_trans, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
            this, SLOT(transactionFinished(PackageKit::Transaction::Exit)));
    connect(m_trans, SIGNAL(errorCode(PackageKit::Transaction::Error, QString)),
            this, SLOT(errorCode(PackageKit::Transaction::Error, QString)));
    connect(m_trans, SIGNAL(changed()),
            this, SLOT(updateUi()));
    connect(m_trans, SIGNAL(eulaRequired(PackageKit::Eula)),
            this, SLOT(eulaRequired(PackageKit::Eula)));
    connect(m_trans, SIGNAL(mediaChangeRequired(PackageKit::Transaction::MediaType, QString, QString)),
            this, SLOT(mediaChangeRequired(PackageKit::Transaction::MediaType, QString, QString)));
    connect(m_trans, SIGNAL(repoSignatureRequired(PackageKit::Signature)),
            this, SLOT(repoSignatureRequired(PackageKit::Signature)));
    // DISCONNECT ALL THESE SIGNALS BEFORE SETTING A NEW ONE
}

void PkTransaction::unsetTransaction()
{
    if (m_trans == 0) {
        return;
    }

    disconnect(m_trans, SIGNAL(package(PackageKit::Package)),
               d->simulateModel, SLOT(addPackage(PackageKit::Package)));
    disconnect(m_trans, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
               this, SLOT(transactionFinished(PackageKit::Transaction::Exit)));
    disconnect(m_trans, SIGNAL(errorCode(PackageKit::Transaction::Error, QString)),
               this, SLOT(errorCode(PackageKit::Transaction::Error, QString)));
    disconnect(m_trans, SIGNAL(changed()),
               this, SLOT(updateUi()));
    disconnect(m_trans, SIGNAL(eulaRequired(PackageKit::Eula)),
               this, SLOT(eulaRequired(PackageKit::Eula)));
    disconnect(m_trans, SIGNAL(mediaChangeRequired(PackageKit::Transaction::MediaType, QString, QString)),
               this, SLOT(mediaChangeRequired(PackageKit::Transaction::MediaType, QString, QString)));
    disconnect(m_trans, SIGNAL(repoSignatureRequired(PackageKit::Signature)),
               this, SLOT(repoSignatureRequired(PackageKit::Signature)));
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
    if (percentage <= 100) {
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(percentage);
    } else if (ui->progressBar->maximum() != 0) {
        ui->progressBar->setMaximum(0);
        ui->progressBar->reset();
    }

    ui->progressView->setSubProgress(transaction->subpercentage());
    ui->progressBar->setRemaining(transaction->remainingTime());

    // Status & Speed
    Transaction::Status status = transaction->status();
    if (m_status != status) {
        m_status = status;
        ui->currentL->setText(PkStrings::status(status));

        KPixmapSequence sequence = KPixmapSequence(PkIcons::statusAnimation(status),
                                                   KIconLoader::SizeLarge);
        if (sequence.isValid()) {
            d->busySeq->setSequence(sequence);
            d->busySeq->start();
        }
    } else if (status == Transaction::StatusDownload && transaction->speed() != 0) {
        uint speed = transaction->speed();
        if (speed) {
            ui->currentL->setText(i18n("Downloading packages at %1/s",
                                         KGlobal::locale()->formatByteSize(speed)));
        }
    }

    Transaction::Role role = transaction->role();
    if (d->role != role &&
        role != Transaction::UnknownRole) {
        d->role = role;
        setWindowTitle(PkStrings::action(role));
        emit titleChanged(PkStrings::action(role));
    }

    // check to see if we can cancel
    bool cancel = transaction->allowCancel();
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
    showSorry(PkStrings::error(error), PkStrings::errorMessage(error), QString(details).replace('\n', "<br>"));

    // when we receive an error we are done
    setExitStatus(Failed);
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

    LicenseAgreement *eula = new LicenseAgreement(info, this);
    connect(eula, SIGNAL(yesClicked()), this, SLOT(acceptEula()));
    connect(eula, SIGNAL(rejected()), this, SLOT(reject()));
    showDialog(eula);
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
    int ret = KMessageBox::questionYesNo(this,
                                         PkStrings::mediaMessage(type, text),
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

    RepoSig *repoSig = new RepoSig(info, this);
    connect(repoSig, SIGNAL(yesClicked()), this, SLOT(installSignature()));
    connect(repoSig, SIGNAL(rejected()), this, SLOT(reject()));
    showDialog(repoSig);
}

void PkTransaction::installSignature()
{
    RepoSig *repoSig = qobject_cast<RepoSig*>(sender());

    if (repoSig)  {
        kDebug() << "Installing Signature" << repoSig->signature().keyId;
        Transaction *trans = new Transaction(this);
        setTransaction(trans, Transaction::RoleInstallSignature);
        trans->installSignature(repoSig->signature());
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

    kDebug() << status << trans->role();
    d->finished = true;
    switch(status) {
    case Transaction::ExitSuccess :
    {
        Transaction::Role role = trans->role();
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(100);

        // If the simulate model exists we were simulating
        if (d->simulateModel) {
            kDebug() << "We have a simulate model";
            requires = new Requirements(d->simulateModel, this);
            connect(requires, SIGNAL(rejected()), this, SLOT(reject()));
            if (requires->shouldShow()) {
                showDialog(requires);
            } else {
                requires->deleteLater();
                requires = 0;
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
                    // As we have requires allow deps removal
                    d->allowDeps = true;
                    connect(requires, SIGNAL(accepted()), this, SLOT(removePackages()));
                } else {
                    removePackages();
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
                if (requires) {
                    connect(requires, SIGNAL(accepted()), this, SLOT(installFiles()));
                } else {
                    installFiles();
                }
                return;
            default:
                break;
            }
        }

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
                d->launcher = new ApplicationLauncher(this);
            }

            Transaction *transaction = new Transaction(this);
            connect(transaction, SIGNAL(package(PackageKit::Package)),
                    d->launcher, SLOT(addPackage(PackageKit::Package)));
            setTransaction(transaction, Transaction::RoleResolve);
            transaction->resolve(d->packagesToResolve, Transaction::FilterInstalled);
            if (!transaction->error()) {
                return; // avoid the exit code
            }
        } else if (showApp &&
                   d->launcher &&
                   !d->launcher->packages().isEmpty() &&
                   role == Transaction::RoleResolve &&
                   d->originalRole != Transaction::UnknownRole) {
            // Let's try to find some desktop files
            Transaction *transaction = new Transaction(this);
            connect(transaction, SIGNAL(files(PackageKit::Package, QStringList)),
                    d->launcher, SLOT(files(PackageKit::Package,QStringList)));
            setTransaction(transaction, Transaction::RoleGetFiles);
            transaction->getFiles(d->launcher->packages());
            if (!transaction->error()) {
                return; // avoid the exit code
            }
        } else if (showApp &&
                   d->launcher &&
                   d->launcher->hasApplications() &&
                   role == Transaction::RoleGetFiles &&
                   d->originalRole != Transaction::UnknownRole) {
            showDialog(d->launcher);
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
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(100);
        // Avoid crash in case we are showing an error
        if (!m_showingError) {
            setExitStatus(Cancelled);
        }
        break;
    case Transaction::ExitFailed :
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
        ui->currentL->setText(PkStrings::status(Transaction::StatusSetup));
        if (!m_handlingActionRequired) {
            kDebug() << "Not Handling Required Action";
            setExitStatus(Failed);
        }
        break;
    default :
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(100);
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
    return d->finished;
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

void PkTransaction::showDialog(KDialog *dlg)
{
    if (ui->cancelButton->isVisible()) {
        dlg->setModal(true);
        dlg->show();
    } else {
        dlg->setProperty("embedded", true);
        emit dialog(dlg);
    }
}

void PkTransaction::showError(const QString &title, const QString &description, const QString &details)
{
    if (ui->cancelButton->isVisible()) {
        if (details.isEmpty()) {
            KMessageBox::error(this, description, title);
        } else {
            KMessageBox::detailedError(this, description, details, title);
        }
    } else {
        emit error(title, description, details);
    }
}

void PkTransaction::showSorry(const QString &title, const QString &description, const QString &details)
{
    if (ui->cancelButton->isVisible()) {
        if (details.isEmpty()) {
            KMessageBox::sorry(this, description, title);
        } else {
            KMessageBox::detailedSorry(this, description, details, title);
        }
    } else {
        emit sorry(title, description, details);
    }
}

QString PkTransaction::title() const
{
    return PkStrings::action(d->role);
}

Transaction::Role PkTransaction::role() const
{
    return d->role;
}

#include "PkTransaction.moc"
