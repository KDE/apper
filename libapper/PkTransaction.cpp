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

#include <KLocalizedString>
#include <KMessageBox>
#include <KPushButton>
#include <KPixmapSequence>
#include <KConfig>

#include <KDebug>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QTreeView>

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
    bool allowDeps;
    bool jobWatcher;
    bool handlingActionRequired;
    bool showingError; //This might replace the above
    qulonglong downloadSizeRemaining;
    PkTransaction::ExitStatus exitStatus;
    Transaction::Status status;
    Transaction::TransactionFlags flags;
    Transaction::Role originalRole;
    Transaction::Error error;
    Transaction::Role role;
    QStringList packages;
    ApplicationLauncher *launcher;
    QStringList files;
    QStringList newPackages;
    PackageModel *simulateModel;
    PkTransactionProgressModel *progressModel;
    QWidget *parentWindow;
    QDBusObjectPath tid;
    Transaction *transaction;
};

PkTransaction::PkTransaction(QObject *parent) :
    QObject(parent),
    d(new PkTransactionPrivate)
{
    // for sanity we are finished till some transaction is set
    d->allowDeps = false;
    d->jobWatcher = false;
    d->handlingActionRequired = false;
    d->showingError = false;
    d->downloadSizeRemaining = 0;
    d->exitStatus = Success;
    d->status = Transaction::StatusUnknown;
    // for sanity we are trusted till an error is given and the user accepts
    d->flags = Transaction::TransactionFlagOnlyTrusted;
    d->originalRole = Transaction::RoleUnknown;
    d->role = Transaction::RoleUnknown;
    d->error = Transaction::ErrorUnknown;
    d->launcher = 0;
    d->simulateModel = 0;
    d->progressModel = new PkTransactionProgressModel(this);
    d->parentWindow = qobject_cast<QWidget*>(parent);
    d->transaction = 0;
}

PkTransaction::~PkTransaction()
{
    // DO NOT disconnect the transaction here,
    // it might not exist when this happen
    delete d;
}

void PkTransaction::installFiles(const QStringList &files)
{
//    if (Daemon::global()->roles() & Transaction::RoleInstallFiles) {
        d->originalRole = Transaction::RoleInstallFiles;
        d->files = files;
        d->flags = Transaction::TransactionFlagOnlyTrusted | Transaction::TransactionFlagSimulate;

        setupTransaction(Daemon::installFiles(files, d->flags));
//    } else {
//        showError(i18n("Current backend does not support installing files."), i18n("Error"));
//    }
}

void PkTransaction::installPackages(const QStringList &packages)
{
//    if (Daemon::global()->roles() & Transaction::RoleInstallPackages) {
        d->originalRole = Transaction::RoleInstallPackages;
        d->packages = packages;
        d->flags = Transaction::TransactionFlagOnlyTrusted | Transaction::TransactionFlagSimulate;

        setupTransaction(Daemon::installPackages(d->packages, d->flags));
//    } else {
//        showError(i18n("Current backend does not support installing packages."), i18n("Error"));
//    }
}

void PkTransaction::removePackages(const QStringList &packages)
{
//    if (Daemon::global()->roles() & Transaction::RoleRemovePackages) {
        d->originalRole = Transaction::RoleRemovePackages;
        d->allowDeps = false; // Default to avoid dependencies removal unless simulate says so
        d->packages = packages;
        d->flags = Transaction::TransactionFlagOnlyTrusted | Transaction::TransactionFlagSimulate;

        setupTransaction(Daemon::removePackages(d->packages, d->allowDeps, AUTOREMOVE, d->flags));
//    } else {
//        showError(i18n("The current backend does not support removing packages."), i18n("Error"));
//    }
}

void PkTransaction::updatePackages(const QStringList &packages, bool downloadOnly)
{
//    if (Daemon::global()->roles() & Transaction::RoleUpdatePackages) {
        d->originalRole = Transaction::RoleUpdatePackages;
        d->packages = packages;
        if (downloadOnly) {
            // Don't simulate if we are just downloading
            d->flags = Transaction::TransactionFlagOnlyDownload;
        } else {
            d->flags = Transaction::TransactionFlagOnlyTrusted | Transaction::TransactionFlagSimulate;
        }

        setupTransaction(Daemon::updatePackages(d->packages, d->flags));
//    } else {
//        showError(i18n("The current backend does not support updating packages."), i18n("Error"));
//    }
}

void PkTransaction::refreshCache(bool force)
{
    setupTransaction(Daemon::refreshCache(force));
}

void PkTransaction::installPackages()
{
    setupTransaction(Daemon::installPackages(d->packages, d->flags));
}

