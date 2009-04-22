/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
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
    bool showDetails;
    bool finished;
};

KpkTransaction::KpkTransaction(Transaction *trans, Behaviors flags, QWidget *parent)
 : KDialog(parent),
   m_trans(trans),
   m_handlingActionRequired(false),
   m_showingError(false),
   m_flags(flags),
   d(new KpkTransactionPrivate)
{
    d->ui.setupUi(mainWidget());
    d->finished = true; // for sanity we are finished till some transaction is set

    // Set Cancel and custom button hide
    setButtons(KDialog::Cancel | KDialog::User1 | KDialog::Details);
    setButtonText(KDialog::User1, i18n("Hide"));
    setButtonToolTip(KDialog::User1, i18n("Allows you to hide the window whilst keeping the transaction task running."));
    setEscapeButton(KDialog::User1);
    enableButtonCancel(false);
    setDetailsWidget(d->ui.detailGroup);
    setDetailsWidgetVisible(false);
    KConfig config("KPackageKit");
    KConfigGroup transactionGroup(&config, "Transaction");
    d->showDetails = transactionGroup.readEntry("ShowDetails", false);

    enableButton(KDialog::Details, false);

    if (m_flags & Modal) {
        setWindowModality(Qt::WindowModal);
    }

    // We need to track when the user close the dialog using the [X] button
    connect(this, SIGNAL(finished()), SLOT(finishedDialog()));

    // after ALL set, lets set the transaction
    setTransaction(m_trans);

//     QLabel label;
 QMovie *movie = new QMovie("/home/daniel/code/packagekit/kde/kpackagekit/KPackageKit/Animations/hi48-action-refresh-cache.mng");
// movie->setScaledSize(QSize(48,48));
// connect(movie, SIGNAL(finished()), movie, SLOT(start()));
// movie->setPaused(false);
 d->ui.label->setMovie(movie);

 movie->start();
 kDebug() << movie->loopCount() << "movie->loopCount()";
}

KpkTransaction::~KpkTransaction()
{
    if (isButtonEnabled(KDialog::Details)) {
        KConfig config("KPackageKit");
        KConfigGroup transactionGroup(&config, "Transaction");
        transactionGroup.writeEntry("ShowDetails", isDetailsWidgetVisible());
    }

    // DO NOT disconnect the transaction here,
    // it might not exist when this happen
    delete d;
}

void KpkTransaction::setTransaction(Transaction *trans)
{
    m_trans = trans;
    d->finished = false;

    // sets the action icon to be the window icon
    setWindowIcon(KpkIcons::actionIcon(m_trans->role().action));
    // Sets the kind of transaction
    setCaption(KpkStrings::action(m_trans->role().action));
    // check to see if we can cancel
    enableButtonCancel(m_trans->allowCancel());
    // clears the package label
    d->ui.packageL->clear();
    d->ui.descriptionL->clear();
    // Now sets the last package
    currPackage(m_trans->lastPackage());
    // sets the current progress
    progressChanged(m_trans->progress());
    // sets the current status
    if (m_trans->status() == Transaction::UnknownStatus) {
       statusChanged(Transaction::StatusSetup);
    } else {
       statusChanged(m_trans->status());
    }

    if (m_trans->role().action == Client::ActionRefreshCache ||
        m_trans->role().action == Client::ActionWhatProvides) {
        d->ui.packageL->hide();
        d->ui.descriptionL->hide();
    } else {
        d->ui.packageL->show();
        d->ui.descriptionL->show();
    }

    connect(m_trans, SIGNAL(package(PackageKit::Package *)),
            this, SLOT(currPackage(PackageKit::Package *)));
    connect(m_trans, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
            this, SLOT(finished(PackageKit::Transaction::ExitStatus, uint)));
    connect(m_trans, SIGNAL(allowCancelChanged(bool)),
            this, SLOT(enableButtonCancel(bool)));
    connect(m_trans, SIGNAL(errorCode(PackageKit::Client::ErrorType, const QString &)),
            this, SLOT(errorCode(PackageKit::Client::ErrorType, const QString &)));
    connect(m_trans, SIGNAL(progressChanged(PackageKit::Transaction::ProgressInfo)),
            this, SLOT(progressChanged(PackageKit::Transaction::ProgressInfo)));
    connect(m_trans, SIGNAL(statusChanged(PackageKit::Transaction::Status)),
            this, SLOT(statusChanged(PackageKit::Transaction::Status)));
    connect(m_trans, SIGNAL(eulaRequired(PackageKit::Client::EulaInfo)),
            this, SLOT(eulaRequired(PackageKit::Client::EulaInfo)));
    connect(m_trans, SIGNAL(mediaChangeRequired(PackageKit::Transaction::MediaType, const QString &, const QString &)),
            this, SLOT(mediaChangeRequired(PackageKit::Transaction::MediaType, const QString &, const QString &)));
    connect(m_trans, SIGNAL(repoSignatureRequired(PackageKit::Client::SignatureInfo)),
            this, SLOT(repoSignatureRequired(PackageKit::Client::SignatureInfo)));
}

