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

#include "KpkTransaction.h"

#include <KLocale>
#include <KMessageBox>

#include <KDebug>

#include <QMovie>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>

#include "KpkStrings.h"
#include "KpkRepoSig.h"
#include "KpkLicenseAgreement.h"
#include "KpkIcons.h"

#include "KpkProgressBar.h"

#include "ui_KpkTransaction.h"

class KpkTransactionPrivate
{
public:
    Ui::KpkTransaction ui;

    QString tid;
    bool showDetails;
    bool finished;
    bool allowDeps;
    bool onlyTrusted;
    QList<QSharedPointer<PackageKit::Package> > packages;
};

KpkTransaction::KpkTransaction(Transaction *trans, Behaviors flags, QWidget *parent)
 : KDialog(parent),
   m_trans(trans),
   m_handlingActionRequired(false),
   m_showingError(false),
   m_flags(flags),
   m_exitStatus(Success),
   m_status(Enum::UnknownStatus),
   d(new KpkTransactionPrivate)
{
    d->ui.setupUi(mainWidget());
    d->finished = true; // for sanity we are finished till some transaction is set
    d->onlyTrusted = true; // for sanity we are trusted till an error is given and the user accepts

    // Set Cancel and custom button hide
    setButtons(KDialog::Cancel | KDialog::User1 | KDialog::Details);
    enableButton(KDialog::Details, false);
    setButtonText(KDialog::User1, i18n("Hide"));
    setButtonToolTip(KDialog::User1, i18n("Allows you to hide the window whilst keeping the transaction task running."));
    setEscapeButton(KDialog::User1);
    enableButtonCancel(false);
    setDetailsWidget(d->ui.detailGroup);
    KConfig config("KPackageKit");
    KConfigGroup transactionGroup(&config, "Transaction");
    d->showDetails = transactionGroup.readEntry("ShowDetails", false);
    setDetailsWidgetVisible(d->showDetails);
    d->ui.detailGroup->setVisible(d->showDetails);

    // This MUST come after setDetailsWidgetVisible since
    // it keeps ajusting the size (which sucks btw)
    // This doesn't work, KDialog resizes when the details
    // button is clicked, maybe it should save the size
    // before showing the details then when hiding it it would
    // go back to the first size.
//     setMinimumSize(QSize(400,120));
//     KConfigGroup transactionDialog(&config, "TransactionDialog");
//     restoreDialogSize(transactionDialog);

    if (m_flags & Modal) {
        setWindowModality(Qt::WindowModal);
    }

    // We need to track when the user close the dialog using the [X] button
    connect(this, SIGNAL(finished()), SLOT(finishedDialog()));
    connect(this, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
            SLOT(setExitStatus(KpkTransaction::ExitStatus)));

    // after ALL set, lets set the transaction
    setTransaction(m_trans);
}

KpkTransaction::~KpkTransaction()
{
    KConfig config("KPackageKit");
    if (isButtonEnabled(KDialog::Details)) {
        KConfigGroup transactionGroup(&config, "Transaction");
        transactionGroup.writeEntry("ShowDetails", isDetailsWidgetVisible());
    }
//     KConfigGroup transactionDialog(&config, "TransactionDialog");
//     saveDialogSize(transactionDialog);

    // DO NOT disconnect the transaction here,
    // it might not exist when this happen
    delete d;
}

QString KpkTransaction::tid() const
{
    return d->tid;
}

bool KpkTransaction::allowDeps() const
{
    return d->allowDeps;
}

bool KpkTransaction::onlyTrusted() const
{
    return d->onlyTrusted;
}

QList<QSharedPointer<PackageKit::Package> > KpkTransaction::packages() const
{
    return d->packages;
}

void KpkTransaction::setAllowDeps(bool allowDeps)
{
    d->allowDeps = allowDeps;
}

void KpkTransaction::setPackages(QList<QSharedPointer<PackageKit::Package> > packages)
{
    d->packages = packages;
}

