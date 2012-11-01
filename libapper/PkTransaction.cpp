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
#include "PkTransactionWidget.h"

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
    QWidget *parentWindow;
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
    d->simulateModel = 0;
    d->launcher = 0;
    d->originalRole = Transaction::RoleUnknown;
    d->role = Transaction::RoleUnknown;
    d->parentWindow = 0;

    // for sanity we are trusted till an error is given and the user accepts
    d->flags = Transaction::TransactionFlagOnlyTrusted;
    d->progressModel = new PkTransactionProgressModel(this);
    connect(this, SIGNAL(repoDetail(QString,QString,bool)),
            d->progressModel, SLOT(currentRepo(QString,QString,bool)));
    connect(this, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            d->progressModel, SLOT(currentPackage(PackageKit::Transaction::Info,QString)));
    connect(this, SIGNAL(itemProgress(QString,PackageKit::Transaction::Status,uint)),
            d->progressModel, SLOT(itemProgress(QString,PackageKit::Transaction::Status,uint)));
    connect(this, SIGNAL(repoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,PackageKit::Transaction::SigType)),
            SLOT(handleRepoSignature(QString,QString,QString,QString,QString,QString,QString,PackageKit::Transaction::SigType)));

    connect(this, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            SLOT(transactionFinished(PackageKit::Transaction::Exit)));
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

        setupTransaction();
        Transaction::installFiles(files, d->flags);
        if (error()) {
            showSorry(i18n("Failed to simulate file install"),
                      PkStrings::daemonError(error()));
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

        setupTransaction();
        Transaction::installPackages(d->packages, d->flags);
        if (error()) {
            showSorry(i18n("Failed to simulate package install"),
                      PkStrings::daemonError(error()));
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

        setupTransaction();
        Transaction::removePackages(d->packages, d->allowDeps, AUTOREMOVE, d->flags);
        if (error()) {
            showSorry(i18n("Failed to simulate package removal"),
                      PkStrings::daemonError(error()));
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

        setupTransaction();
        Transaction::updatePackages(d->packages, d->flags);
        if (error()) {
            showSorry(i18n("Failed to simulate package update"),
                      PkStrings::daemonError(error()));
        }
    } else {
        showError(i18n("The current backend does not support updating packages."), i18n("Error"));
    }
}

void PkTransaction::setupTransaction()
{
    reset();
    if (d->flags & Transaction::TransactionFlagSimulate) {
        d->simulateModel = new PackageModel(this);
        connect(this, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                d->simulateModel, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
    }

#ifdef HAVE_DEBCONFKDE
    QString _tid = tid();
    QString socket;
    // Build a socket path like /tmp/1761_edeceabd_data_debconf
    socket = QLatin1String("/tmp") % _tid % QLatin1String("_debconf");
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.kde.ApperSentinel"),
                                             QLatin1String("/"),
                                             QLatin1String("org.kde.ApperSentinel"),
                                             QLatin1String("SetupDebconfDialog"));
    // Use our own cached tid to avoid crashes
    message << qVariantFromValue(_tid);
    message << qVariantFromValue(socket);
    message << qVariantFromValue(static_cast<uint>(effectiveWinId()));
    QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        kWarning() << "Message did not receive a reply";
    }

    setHints(QLatin1String("frontend-socket=") % socket);
#endif //HAVE_DEBCONFKDE
}

void PkTransaction::installPackages()
{
    setupTransaction();
    Transaction::installPackages(d->packages, d->flags);
    if (error()) {
        showSorry(i18n("Failed to install package"),
                  PkStrings::daemonError(error()));
    }
}

void PkTransaction::installFiles()
{
    setupTransaction();
    Transaction::installFiles(d->files, d->flags);
    if (error()) {
        showSorry(i18np("Failed to install file",
                        "Failed to install files", d->files.size()),
                  PkStrings::daemonError(error()));
    }
}

void PkTransaction::removePackages()
{
    setupTransaction();
    Transaction::removePackages(d->packages, d->allowDeps, AUTOREMOVE, d->flags);
    if (error()) {
        showSorry(i18n("Failed to remove package"),
                  PkStrings::daemonError(error()));
    }
}

void PkTransaction::updatePackages()
{
    setupTransaction();
    Transaction::updatePackages(d->packages, d->flags);
    if (error()) {
        showSorry(i18n("Failed to update package"),
                  PkStrings::daemonError(error()));
    }
}

