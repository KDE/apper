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

#include "PkTransactionWidget.h"
#include "ui_PkTransactionWidget.h"

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
#include <QScrollBar>

#include <Daemon>

#include "Enum.h"
#include "PkStrings.h"
#include "RepoSig.h"
#include "LicenseAgreement.h"
#include "PkIcons.h"
#include "ApplicationLauncher.h"
#include "Requirements.h"
#include "PkTransaction.h"
#include "TransactionDelegate.h"
#include "PkTransactionProgressModel.h"

class PkTransactionWidgetPrivate
{
public:
    ApplicationLauncher *launcher;
    Transaction::Role role;
    KPixmapSequenceOverlayPainter *busySeq;
};

PkTransactionWidget::PkTransactionWidget(QWidget *parent) :
    QWidget(parent),
    m_trans(0),
    m_keepScrollBarAtBottom(true),
    m_handlingActionRequired(false),
    m_showingError(false),
    m_status(Transaction::StatusUnknown),
    ui(new Ui::PkTransactionWidget),
    d(new PkTransactionWidgetPrivate)
{
    ui->setupUi(this);

    // Setup the animation sequence
    d->busySeq = new KPixmapSequenceOverlayPainter(this);
    d->busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->busySeq->setWidget(ui->label);
    ui->label->clear();

    // Connect stuff from the progressView
    QScrollBar *scrollBar = ui->progressView->verticalScrollBar();
    connect(scrollBar, SIGNAL(sliderMoved(int)),
            this, SLOT(followBottom(int)));
    connect(scrollBar, SIGNAL(valueChanged(int)),
            this, SLOT(followBottom(int)));
    connect(scrollBar, SIGNAL(rangeChanged(int,int)),
            this, SLOT(rangeChanged(int,int)));

    ui->progressView->setItemDelegate(new TransactionDelegate(this));
    
    connect(ui->cancelButton, SIGNAL(rejected()), this, SLOT(cancel()));
}

PkTransactionWidget::~PkTransactionWidget()
{
    // DO NOT disconnect the transaction here,
    // it might not exist when this happen
    delete d;
}

void PkTransactionWidget::hideCancelButton()
{
    // On the session installed we hide the
    // cancel button to use the KDialog main one
    ui->cancelButton->hide();
}

void PkTransactionWidget::cancel()
{
    if (m_trans) {
        m_trans->cancel();
    }
}

void PkTransactionWidget::setTransaction(PkTransaction *trans, Transaction::Role role)
{
    Q_ASSERT(trans);

    m_trans = trans;
    d->role = role;

    // This makes sure the Columns will properly resize to contents
        ui->progressView->header()->setStretchLastSection(false);
    if (role == Transaction::RoleRefreshCache) {
        trans->progressModel()->setColumnCount(1);
        ui->progressView->setModel(trans->progressModel());
        ui->progressView->header()->setResizeMode(0, QHeaderView::Stretch);
    } else {
        trans->progressModel()->setColumnCount(3);
        ui->progressView->setModel(trans->progressModel());
        ui->progressView->header()->reset();
        ui->progressView->header()->setResizeMode(0, QHeaderView::ResizeToContents);
        ui->progressView->header()->setResizeMode(1, QHeaderView::ResizeToContents);
        ui->progressView->header()->setResizeMode(2, QHeaderView::Stretch);
    }

    connect(trans, SIGNAL(changed()), this, SLOT(updateUi()));

    // sets ui
    updateUi();

    // DISCONNECT ALL THESE SIGNALS BEFORE SETTING A NEW ONE
//    connect(m_trans, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
//            this, SLOT(transactionFinished(PackageKit::Transaction::Exit)));
    connect(m_trans, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            this, SLOT(errorCode(PackageKit::Transaction::Error,QString)));
    connect(m_trans, SIGNAL(changed()),
            this, SLOT(updateUi()));
    connect(m_trans, SIGNAL(eulaRequired(QString,QString,QString,QString)),
            this, SLOT(eulaRequired(QString,QString,QString,QString)));
    connect(m_trans, SIGNAL(mediaChangeRequired(PackageKit::Transaction::MediaType,QString,QString)),
            this, SLOT(mediaChangeRequired(PackageKit::Transaction::MediaType,QString,QString)));
    connect(m_trans, SIGNAL(repoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,PackageKit::Transaction::SigType)),
            this, SLOT(repoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,PackageKit::Transaction::SigType)));
    // DISCONNECT ALL THESE SIGNALS BEFORE SETTING A NEW ONE
}