void KpkTransaction::setTransaction(Transaction *trans)
{
    m_trans = trans;
    d->tid = trans->tid();
    d->finished = false;

    // sets the action icon to be the window icon
    setWindowIcon(KpkIcons::actionIcon(m_trans->role()));
    // Sets the kind of transaction
    setCaption(KpkStrings::action(m_trans->role()));
    // check to see if we can cancel
    enableButtonCancel(m_trans->allowCancel());
    // clears the package label
    d->ui.packageL->clear();
    d->ui.descriptionL->setText(QString());
    // Now sets the last package
    currPackage(m_trans->lastPackage());
    // sets the current progress
    updateUi();
    // sets the current status
    // TODO
//     if (m_trans->status() == Transaction::UnknownStatus) {
//        statusChanged(Transaction::StatusSetup);
//     } else {
//        statusChanged(m_trans->status());
//     }

    if (m_trans->role() == Enum::RoleRefreshCache ||
        m_trans->role() == Enum::RoleWhatProvides) {
        d->ui.packageL->hide();
        d->ui.descriptionL->hide();
    } else {
        d->ui.packageL->show();
        d->ui.descriptionL->show();
    }

    // DISCONNECT ALL THESE SIGNALS BEFORE CLOSING
    connect(m_trans, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
            this, SLOT(currPackage(QSharedPointer<PackageKit::Package>)));
    connect(m_trans, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            this, SLOT(finished(PackageKit::Enum::Exit, uint)));
    connect(m_trans, SIGNAL(errorCode(PackageKit::Enum::Error, const QString &)),
            this, SLOT(errorCode(PackageKit::Enum::Error, const QString &)));
    connect(m_trans, SIGNAL(changed()),
            this, SLOT(updateUi()));
    connect(m_trans, SIGNAL(eulaRequired(PackageKit::Client::EulaInfo)),
            this, SLOT(eulaRequired(PackageKit::Client::EulaInfo)));
    connect(m_trans, SIGNAL(mediaChangeRequired(PackageKit::Enum::MediaType, const QString &, const QString &)),
            this, SLOT(mediaChangeRequired(PackageKit::Enum::MediaType, const QString &, const QString &)));
    connect(m_trans, SIGNAL(repoSignatureRequired(PackageKit::Client::SignatureInfo)),
            this, SLOT(repoSignatureRequired(PackageKit::Client::SignatureInfo)));
    // DISCONNECT ALL THESE SIGNALS BEFORE CLOSING
}

void KpkTransaction::unsetTransaction()
{
    disconnect(m_trans, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
               this, SLOT(currPackage(QSharedPointer<PackageKit::Package>)));
    disconnect(m_trans, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
               this, SLOT(finished(PackageKit::Enum::Exit, uint)));
    disconnect(m_trans, SIGNAL(errorCode(PackageKit::Enum::Error, const QString &)),
               this, SLOT(errorCode(PackageKit::Enum::Error, const QString &)));
    disconnect(m_trans, SIGNAL(changed()),
               this, SLOT(updateUi()));
    disconnect(m_trans, SIGNAL(eulaRequired(PackageKit::Client::EulaInfo)),
               this, SLOT(eulaRequired(PackageKit::Client::EulaInfo)));
    disconnect(m_trans, SIGNAL(mediaChangeRequired(PackageKit::Enum::MediaType,
                                                   const QString &, const QString &)),
               this, SLOT(mediaChangeRequired(PackageKit::Enum::MediaType, const QString &, const QString &)));
    disconnect(m_trans, SIGNAL(repoSignatureRequired(PackageKit::Client::SignatureInfo)),
               this, SLOT(repoSignatureRequired(PackageKit::Client::SignatureInfo)));
}

void KpkTransaction::updateUi()
{
    uint percentage = m_trans->percentage();
    uint subpercentage = m_trans->subpercentage();
    if (percentage <= 100) {
        d->ui.progressBar->setMaximum(100);
        d->ui.progressBar->setValue(percentage);
    } else if (d->ui.progressBar->maximum() != 0) {
        d->ui.progressBar->setMaximum(0);
        d->ui.progressBar->reset();
    }

    if (subpercentage <= 100) {
        d->ui.subprogressBar->setMaximum(100);
        d->ui.subprogressBar->setValue(subpercentage);
    // Check if we didn't already set the maximum as this
    // causes a weird behavior when we keep reseting
    } else if (d->ui.subprogressBar->maximum() != 0) {
        d->ui.subprogressBar->setMaximum(0);
        d->ui.subprogressBar->reset();
    }

    d->ui.progressBar->setRemaining(m_trans->remainingTime());

    // Status
    Enum::Status status = m_trans->status();
    if (m_status != status) {
        m_status = status;
        d->ui.currentL->setText(KpkStrings::status(status));

        QMovie *movie;
        // Grab the right icon name
        QString icon(KpkIcons::statusAnimation(status));
        movie = KIconLoader::global()->loadMovie(icon,
                                                KIconLoader::NoGroup,
                                                48,
                                                this);
        if (movie) {
            // If the movie is set we KIconLoader it,
            // set it and start
            d->ui.label->setMovie(movie);
            movie->start();
        } else {
            // Else it's probably a static icon so try to load
            d->ui.label->setPixmap(KpkIcons::getIcon(icon).pixmap(48,48));
        }
    }

    // Allow cancel
    enableButtonCancel(m_trans->allowCancel());
}