void PkTransaction::installFiles()
{
    setupTransaction(Daemon::installFiles(d->files, d->flags));
}

void PkTransaction::removePackages()
{
    setupTransaction(Daemon::removePackages(d->packages, d->allowDeps, AUTOREMOVE, d->flags));
}

void PkTransaction::updatePackages()
{
    setupTransaction(Daemon::updatePackages(d->packages, d->flags));
}

void PkTransaction::requeueTransaction()
{
    Requirements *requires = qobject_cast<Requirements *>(sender());
    if (requires) {
        // As we have requires allow deps removal
        d->allowDeps = true;
        if (!requires->trusted()) {
            // Set only trusted to false, to do as the user asked
            setTrusted(false);
        }
    }

    // Delete the simulate model
    if (d->simulateModel) {
        d->simulateModel->deleteLater();
        d->simulateModel = 0;
    }

    // We are not handling any required action yet for the requeued transaction.
    // Without this a second license agreement f.e. does not get shown,
    // see http://bugs.kde.org/show_bug.cgi?id=326619
    d->handlingActionRequired = false;

    switch (d->originalRole) {
    case Transaction::RoleRemovePackages:
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

void PkTransaction::slotErrorCode(Transaction::Error error, const QString &details)
{
    kDebug() << "errorCode: " << error << details;
    d->error = error;

    if (d->handlingActionRequired) {
        // We are already handling required actions
        // like eulaRequired() and repoSignatureRequired()
        return;
    }

    switch (error) {
    case Transaction::ErrorTransactionCancelled:
    case Transaction::ErrorProcessKill:
        // these errors should be ignored
        break;
    case Transaction::ErrorGpgFailure:
    case Transaction::ErrorBadGpgSignature:
    case Transaction::ErrorMissingGpgSignature:
    case Transaction::ErrorCannotInstallRepoUnsigned:
    case Transaction::ErrorCannotUpdateRepoUnsigned:
    {
        d->handlingActionRequired = true;
        int ret = KMessageBox::warningYesNo(d->parentWindow,
                                            i18n("You are about to install unsigned packages that can compromise your system, "
                                            "as it is impossible to verify if the software came from a trusted "
                                            "source.\n\nAre you sure you want to proceed with the installation?"),
                                            i18n("Installing unsigned software"));
        if (ret == KMessageBox::Yes) {
            // Set only trusted to false, to do as the user asked
            setTrusted(false);
            requeueTransaction();
        } else {
            setExitStatus(Cancelled);
        }
        d->handlingActionRequired = false;
        return;
    }
    default:
        d->showingError = true;
        showSorry(PkStrings::error(error), PkStrings::errorMessage(error), QString(details).replace('\n', "<br>"));

        // when we receive an error we are done
        setExitStatus(Failed);
    }
}

void PkTransaction::slotEulaRequired(const QString &eulaID, const QString &packageID, const QString &vendor, const QString &licenseAgreement)
{
    if (d->handlingActionRequired) {
        // if its true means that we alread passed here
        d->handlingActionRequired = false;
        return;
    } else {
        d->handlingActionRequired = true;
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
        setupTransaction(Daemon::acceptEula(eula->id()));
    } else {
        kWarning() << "something is broken, slot is bound to LicenseAgreement but signalled from elsewhere.";
    }
}

void PkTransaction::slotChanged()
{
    Transaction *transaction = qobject_cast<Transaction*>(sender());
    d->downloadSizeRemaining = transaction->downloadSizeRemaining();
    d->role = transaction->role();

    if (!d->jobWatcher) {
        return;
    }

    QDBusObjectPath _tid = transaction->tid();
    if (d->tid != _tid && !(d->flags & Transaction::TransactionFlagSimulate)) {
        d->tid = _tid;
        // if the transaction changed and
        // the user wants the watcher send the tid
        QDBusMessage message;
        message = QDBusMessage::createMethodCall(QLatin1String("org.kde.apperd"),
                                                 QLatin1String("/"),
                                                 QLatin1String("org.kde.apperd"),
                                                 QLatin1String("WatchTransaction"));
        // Use our own cached tid to avoid crashes
        message << qVariantFromValue(_tid);
        if (!QDBusConnection::sessionBus().send(message)) {
            kWarning() << "Failed to put WatchTransaction on the DBus queue";
        }
    }
}

void PkTransaction::slotMediaChangeRequired(Transaction::MediaType type, const QString &id, const QString &text)
{
    Q_UNUSED(id)

    d->handlingActionRequired = true;
    int ret = KMessageBox::questionYesNo(d->parentWindow,
                                         PkStrings::mediaMessage(type, text),
                                         i18n("A media change is required"),
                                         KStandardGuiItem::cont(),
                                         KStandardGuiItem::cancel());
    d->handlingActionRequired = false;

    // if the user clicked continue we got yes
    if (ret == KMessageBox::Yes) {
        requeueTransaction();
    } else {
        setExitStatus(Cancelled);
    }
}

void PkTransaction::slotRepoSignature(const QString &packageID,
                                      const QString &repoName,
                                      const QString &keyUrl,
                                      const QString &keyUserid,
                                      const QString &keyId,
                                      const QString &keyFingerprint,
                                      const QString &keyTimestamp,
                                      Transaction::SigType type)
{
    if (d->handlingActionRequired) {
        // if its true means that we alread passed here
        d->handlingActionRequired = false;
        return;
    } else {
        d->handlingActionRequired = true;
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
        setupTransaction(Daemon::installSignature(repoSig->sigType(), repoSig->keyID(), repoSig->packageID()));
    } else {
        kWarning() << "something is broken, slot is bound to RepoSig but signalled from elsewhere.";
    }
}

void PkTransaction::slotFinished(Transaction::Exit status)
{
    // Clear the model to don't keep trash when reusing the transaction
    d->progressModel->clear();

    Requirements *requires = 0;
    Transaction::Role _role = qobject_cast<Transaction*>(sender())->role();
    d->transaction = 0; // Will be deleted later
    kDebug() << status << _role;

    switch (_role) {
    case Transaction::RoleInstallSignature:
    case Transaction::RoleAcceptEula:
        if (status == Transaction::ExitSuccess) {
            // if the required action was performed with success
            // requeue our main transaction
            requeueTransaction();
            return;
        }
        break;
    default:
        break;
    }

    switch(status) {
    case Transaction::ExitSuccess:
        // Check if we are just simulating
        if (d->flags & Transaction::TransactionFlagSimulate) {
            // Disable the simulate flag
            d->flags ^= Transaction::TransactionFlagSimulate;
            d->simulateModel->finished();

            // Remove the transaction packages
            foreach (const QString &packageID, d->packages) {
                d->simulateModel->removePackage(packageID);
            }

            d->newPackages = d->simulateModel->packagesWithInfo(Transaction::InfoInstalling);
            if (_role == Transaction::RoleInstallPackages) {
                d->newPackages << d->packages;
                d->newPackages.removeDuplicates();
            }

            requires = new Requirements(d->simulateModel, d->parentWindow);
            requires->setDownloadSizeRemaining(d->downloadSizeRemaining);
            connect(requires, SIGNAL(accepted()), this, SLOT(requeueTransaction()));
            connect(requires, SIGNAL(rejected()), this, SLOT(reject()));
            if (requires->shouldShow()) {
                showDialog(requires);
            } else {
                requires->deleteLater();

                // Since we removed the Simulate Flag this will procced
                // with the actual action
                requeueTransaction();
            }
        } else {
            KConfig config("apper");
            KConfigGroup transactionGroup(&config, "Transaction");
            bool showApp = transactionGroup.readEntry("ShowApplicationLauncher", true);
            if (showApp &&
                    !d->newPackages.isEmpty() &&
                    (_role == Transaction::RoleInstallPackages ||
                     _role == Transaction::RoleInstallFiles ||
                     _role == Transaction::RoleRemovePackages ||
                     _role == Transaction::RoleUpdatePackages)) {
                // When installing files or updates that involves new packages
                // try to resolve the available packages at simulation time
                // to maybe show the user the new applications that where installed
                if (d->launcher) {
                    delete d->launcher;
                }
                d->launcher = new ApplicationLauncher(d->parentWindow);
                connect(this, SIGNAL(files(QString,QStringList)),
                        d->launcher, SLOT(files(QString,QStringList)));

                setupTransaction(Daemon::getFiles(d->newPackages));
                d->newPackages.clear();
                return; // avoid the exit code
            } else if (_role == Transaction::RoleGetFiles &&
                       d->launcher &&
                       d->launcher->hasApplications()) {
                // if we have a launcher and the laucher has applications
                // show them to the user
                showDialog(d->launcher);
                connect(d->launcher, SIGNAL(finished()), SLOT(setExitStatus()));
                return;
            }
            setExitStatus(Success);
        }
        break;
    case Transaction::ExitNeedUntrusted:
    case Transaction::ExitKeyRequired:
    case Transaction::ExitEulaRequired:
    case Transaction::ExitMediaChangeRequired:
        kDebug() << "finished KeyRequired or EulaRequired: " << status;
        if (!d->handlingActionRequired) {
            kDebug() << "Not Handling Required Action";
            setExitStatus(Failed);
        }
        break;
    case Transaction::ExitCancelled:
        // Avoid crash in case we are showing an error
        if (!d->showingError) {
            setExitStatus(Cancelled);
        }
        break;
    case Transaction::ExitFailed:
        if (!d->handlingActionRequired && !d->showingError) {
            kDebug() << "Yep, we failed.";
            setExitStatus(Failed);
        }
        break;
    default :
        kDebug() << "finished default" << status;
        setExitStatus(Failed);
        break;
    }
}

PkTransaction::ExitStatus PkTransaction::exitStatus() const
{
    return d->exitStatus;
}

bool PkTransaction::isFinished() const
{
    kDebug() << d->transaction->status() << d->transaction->role();
    return d->transaction->status() == Transaction::StatusFinished;
}

PackageModel *PkTransaction::simulateModel() const
{
    return d->simulateModel;
}

uint PkTransaction::percentage() const
{
    if (d->transaction) {
        return d->transaction->percentage();
    }
    return 0;
}

uint PkTransaction::remainingTime() const
{
    if (d->transaction) {
        return d->transaction->remainingTime();
    }
    return 0;
}

uint PkTransaction::speed() const
{
    if (d->transaction) {
        return d->transaction->speed();
    }
    return 0;
}

qulonglong PkTransaction::downloadSizeRemaining() const
{
    if (d->transaction) {
        return d->transaction->downloadSizeRemaining();
    }
    return 0;
}

Transaction::Status PkTransaction::status() const
{
    if (d->transaction) {
        return d->transaction->status();
    }
    return Transaction::StatusUnknown;
}

Transaction::Role PkTransaction::role() const
{
    if (d->transaction) {
        return d->transaction->role();
    }
    return Transaction::RoleUnknown;
}

bool PkTransaction::allowCancel() const
{
    if (d->transaction) {
        return d->transaction->allowCancel();
    }
    return false;
}

Transaction::TransactionFlags PkTransaction::transactionFlags() const
{
    if (d->transaction) {
        return d->transaction->transactionFlags();
    }
    return Transaction::TransactionFlagNone;
}

void PkTransaction::getUpdateDetail(const QString &packageID)
{
    setupTransaction(Daemon::getUpdateDetail(packageID));
}

void PkTransaction::getUpdates()
{
    setupTransaction(Daemon::getUpdates());
}

void PkTransaction::cancel()
{
    if (d->transaction) {
        d->transaction->cancel();
    }
}

void PkTransaction::setTrusted(bool trusted)
{
    if (trusted) {
        d->flags |= Transaction::TransactionFlagOnlyTrusted;
    } else {
        d->flags ^= Transaction::TransactionFlagOnlyTrusted;
    }
}

void PkTransaction::setExitStatus(PkTransaction::ExitStatus status)
{
    kDebug() << status;
    if (d->launcher) {
        d->launcher->deleteLater();
        d->launcher = 0;
    }

    d->exitStatus = status;
    if (!d->handlingActionRequired || !d->showingError) {
        emit finished(status);
    }
}

void PkTransaction::reject()
{
    setExitStatus(Cancelled);
}

void PkTransaction::setupTransaction(Transaction *transaction)
{
    // Clear the model to don't keep trash when reusing the transaction
    d->progressModel->clear();

    d->transaction = transaction;
    if (!(transaction->transactionFlags() & Transaction::TransactionFlagSimulate) &&
            transaction->role() != Transaction::RoleGetUpdates &&
            transaction->role() != Transaction::RoleGetUpdateDetail) {
        connect(transaction, SIGNAL(repoDetail(QString,QString,bool)),
                d->progressModel, SLOT(currentRepo(QString,QString,bool)));
        connect(transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                d->progressModel, SLOT(currentPackage(PackageKit::Transaction::Info,QString,QString)));
        connect(transaction, SIGNAL(itemProgress(QString,PackageKit::Transaction::Status,uint)),
                d->progressModel, SLOT(itemProgress(QString,PackageKit::Transaction::Status,uint)));
    }

    connect(transaction, SIGNAL(updateDetail(QString,QStringList,QStringList,QStringList,QStringList,QStringList,PackageKit::Transaction::Restart,QString,QString,PackageKit::Transaction::UpdateState,QDateTime,QDateTime)),
            SIGNAL(updateDetail(QString,QStringList,QStringList,QStringList,QStringList,QStringList,PackageKit::Transaction::Restart,QString,QString,PackageKit::Transaction::UpdateState,QDateTime,QDateTime)));
    connect(transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            SIGNAL(package(PackageKit::Transaction::Info,QString,QString)));
    connect(transaction, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            SIGNAL(errorCode(PackageKit::Transaction::Error,QString)));

    // Required actions
    connect(transaction, SIGNAL(allowCancelChanged()),
            SIGNAL(allowCancelChanged()));
    connect(transaction, SIGNAL(downloadSizeRemainingChanged()),
            SIGNAL(downloadSizeRemainingChanged()));
    connect(transaction, SIGNAL(elapsedTimeChanged()),
            SIGNAL(elapsedTimeChanged()));
    connect(transaction, SIGNAL(isCallerActiveChanged()),
            SIGNAL(isCallerActiveChanged()));
    connect(transaction, SIGNAL(lastPackageChanged()),
            SIGNAL(lastPackageChanged()));
    connect(transaction, SIGNAL(percentageChanged()),
            SIGNAL(percentageChanged()));
    connect(transaction, SIGNAL(remainingTimeChanged()),
            SIGNAL(remainingTimeChanged()));
    connect(transaction, SIGNAL(roleChanged()),
            SIGNAL(roleChanged()));
    connect(transaction, SIGNAL(speedChanged()),
            SIGNAL(speedChanged()));
    connect(transaction, SIGNAL(statusChanged()),
            SIGNAL(statusChanged()));
    connect(transaction, SIGNAL(transactionFlagsChanged()),
            SIGNAL(transactionFlagsChanged()));
    connect(transaction, SIGNAL(uidChanged()),
            SIGNAL(uidChanged()));

    connect(transaction, SIGNAL(downloadSizeRemainingChanged()),
            SLOT(slotChanged()));
    connect(transaction, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            SLOT(slotErrorCode(PackageKit::Transaction::Error,QString)));
    connect(transaction, SIGNAL(eulaRequired(QString,QString,QString,QString)),
            SLOT(slotEulaRequired(QString,QString,QString,QString)));
    connect(transaction, SIGNAL(mediaChangeRequired(PackageKit::Transaction::MediaType,QString,QString)),
            SLOT(slotMediaChangeRequired(PackageKit::Transaction::MediaType,QString,QString)));
    connect(transaction, SIGNAL(repoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,PackageKit::Transaction::SigType)),
            SLOT(slotRepoSignature(QString,QString,QString,QString,QString,QString,QString,PackageKit::Transaction::SigType)));

    connect(transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            SLOT(slotFinished(PackageKit::Transaction::Exit)));

    if (d->flags & Transaction::TransactionFlagSimulate) {
        d->simulateModel = new PackageModel(this);
        connect(d->transaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                d->simulateModel, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
    }

#ifdef HAVE_DEBCONFKDE
    QString _tid = transaction->tid().path();
    QString socket;
    // Build a socket path like /tmp/1761_edeceabd_data_debconf
    socket = QLatin1String("/tmp") % _tid % QLatin1String("_debconf");
    QDBusMessage message;
    message = QDBusMessage::createMethodCall(QLatin1String("org.kde.apperd"),
                                             QLatin1String("/"),
                                             QLatin1String("org.kde.apperd"),
                                             QLatin1String("SetupDebconfDialog"));
    // Use our own cached tid to avoid crashes
    message << qVariantFromValue(_tid);
    message << qVariantFromValue(socket);
    if (d->parentWindow) {
        message << qVariantFromValue(static_cast<uint>(d->parentWindow->effectiveWinId()));
    } else {
        message << qVariantFromValue(0u);
    }

    if (!QDBusConnection::sessionBus().send(message)) {
        kWarning() << "Failed to put SetupDebconfDialog message in DBus queue";
    }

    transaction->setHints(QLatin1String("frontend-socket=") % socket);
#endif //HAVE_DEBCONFKDE
}

void PkTransaction::showDialog(KDialog *dlg)
{
    PkTransactionWidget *widget = qobject_cast<PkTransactionWidget *>(d->parentWindow);
    if (!widget || widget->isCancelVisible()) {
        dlg->setModal(d->parentWindow);
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
            if (d->parentWindow) {
                KMessageBox::error(d->parentWindow, description, title);
            } else {
                KMessageBox::errorWId(0, description, title);
            }
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
    return PkStrings::action(d->originalRole, d->flags);
}

Transaction::Role PkTransaction::cachedRole() const
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

void PkTransaction::enableJobWatcher(bool enable)
{
    d->jobWatcher = enable;
}

#include "PkTransaction.moc"