void PkTransactionWidget::unsetTransaction()
{
    if (m_trans == 0) {
        return;
    }

    disconnect(m_trans, SIGNAL(ItemProgress(QString,uint)),
               ui->progressView, SLOT(itemProgress(QString,uint)));
//    disconnect(m_trans, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
//               d->simulateModel, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
    disconnect(m_trans, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
               this, SLOT(transactionFinished(PackageKit::Transaction::Exit)));
    disconnect(m_trans, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
               this, SLOT(errorCode(PackageKit::Transaction::Error,QString)));
    disconnect(m_trans, SIGNAL(changed()),
               this, SLOT(updateUi()));
    disconnect(m_trans, SIGNAL(eulaRequired(QString,QString,QString,QString)),
               this, SLOT(eulaRequired(QString,QString,QString,QString)));
    disconnect(m_trans, SIGNAL(mediaChangeRequired(PackageKit::Transaction::MediaType,QString,QString)),
               this, SLOT(mediaChangeRequired(PackageKit::Transaction::MediaType,QString,QString)));
    disconnect(m_trans, SIGNAL(repoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,PackageKit::Transaction::SigType)),
               this, SLOT(repoSignatureRequired(QString,QString,QString,QString,QString,QString,QString,PackageKit::Transaction::SigType)));
}

void PkTransactionWidget::updateUi()
{
    // sets the action icon to be the window icon
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
    } else if (status == Transaction::StatusDownload) {
        uint speed = transaction->speed();
        qulonglong downloadRemaining = transaction->downloadSizeRemaining();
        if (speed != 0 && downloadRemaining != 0) {
            ui->currentL->setText(i18n("Downloading at %1/s, %2 remaining",
                                       KGlobal::locale()->formatByteSize(speed),
                                       KGlobal::locale()->formatByteSize(downloadRemaining)));
        } else if (speed != 0 && downloadRemaining == 0) {
            ui->currentL->setText(i18n("Downloading at %1/s",
                                         KGlobal::locale()->formatByteSize(speed)));
        } else if (speed == 0 && downloadRemaining != 0) {
            ui->currentL->setText(i18n("Downloading, %1 remaining",
                                         KGlobal::locale()->formatByteSize(downloadRemaining)));
        }
    }

    Transaction::Role role = transaction->role();
    if (d->role != role) {
        QString windowTitle;
        KIcon windowIcon;
        if (role == Transaction::RoleUnknown) {
            windowTitle  = PkStrings::status(Transaction::StatusSetup);
            windowIcon = PkIcons::statusIcon(Transaction::StatusSetup);
        } else {
            windowTitle = PkStrings::action(role);
            windowIcon = PkIcons::actionIcon(role);
        }
        d->role = role;
        setWindowIcon(PkIcons::actionIcon(role));
        setWindowTitle(windowTitle);
        emit titleChanged(windowTitle);
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

void PkTransactionWidget::errorCode(Transaction::Error error, const QString &details)
{
//     kDebug() << "errorCode: " << error << details;
//    d->error = error;
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
//            d->flags ^= Transaction::TransactionFlagOnlyTrusted;
//            requeueTransaction();
        } else {
//            setExitStatus(Cancelled);
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
//    setExitStatus(Failed);
}

void PkTransactionWidget::eulaRequired(const QString &eulaID, const QString &packageID, const QString &vendor, const QString &licenseAgreement)
{
    if (m_handlingActionRequired) {
        // if its true means that we alread passed here
        m_handlingActionRequired = false;
        return;
    } else {
        m_handlingActionRequired = true;
    }

    LicenseAgreement *eula = new LicenseAgreement(eulaID, packageID, vendor, licenseAgreement, this);
    connect(eula, SIGNAL(yesClicked()), this, SLOT(acceptEula()));
    connect(eula, SIGNAL(rejected()), this, SLOT(reject()));
    showDialog(eula);
}

void PkTransactionWidget::acceptEula()
{
    LicenseAgreement *eula = qobject_cast<LicenseAgreement*>(sender());

    if (eula) {
        kDebug() << "Accepting EULA" << eula->id();
        Transaction *trans = new Transaction(this);
//        setTransaction(trans, Transaction::RoleAcceptEula);
        trans->acceptEula(eula->id());
        if (trans->error()) {
            showSorry(i18n("Failed to install signature"),
                      PkStrings::daemonError(trans->error()));
        }
    } else {
        kWarning() << "something is broken, slot is bound to LicenseAgreement but signalled from elsewhere.";
    }
}

void PkTransactionWidget::mediaChangeRequired(Transaction::MediaType type, const QString &id, const QString &text)
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
//        requeueTransaction();
    } else {
//        setExitStatus(Cancelled);
    }
}