void KpkTransaction::currPackage(QSharedPointer<PackageKit::Package>p)
{
    if (!p->id().isEmpty()) {
        QString packageText(p->name());
        if (!p->version().isEmpty()) {
            packageText += ' ' + p->version();
        }
        d->ui.packageL->setText(packageText);
        d->ui.descriptionL->setText(p->summary());
        enableButton(KDialog::Details, true);
    } else {
        d->ui.packageL->clear();
        d->ui.descriptionL->setText(QString());
        enableButton(KDialog::Details, false);
        setDetailsWidgetVisible(false);
    }
}

void KpkTransaction::finishedDialog()
{
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
        // Always disconnect BEFORE emitting finished
        unsetTransaction();

        emit kTransactionFinished(Success);
    }
}

void KpkTransaction::slotButtonClicked(int button)
{
    switch(button) {
    case KDialog::Cancel :
        kDebug() << "KDialog::Cancel";
        m_trans->cancel();
        m_flags |= CloseOnFinish;
        break;
    case KDialog::User1 :
        kDebug() << "KDialog::User1";
        // when we're done finishedDialog() is called
        done(KDialog::User1);
        break;
    case KDialog::Close :
        kDebug() << "KDialog::Close";
        // Always disconnect BEFORE emitting finished
        unsetTransaction();
        emit kTransactionFinished(Cancelled);
        done(KDialog::Close);
        break;
    case KDialog::Details :
        d->showDetails = !d->showDetails;
//         d->ui.detailGroup->setVisible(d->showDetails);
//         break;
    default : // Should be only details
        KDialog::slotButtonClicked(button);
    }
}

// Return value: if the error code suggests to try with only_trusted %FALSE

static bool untrustedIsNeed(Enum::Error error)
{
    switch (error) {
    case Enum::ErrorGpgFailure:
    case Enum::ErrorBadGpgSignature:
    case Enum::ErrorMissingGpgSignature:
    case Enum::ErrorCannotInstallRepoUnsigned:
    case Enum::ErrorCannotUpdateRepoUnsigned:
        return true;
    default:
        return false;
    }
}

void KpkTransaction::errorCode(PackageKit::Enum::Error error, const QString &details)
{
//     kDebug() << "errorCode: " << error << details;
    // obvious message, don't tell the user
    if (error == Enum::ErrorTransactionCancelled) {
        return;
    }

    if (untrustedIsNeed(error)) {
        kDebug() << "Missing GPG!";
        m_handlingActionRequired = true;
        int ret = KMessageBox::warningYesNo(this,
                                            i18n("You are about to install unsigned packages can compromise your system, "
                                            "as it is impossible to verify if the software came from a trusted "
                                            "source.\n Are you sure you want to continue installation?"),
                                            i18n("Installing unsigned software"));
        if (ret == KMessageBox::Yes) {
            // Set only trusted to false, to do as the user asked
            d->onlyTrusted = false;
            emit kTransactionFinished(ReQueue);
            kDebug() << "Asking for a re-queue";
        } else {
            emit kTransactionFinished(Cancelled);
            if (m_flags & CloseOnFinish)
                done(QDialog::Rejected);
        }
        m_handlingActionRequired = false;
        return;
    }

    // check to see if we are already handlying these errors
    if (error == Enum::ErrorNoLicenseAgreement ||
        error == Enum::ErrorMediaChangeRequired)
    {
        if (m_handlingActionRequired) {
            return;
        }
    }

// this will be for files signature as seen in gpk
//     if ( error == Client::BadGpgSignature || error Client::MissingGpgSignature)

    // ignoring these as gpk does
    if (error == Enum::ErrorTransactionCancelled || error == Enum::ErrorProcessKill) {
        return;
    }

    m_showingError = true;
    KMessageBox::detailedSorry(this,
                               KpkStrings::errorMessage(error),
                               QString(details).replace('\n', "<br />"),
                               KpkStrings::error(error),
                               KMessageBox::Notify);
    m_showingError = false;

    // when we receive an error we are done
    if (m_flags & CloseOnFinish) {
        done(QDialog::Rejected);
    }
}