void KpkTransaction::progressChanged(PackageKit::Transaction::ProgressInfo info)
{
    if (info.percentage && info.percentage <= 100) {
        d->ui.progressBar->setMaximum(100);
        d->ui.progressBar->setValue(info.percentage);
    } else if (d->ui.progressBar->maximum() != 0) {
        d->ui.progressBar->setMaximum(0);
        d->ui.progressBar->reset();
    }

    if (info.subpercentage && info.subpercentage <= 100) {
        d->ui.subprogressBar->setMaximum(100);
        d->ui.subprogressBar->setValue(info.subpercentage);
    // Check if we didn't already set the maximum as this
    // causes a weird behavior when we keep reseting
    } else if (d->ui.subprogressBar->maximum() != 0) {
        d->ui.subprogressBar->setMaximum(0);
        d->ui.subprogressBar->reset();
    }

    d->ui.progressBar->setRemaining(info.remaining);
}

void KpkTransaction::currPackage(Package *p)
{
    if (!p->id().isEmpty()) {
        QString packageText(p->name());
        if (!p->version().isEmpty()) {
            packageText += ' ' + p->version();
        }
        d->ui.packageL->setText(packageText);
        d->ui.descriptionL->setText(p->summary());
        enableButton(KDialog::Details, true);
        setDetailsWidgetVisible(d->showDetails);
    } else {
        d->ui.packageL->clear();
        d->ui.descriptionL->clear();
        enableButton(KDialog::Details, false);
    }
}

void KpkTransaction::finishedDialog()
{
    slotButtonClicked(KDialog::User1);
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
            if (!d->finished) {
                // We are going to hide the transaction,
                // which can make the user even close System Settings or KPackageKit
                // so we call the tray icon to keep watching the transaction so if the
                // transaction receives some error we can display them
                QDBusMessage message;
                message = QDBusMessage::createMethodCall("org.kde.KPackageKit.Tray",
                                                        "/",
                                                        "org.kde.KPackageKit.Tray",
                                                        QLatin1String("WatchTransaction"));
                message << qVariantFromValue(m_trans->tid());
                QDBusMessage reply = QDBusConnection::sessionBus().call(message);
                if (reply.type() != QDBusMessage::ReplyMessage) {
                    kWarning() << "Message did not receive a reply";
                }
                // Always disconnect BEFORE emitting finished
                m_trans->disconnect();
            }
            emit kTransactionFinished(Success);
            // If you call Close it will
            // come back to hunt you with Cancel
            done(KDialog::User1);
            break;
        case KDialog::Close :
            kDebug() << "KDialog::Close";
            // Always disconnect BEFORE emitting finished
            m_trans->disconnect();
            emit kTransactionFinished(Cancelled);
            done(KDialog::Close);
            break;
        case KDialog::Details :
            d->showDetails = !d->showDetails;
        default : // Should be only details
            KDialog::slotButtonClicked(button);
    }
}