void PkTransaction::requeueTransaction()
{
    Requirements *requires = qobject_cast<Requirements *>(sender());
    switch (d->originalRole) {
    case Transaction::RoleRemovePackages:
        if (requires) {
            // As we have requires allow deps removal
            d->allowDeps = true;
        }
        removePackages();
        break;
    case Transaction::RoleInstallPackages:
        installPackages();
        break;
    case Transaction::RoleInstallFiles:
        installFiles();
        break;
    case Transaction::RoleUpdatePackages:
        updatePackages();
        break;
    default :
        setExitStatus(Failed);
        return;
    }
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
        int ret = KMessageBox::warningYesNo(d->parentWindow,
                                            i18n("You are about to install unsigned packages that can compromise your system, "
                                            "as it is impossible to verify if the software came from a trusted "
                                            "source.\n\nAre you sure you want to proceed with the installation?"),
                                            i18n("Installing unsigned software"));
        if (ret == KMessageBox::Yes) {
            // Set only trusted to false, to do as the user asked
            d->flags ^= Transaction::TransactionFlagOnlyTrusted;
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

void PkTransaction::eulaRequired(const QString &eulaID, const QString &packageID, const QString &vendor, const QString &licenseAgreement)
{
    if (m_handlingActionRequired) {
        // if its true means that we alread passed here
        m_handlingActionRequired = false;
        return;
    } else {
        m_handlingActionRequired = true;
    }

    LicenseAgreement *eula = new LicenseAgreement(eulaID, packageID, vendor, licenseAgreement, d->parentWindow);
    connect(eula, SIGNAL(yesClicked()), this, SLOT(acceptEula()));
    connect(eula, SIGNAL(rejected()), this, SLOT(reject()));
    showDialog(eula);
}

void PkTransaction::acceptEula()
{
    LicenseAgreement *eula = qobject_cast<LicenseAgreement*>(sender());

    if (eula) {
        kDebug() << "Accepting EULA" << eula->id();
        reset();
        Transaction::acceptEula(eula->id());
        if (error()) {
            showSorry(i18n("Failed to install signature"),
                      PkStrings::daemonError(error()));
        }
    } else {
        kWarning() << "something is broken, slot is bound to LicenseAgreement but signalled from elsewhere.";
    }
}

void PkTransaction::mediaChangeRequired(Transaction::MediaType type, const QString &id, const QString &text)
{
    Q_UNUSED(id)

    m_handlingActionRequired = true;
    int ret = KMessageBox::questionYesNo(d->parentWindow,
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

void PkTransaction::handleRepoSignature(const QString &packageID,
                                        const QString &repoName,
                                        const QString &keyUrl,
                                        const QString &keyUserid,
                                        const QString &keyId,
                                        const QString &keyFingerprint,
                                        const QString &keyTimestamp,
                                        Transaction::SigType type)
{
    if (m_handlingActionRequired) {
        // if its true means that we alread passed here
        m_handlingActionRequired = false;
        return;
    } else {
        m_handlingActionRequired = true;
    }

    RepoSig *repoSig = new RepoSig(packageID, repoName, keyUrl, keyUserid, keyId, keyFingerprint, keyTimestamp, type, d->parentWindow);
    connect(repoSig, SIGNAL(yesClicked()), this, SLOT(installSignature()));
    connect(repoSig, SIGNAL(rejected()), this, SLOT(reject()));
    showDialog(repoSig);
}

void PkTransaction::installSignature()
{
    RepoSig *repoSig = qobject_cast<RepoSig*>(sender());

    if (repoSig)  {
        kDebug() << "Installing Signature" << repoSig->keyID();
        reset();
        Transaction::installSignature(repoSig->sigType(), repoSig->keyID(), repoSig->packageID());
        if (error()) {
            showSorry(i18n("Failed to install signature"),
                      PkStrings::daemonError(error()));
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
    case Transaction::ExitSuccess:
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
            requires = new Requirements(d->simulateModel, d->parentWindow);
            connect(requires, SIGNAL(accepted()), this, SLOT(requeueTransaction()));
            connect(requires, SIGNAL(rejected()), this, SLOT(reject()));
            if (requires->shouldShow()) {
                showDialog(requires);
            } else {
                d->simulateModel->deleteLater();
                d->simulateModel = 0;
                requires->deleteLater();
                requires = 0;

                // Since we removed the Simulate Flag this will procced
                // with the actual action
                requeueTransaction();
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

                // TODO fix this
//                Transaction *transaction = new Transaction(this);
//                connect(transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
//                        d->launcher, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
//                setTransaction(transaction, Transaction::RoleResolve);
//                transaction->resolve(d->packagesToResolve, Transaction::FilterInstalled);
//                if (!transaction->error()) {
//                    return; // avoid the exit code
//                }
            } else if (showApp &&
                       d->launcher &&
                       !d->launcher->packages().isEmpty() &&
                       role == Transaction::RoleResolve &&
                       d->originalRole != Transaction::RoleUnknown) {
                // Let's try to find some desktop files
                // TODO fix this too
//                Transaction *transaction = new Transaction(this);
//                connect(transaction, SIGNAL(files(QString,QStringList)),
//                        d->launcher, SLOT(files(QString,QStringList)));
//                setTransaction(transaction, Transaction::RoleGetFiles);
//                transaction->getFiles(d->launcher->packages());
//                if (!transaction->error()) {
//                    return; // avoid the exit code
//                }
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

void PkTransaction::showDialog(KDialog *dlg)
{
    kDebug() << dlg;
    PkTransactionWidget *widget = qobject_cast<PkTransactionWidget *>(d->parentWindow);
    if (!widget || widget->isCancelVisible()) {
        dlg->setModal(true);
        dlg->show();
    } else {
        dlg->setProperty("embedded", true);
        emit dialog(dlg);
    }
}

void PkTransaction::showError(const QString &title, const QString &description, const QString &details)
{
    PkTransactionWidget *widget = qobject_cast<PkTransactionWidget *>(d->parentWindow);
    if (!widget || widget->isCancelVisible()) {
        if (details.isEmpty()) {
            KMessageBox::error(d->parentWindow, description, title);
        } else {
            KMessageBox::detailedError(d->parentWindow, description, details, title);
        }
    } else {
        emit errorMessage(title, description, details);
    }
}

void PkTransaction::showSorry(const QString &title, const QString &description, const QString &details)
{
    PkTransactionWidget *widget = qobject_cast<PkTransactionWidget *>(d->parentWindow);
    if (!widget || widget->isCancelVisible()) {
        if (details.isEmpty()) {
            KMessageBox::sorry(d->parentWindow, description, title);
        } else {
            KMessageBox::detailedSorry(d->parentWindow, description, details, title);
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

Transaction::TransactionFlags PkTransaction::flags() const
{
    return d->flags;
}

PkTransactionProgressModel *PkTransaction::progressModel() const
{
    return d->progressModel;
}

#include "PkTransaction.moc"
