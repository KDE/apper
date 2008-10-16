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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <KLocale>
#include <KMessageBox>

#include <KDebug>

#include "KpkTransaction.h"
#include "KpkStrings.h"
#include "KpkRepoSig.h"
#include "KpkLicenseAgreement.h"

KpkTransaction::KpkTransaction( Transaction *trans, bool modal, QWidget *parent )
 : KDialog(parent), m_trans(trans), m_handlyingGpgOrEula(false)
{
    setupUi( mainWidget() );
    setModal(modal);

    // Set Cancel and custom buoton hide
    setButtons( KDialog::Cancel | KDialog::User1 );
    setButtonText( KDialog::User1, i18n("Hide") );
    setButtonToolTip( KDialog::User1, i18n("Allows you to hide the window but keeps running transaction task") );
    enableButtonCancel(false);

    setTransaction(m_trans);
}

KpkTransaction::~KpkTransaction()
{
}

void KpkTransaction::setTransaction(Transaction *trans)
{
    m_trans = trans;

    setWindowIcon( KpkStrings::actionIcon( m_trans->role().action ) );
    // Sets all the status of the current transaction
    setCaption( KpkStrings::action( m_trans->role().action ) );

    enableButtonCancel( m_trans->allowCancel() );

    currentL->setText( KpkStrings::status( m_trans->status() ) );

    connect( m_trans, SIGNAL( package(PackageKit::Package *) ),
	this, SLOT( currPackage(PackageKit::Package *) ) );
    connect( m_trans, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint) ),
	this, SLOT( finished(PackageKit::Transaction::ExitStatus, uint) ) );
    connect( m_trans, SIGNAL( allowCancelChanged(bool) ),
	this, SLOT( enableButtonCancel(bool) ) );
    connect( m_trans, SIGNAL( errorCode(PackageKit::Client::ErrorType, const QString&) ),
	this, SLOT( errorCode(PackageKit::Client::ErrorType, const QString&) ) );
    connect( m_trans, SIGNAL( progressChanged(PackageKit::Transaction::ProgressInfo) ),
	this, SLOT( progressChanged(PackageKit::Transaction::ProgressInfo) ) );
    connect( m_trans, SIGNAL( statusChanged(PackageKit::Transaction::Status) ),
	this, SLOT( statusChanged(PackageKit::Transaction::Status) ) );
    connect( m_trans, SIGNAL( eulaRequired(PackageKit::Client::EulaInfo) ),
	this, SLOT( eulaRequired(PackageKit::Client::EulaInfo) ) );
    connect( m_trans, SIGNAL( repoSignatureRequired(PackageKit::Client::SignatureInfo) ),
	this, SLOT( repoSignatureRequired(PackageKit::Client::SignatureInfo) ) );
}

void KpkTransaction::progressChanged(PackageKit::Transaction::ProgressInfo info)
{
    if (info.percentage) {
	progressBar->setMaximum(100);
	progressBar->setValue(info.percentage);
    }
    else
	progressBar->setMaximum(0);
}

void KpkTransaction::currPackage(Package *p)
{
    packageL->setText( p->name() + " - " + p->version() + " (" + p->arch() + ")" );
    descriptionL->setText( p->summary() );
}

void KpkTransaction::slotButtonClicked(int button)
{
    switch(button) {
        case KDialog::Cancel :
	    kDebug() << "KDialog::Cancel";
            m_trans->cancel();
            break;
        case KDialog::User1 :
	    kDebug() << "KDialog::User1";
	    emit kTransactionFinished(Success);
	    // If you call Close it will
	    // come back to hunt you with Cancel
	    KDialog::slotButtonClicked(KDialog::Yes);
            break;
	case KDialog::Close :
	    kDebug() << "KDialog::Close";
	    emit kTransactionFinished(Cancelled);
            KDialog::slotButtonClicked(KDialog::Close);
            break;
        default :
            KDialog::slotButtonClicked(button);
    }
}

void KpkTransaction::statusChanged(PackageKit::Transaction::Status status)
{
    currentL->setText( KpkStrings::status(status) );
}

void KpkTransaction::errorCode(PackageKit::Client::ErrorType error, const QString &details)
{
    kDebug() << "errorCode: " << error;
    // check to see if we are already handlying these errors
    if ( error == Client::GpgFailure || error == Client::NoLicenseAgreement )
	if (m_handlyingGpgOrEula)
	    return;

// this will be for files signature as seen in gpk
//     if ( error == Client::BadGpgSignature || error Client::MissingGpgSignature)

    // ignoring these as gpk does
    if ( error == Client::TransactionCancelled || error == Client::ProcessKill )
	return;

    KMessageBox::detailedSorry( this, KpkStrings::errorMessage(error), details, KpkStrings::error(error), KMessageBox::Notify );
}

void KpkTransaction::eulaRequired(PackageKit::Client::EulaInfo info)
{
    kDebug() << "eula by: " << info.vendorName;
    if (m_handlyingGpgOrEula) {
	// if its true means that we alread passed here
	m_handlyingGpgOrEula = false;
	return;
    }
    else
	m_handlyingGpgOrEula = true;

    KpkLicenseAgreement *frm = new KpkLicenseAgreement(info, true, this);
    if (frm->exec() == KDialog::Yes && Client::instance()->acceptEula(info) )
	m_handlyingGpgOrEula = false;
    // Well try again, if fail will show the erroCode
    emit kTransactionFinished(ReQueue);
}

void KpkTransaction::repoSignatureRequired(PackageKit::Client::SignatureInfo info)
{
    kDebug() << "signature by: " << info.keyId;
    if (m_handlyingGpgOrEula) {
	// if its true means that we alread passed here
	m_handlyingGpgOrEula = false;
	return;
    }
    else
	m_handlyingGpgOrEula = true;

    KpkRepoSig *frm = new KpkRepoSig(info, true, this);
    if (frm->exec() == KDialog::Yes &&
    Client::instance()->installSignature(info.type, info.keyId, info.package) )
	m_handlyingGpgOrEula = false;
    emit kTransactionFinished(ReQueue);
}

void KpkTransaction::finished(PackageKit::Transaction::ExitStatus status, uint /*runtime*/)
{
    switch(status) {
        case Transaction::Success :
	    kDebug() << "finished succes: " << status;
	    emit kTransactionFinished(Success);
	    KDialog::slotButtonClicked(KDialog::Close);
	    break;
	case Transaction::Cancelled :
            kDebug() << "finished cancelled: " << status;
	    emit kTransactionFinished(Cancelled);
	    KDialog::slotButtonClicked(KDialog::Close);
            break;
	case Transaction::Failed :
	    kDebug() << "finished failed: " << status;
	    emit kTransactionFinished(Failed);
	    KDialog::slotButtonClicked(KDialog::Close);
	    break;
	case Transaction::KeyRequired :
	case Transaction::EulaRequired :
	    kDebug() << "finished KeyRequired or EulaRequired: " << status;
	    break;
	default :
            kDebug() << "finished default" << status;
	    KDialog::slotButtonClicked(KDialog::Close);
            break;
    }
}

#include "KpkTransaction.moc"