void KpkTransaction::statusChanged(PackageKit::Transaction::Status status)
{
    d->ui.currentL->setText(KpkStrings::status(status));
}

void KpkTransaction::errorCode(PackageKit::Client::ErrorType error, const QString &details)
{
//     kDebug() << "errorCode: " << error << details;
    // obvious message, don't tell the user
    if (error == Client::ErrorTransactionCancelled) {
        return;
    }

    if (error == Client::ErrorMissingGpgSignature) {
        kDebug() << "Missing GPG!";
        m_handlingActionRequired = true;
        int ret = KMessageBox::warningYesNo(this,
                                            details +
                                            i18n("<br />Installing unsigned packages can compromise your system, "
                                            "as it is impossible to verify if the software came from a trusted "
                                            "source. Are you sure you want to continue installation?"),
                                            i18n("Installing unsigned software"));
        if (ret == KMessageBox::Yes) {
            emit kTransactionFinished(ReQueue);
            kDebug() << "Asking for a re-queue";
        } else {
            emit kTransactionFinished(Cancelled);
            if (m_flags & CloseOnFinish)
                done(QDialog::Rejected);
        }
        return;
    }

    // check to see if we are already handlying these errors
    if (error == Client::ErrorGpgFailure ||
        error == Client::ErrorNoLicenseAgreement ||
        error == Client::ErrorMediaChangeRequired)
    {
        if (m_handlingActionRequired) {
            return;
        }
    }

// this will be for files signature as seen in gpk
//     if ( error == Client::BadGpgSignature || error Client::MissingGpgSignature)

    // ignoring these as gpk does
    if (error == Client::ErrorTransactionCancelled || error == Client::ErrorProcessKill) {
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

    KpkLicenseAgreement *frm = new KpkLicenseAgreement(info, true, this);
    if (frm->exec() == KDialog::Yes && Client::instance()->acceptEula(info)) {
        m_handlingActionRequired = false;
    }

    // Well try again, if fail will show the erroCode
    emit kTransactionFinished(ReQueue);
}

void KpkTransaction::mediaChangeRequired(PackageKit::Transaction::MediaType type, const QString &id, const QString &text)
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

    KpkRepoSig *frm = new KpkRepoSig(info, true, this);
    if (frm->exec() == KDialog::Yes &&
        Client::instance()->installSignature(info.type, info.keyId, info.package)) {
        m_handlingActionRequired = false;
    }
    kDebug() << "Requeue!";
    emit kTransactionFinished(ReQueue);
}

void KpkTransaction::finished(PackageKit::Transaction::ExitStatus status, uint runtime)
{
    Q_UNUSED(runtime)
    d->finished = true;
    switch(status) {
    case Transaction::ExitSuccess :
        d->ui.progressBar->setMaximum(100);
        d->ui.progressBar->setValue(100);
        emit kTransactionFinished(Success);
        break;
    case Transaction::ExitCancelled :
        d->ui.progressBar->setMaximum(100);
        d->ui.progressBar->setValue(100);
        emit kTransactionFinished(Cancelled);
        break;
    case Transaction::ExitFailed :
        kDebug() << "Failed.";
        if (!m_handlingActionRequired) {
            d->ui.progressBar->setMaximum(0);
            d->ui.progressBar->reset();
            kDebug() << "Yep, we failed.";
            emit kTransactionFinished(Failed);
        }
        break;
    case Transaction::ExitKeyRequired :
    case Transaction::ExitEulaRequired :
    case Transaction::ExitMediaChangeRequired :
        kDebug() << "finished KeyRequired or EulaRequired: " << status;
        d->ui.currentL->setText(KpkStrings::status(Transaction::StatusSetup));
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

#include "KpkTransaction.moc"