void KpkTransaction::eulaRequired(PackageKit::Client::EulaInfo info)
{
    kDebug() << "eula by: " << info.vendorName;

    if (m_handlingActionRequired) {
        // if its true means that we alread passed here
        m_handlingActionRequired = false;
        return;
    } else {
        m_handlingActionRequired = true;
    }

    QPointer<KpkLicenseAgreement> frm = new KpkLicenseAgreement(info, true, this);
    if (frm->exec() == KDialog::Yes && Client::instance()->acceptEula(info)) {
        m_handlingActionRequired = false;
    }
    delete frm;

    // Well try again, if fail will show the erroCode
    emit kTransactionFinished(ReQueue);
}

void KpkTransaction::mediaChangeRequired(PackageKit::Enum::MediaType type, const QString &id, const QString &text)
{
    Q_UNUSED(id)
    kDebug() << "mediaChangeRequired: " << id << text;

    m_handlingActionRequired = true;
    int ret = KMessageBox::questionYesNo(this,
                                         KpkStrings::mediaMessage(type, text),
                                         i18n("A media change is required"),
                                         KStandardGuiItem::cont(),
                                         KStandardGuiItem::cancel());
    m_handlingActionRequired = false;

    // if the user clicked continue we got yes
    if (ret == KMessageBox::Yes) {
        emit kTransactionFinished(ReQueue);
    } else {
        // when we receive an error we are done
        if (m_flags & CloseOnFinish) {
            done(QDialog::Rejected);
        }
    }
}

void KpkTransaction::repoSignatureRequired(PackageKit::Client::SignatureInfo info)
{
    kDebug() << "signature by: " << info.keyId;
    if (m_handlingActionRequired) {
        // if its true means that we alread passed here
        m_handlingActionRequired = false;
        return;
    } else {
        m_handlingActionRequired = true;
    }

    QPointer<KpkRepoSig> frm = new KpkRepoSig(info, true, this);
    if (frm->exec() == KDialog::Yes &&
        Client::instance()->installSignature(info.type, info.keyId, info.package)) {
        m_handlingActionRequired = false;
    }
    delete frm;

//     kDebug() << "Requeue!";
    emit kTransactionFinished(ReQueue);
}

void KpkTransaction::finished(PackageKit::Enum::Exit status, uint runtime)
{
    Q_UNUSED(runtime)
    d->finished = true;
    switch(status) {
    case Enum::ExitSuccess :
        d->ui.progressBar->setMaximum(100);
        d->ui.progressBar->setValue(100);
        emit kTransactionFinished(Success);
        break;
    case Enum::ExitCancelled :
        d->ui.progressBar->setMaximum(100);
        d->ui.progressBar->setValue(100);
        emit kTransactionFinished(Cancelled);
        break;
    case Enum::ExitFailed :
        kDebug() << "Failed.";
        if (!m_handlingActionRequired) {
            d->ui.progressBar->setMaximum(0);
            d->ui.progressBar->reset();
            kDebug() << "Yep, we failed.";
            emit kTransactionFinished(Failed);
        }
        break;
    case Enum::ExitKeyRequired :
    case Enum::ExitEulaRequired :
    case Enum::ExitMediaChangeRequired :
    case Enum::ExitNeedUntrusted :
        kDebug() << "finished KeyRequired or EulaRequired: " << status;
        d->ui.currentL->setText(KpkStrings::status(Enum::StatusSetup));
        if (!m_handlingActionRequired) {
            emit kTransactionFinished(Failed);
        }
        break;
    default :
        d->ui.progressBar->setMaximum(100);
        d->ui.progressBar->setValue(100);
        kDebug() << "finished default" << status;
        KDialog::slotButtonClicked(KDialog::Close);
        break;
    }
    // if we're not showing an error or don't have the
    // CloseOnFinish flag don't close the dialog
    if (m_flags & CloseOnFinish && !m_handlingActionRequired && !m_showingError) {
        done(QDialog::Rejected);
        deleteLater();
    }
}

KpkTransaction::ExitStatus KpkTransaction::exitStatus() const
{
    return m_exitStatus;
}

void KpkTransaction::setExitStatus(KpkTransaction::ExitStatus status)
{
    m_exitStatus = status;
}

#include "KpkTransaction.moc"