void PkTransactionWidget::repoSignatureRequired(const QString &packageID, const QString &repoName, const QString &keyUrl, const QString &keyUserid, const QString &keyId, const QString &keyFingerprint, const QString &keyTimestamp, Transaction::SigType type)
{
    if (m_handlingActionRequired) {
        // if its true means that we alread passed here
        m_handlingActionRequired = false;
        return;
    } else {
        m_handlingActionRequired = true;
    }

    RepoSig *repoSig = new RepoSig(packageID, repoName, keyUrl, keyUserid, keyId, keyFingerprint, keyTimestamp, type, this);
    connect(repoSig, SIGNAL(yesClicked()), this, SLOT(installSignature()));
    connect(repoSig, SIGNAL(rejected()), this, SLOT(reject()));
    showDialog(repoSig);
}

void PkTransactionWidget::installSignature()
{
    RepoSig *repoSig = qobject_cast<RepoSig*>(sender());

    if (repoSig)  {
        kDebug() << "Installing Signature" << repoSig->keyID();
        Transaction *trans = new Transaction(this);
//        setTransaction(trans, Transaction::RoleInstallSignature);
        trans->installSignature(repoSig->sigType(), repoSig->keyID(), repoSig->packageID());
        if (trans->error()) {
            showSorry(i18n("Failed to install signature"),
                      PkStrings::daemonError(trans->error()));
        }
    } else {
        kWarning() << "something is broken, slot is bound to RepoSig but signalled from elsewhere.";
    }
}

bool PkTransactionWidget::isFinished() const
{
//    return d->finished;
    return false;
}

bool PkTransactionWidget::isCancelVisible() const
{
    return ui->cancelButton->isVisible();
}

void PkTransactionWidget::reject()
{
//    d->finished = true;
    //    setExitStatus(Cancelled);
}

void PkTransactionWidget::followBottom(int value)
{
    // If the user moves the slider to the bottom
    // keep it there as the list expands
    QScrollBar *scrollBar = qobject_cast<QScrollBar*>(sender());
    m_keepScrollBarAtBottom = value == scrollBar->maximum();
}

void PkTransactionWidget::rangeChanged(int min, int max)
{
    Q_UNUSED(min)
    QScrollBar *scrollBar = qobject_cast<QScrollBar*>(sender());
    if (m_keepScrollBarAtBottom && scrollBar->value() != max) {
        scrollBar->setValue(max);
    }
}

void PkTransactionWidget::showDialog(KDialog *dlg)
{
    if (ui->cancelButton->isVisible()) {
        dlg->setModal(true);
        dlg->show();
    } else {
        dlg->setProperty("embedded", true);
        emit dialog(dlg);
    }
}

void PkTransactionWidget::showError(const QString &title, const QString &description, const QString &details)
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

void PkTransactionWidget::showSorry(const QString &title, const QString &description, const QString &details)
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

QString PkTransactionWidget::title() const
{
    return PkStrings::action(d->role);
}

Transaction::Role PkTransactionWidget::role() const
{
    return d->role;
}

#include "PkTransactionWidget.moc"
