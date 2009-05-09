/***************************************************************************
 *   Copyright (C) 2008-2009 by Daniel Nicoletti                           *
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

#include "KpkReviewChanges.h"

#include <KMessageBox>
#include <KDebug>

#include <KpkStrings.h>
#include "KpkRequirements.h"
#include "ui_KpkReviewChanges.h"

#define UNIVERSAL_PADDING 6

class KpkReviewChangesPrivate
{
public:
    Ui::KpkReviewChanges ui;
};

KpkReviewChanges::KpkReviewChanges(const QList<Package*> &packages, QWidget *parent)
 : KDialog(parent), d(new KpkReviewChangesPrivate)
{
    d->ui.setupUi(mainWidget());

    //initialize the model, delegate, client and  connect it's signals
    d->ui.packageView->setItemDelegate(m_pkgDelegate = new KpkDelegate(d->ui.packageView));
    d->ui.packageView->setModel(m_pkgModelMain = new KpkPackageModel(packages, this, d->ui.packageView));
    m_pkgModelMain->checkAll();
    d->ui.packageView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(m_pkgModelMain, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)),
            this, SLOT(checkChanged()));

    // Set Apply and Cancel buttons
    setButtons(KDialog::Apply | KDialog::Cancel);

    // Count how many items will be installed and removed to set
    // better apply text and description text
    int countRemove  = 0;
    int countInstall = 0;
    foreach (Package *package, packages) {
        // If the package is installed we are going to remove it
        if (package->state() == Package::StateInstalled) {
            countRemove++;
        } else {
            countInstall++;
        }
    }

    if (packages.size() == countInstall) {
        setText(i18np("The following package will be installed:",
                      "The following packages will be installed:", countInstall));
        setButtonText(KDialog::Apply, i18n("Install Now"));
    } else if (packages.size() == countRemove) {
        setText(i18np("The following package will be removed:",
                      "The following packages will be removed:", countRemove));
        setButtonText(KDialog::Apply, i18n("Remove Now"));
    } else {
        setText(i18np("The following package will be removed and installed:",
                      "The following packages will be removed and installed:", packages.size()));
        setButtonText(KDialog::Apply, i18n("Apply Now"));
    }

    // restore size
    setMinimumSize(QSize(320,280));
    KConfig config("KPackageKit");
    KConfigGroup reviewChangesDialog(&config, "ReviewChangesDialog");
    restoreDialogSize(reviewChangesDialog);
}

KpkReviewChanges::~KpkReviewChanges()
{
    // save size
    KConfig config("KPackageKit");
    KConfigGroup reviewChangesDialog(&config, "ReviewChangesDialog");
    saveDialogSize(reviewChangesDialog);
}

void KpkReviewChanges::setTitle(const QString &title)
{
    setCaption(title);
}

void KpkReviewChanges::setText(const QString &text)
{
    d->ui.label->setText(text);
}

void KpkReviewChanges::doAction()
{
    m_client = Client::instance();
    m_actions = m_client->getActions();
    // check what packages are installed and marked to be removed
    for (int i = 0; i < m_pkgModelMain->selectedPackages().size(); ++i) {
        if (m_pkgModelMain->selectedPackages().at(i)->state() == Package::StateInstalled) {
            m_remPackages << m_pkgModelMain->selectedPackages().takeAt(i);
        }
    }

    // check what packages are available and marked to be installed
    for (int i = 0; i < m_pkgModelMain->selectedPackages().size(); ++i) {
        if ( m_pkgModelMain->selectedPackages().at(i)->state() == Package::StateAvailable )
            m_addPackages << m_pkgModelMain->selectedPackages().takeAt(i);
    }

    checkTask();
}

void KpkReviewChanges::checkTask()
{
    if (!m_remPackages.isEmpty()) {
        kDebug() << "task rm if";
        if (m_actions & Client::ActionRemovePackages) {
            if (m_actions & Client::ActionGetRequires) {
                m_reqDepPackages = m_remPackages;
                // Create the requirements transaction and it's model
                m_pkgModelReq = new KpkSimplePackageModel(this);
                m_transactionReq = m_client->getRequires(m_reqDepPackages, Client::FilterInstalled, true);
                connect(m_transactionReq, SIGNAL(package(PackageKit::Package *)),
                        m_pkgModelReq, SLOT(addPackage(PackageKit::Package *)));
                connect(m_transactionReq, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                        this, SLOT(reqFinished(PackageKit::Transaction::ExitStatus, uint)));
                connect(m_transactionReq, SIGNAL(errorCode(PackageKit::Client::ErrorType, const QString &)),
                        this, SLOT(errorCode(PackageKit::Client::ErrorType, const QString &)));
                // Create a Transaction dialog to don't upset the user
                KpkTransaction* reqFinder = new KpkTransaction(m_transactionReq, KpkTransaction::CloseOnFinish | KpkTransaction::Modal, this);
                reqFinder->exec();
            } else {
                removePackages();
            }
        } else {
            KMessageBox::error(this, i18n("The current backend does not support removing packages."), i18n("KPackageKit Error"));
        }
    } else if (!m_addPackages.isEmpty()) {
        kDebug() << "task add else";
        if (m_actions & Client::ActionInstallPackages) {
            if (m_actions & Client::ActionGetDepends) {
                m_reqDepPackages = m_addPackages;
                // Create the depends transaction and it's model
                m_pkgModelDep = new KpkSimplePackageModel(this);
                m_transactionDep = m_client->getDepends(m_reqDepPackages, Client::FilterNotInstalled, true);
                connect(m_transactionDep, SIGNAL(package(PackageKit::Package *)),
                        m_pkgModelDep, SLOT(addPackage(PackageKit::Package *)));
                connect(m_transactionDep, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint)),
                        this, SLOT(depFinished(PackageKit::Transaction::ExitStatus, uint)));
                connect(m_transactionDep, SIGNAL(errorCode(PackageKit::Client::ErrorType, const QString &)),
                        this, SLOT(errorCode(PackageKit::Client::ErrorType, const QString &)));
                // Create a Transaction dialog to don't upset the user
                KpkTransaction* reqFinder = new KpkTransaction(m_transactionDep, KpkTransaction::Modal | KpkTransaction::CloseOnFinish, this);
                reqFinder->exec();
            } else {
                installPackages();
            }
        } else {
            KMessageBox::error(this, i18n("Current backend does not support installing packages."), i18n("KPackageKit Error"));
        }
    } else {
        kDebug() << "task else";
        KDialog::slotButtonClicked(KDialog::Ok);
    }
}

void KpkReviewChanges::reqFinished(PackageKit::Transaction::ExitStatus status, uint /*runtime*/)
{
    kDebug() << "reqFinished";
    if (status == Transaction::ExitSuccess) {
        if (m_pkgModelReq->rowCount( QModelIndex() ) > 0) {
            KpkRequirements *requimentD = new KpkRequirements(i18n("The following packages will also be removed for dependencies"), m_pkgModelReq, this);
            connect(requimentD, SIGNAL(okClicked()), this, SLOT(removePackages()));
            connect(requimentD, SIGNAL(cancelClicked()), this, SLOT(close()));
            requimentD->show();
        } else {
            removePackages();
        }
    } else {
        // TODO inform the user
        kDebug() << "getReq Failed: " << status;
        m_reqDepPackages.clear();
        m_remPackages.clear();
        checkTask();
    }
    kDebug() << "reqFinished2";
}

void KpkReviewChanges::removePackages()
{
    kDebug() << "removePackages";
    if (Transaction *t = m_client->removePackages(m_remPackages)) {
        KpkTransaction *frm = new KpkTransaction(t, KpkTransaction::CloseOnFinish | KpkTransaction::Modal, this);
        connect(frm, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
                this, SLOT(remFinished(KpkTransaction::ExitStatus)));
        frm->show();
    } else {
        KMessageBox::sorry(this,
                           i18n("You do not have the necessary privileges to perform this action."),
                           i18n("Failed to remove package"));
    }
    kDebug() << "finished remove";
}

void KpkReviewChanges::depFinished(PackageKit::Transaction::ExitStatus status, uint /*runtime*/)
{
    kDebug() << "depFinished";
    if (status == Transaction::ExitSuccess) {
        if (m_pkgModelDep->rowCount(QModelIndex()) > 0) {
            KpkRequirements *requimentD = new KpkRequirements(i18n("The following packages will also be installed as dependencies"), m_pkgModelDep, this);
            connect(requimentD, SIGNAL(okClicked()), this, SLOT(installPackages()));
            connect(requimentD, SIGNAL(cancelClicked()), this, SLOT(close()));
            requimentD->show();
        } else {
            installPackages();
        }
    } else {
        kDebug() << "getDep Failed: " << status;
        m_reqDepPackages.clear();
        m_addPackages.clear();
        checkTask();
    }
    kDebug() << "depFinished2";
}

void KpkReviewChanges::installPackages()
{
    kDebug() << "installPackages";
    if ( Transaction *t = m_client->installPackages(m_addPackages) ) {
        KpkTransaction *frm = new KpkTransaction(t, KpkTransaction::CloseOnFinish | KpkTransaction::Modal, this);
        connect(frm, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
                this, SLOT(addFinished(KpkTransaction::ExitStatus)));
        frm->show();
    } else {
        KMessageBox::sorry(this,
                             i18n("You do not have the necessary privileges to perform this action."),
                             i18n("Failed to install package"));
    }
    kDebug() << "finished install";
}

void KpkReviewChanges::remFinished(KpkTransaction::ExitStatus status)
{
    switch (status) {
        case KpkTransaction::Success :
            m_remPackages.clear();
            checkTask();
            break;
        case KpkTransaction::Failed :
            //TODO This is not nice we should close instead
            setButtons(KDialog::Close);
            break;
        case KpkTransaction::Cancelled :
            KDialog::slotButtonClicked(KDialog::Close);
            break;
        case KpkTransaction::ReQueue :
            KpkTransaction *trans = (KpkTransaction *) sender();
            trans->setTransaction(m_client->removePackages(m_remPackages));
            break;
    }
}

void KpkReviewChanges::addFinished(KpkTransaction::ExitStatus status)
{
    switch (status) {
        case KpkTransaction::Success :
            m_addPackages.clear();
            checkTask();
            break;
        case KpkTransaction::Failed :
            setButtons(KDialog::Close);
            break;
        case KpkTransaction::Cancelled :
            KDialog::slotButtonClicked(KDialog::Close);
            break;
        case KpkTransaction::ReQueue :
            KpkTransaction *trans = (KpkTransaction *) sender();
            trans->setTransaction(m_client->installPackages(m_addPackages));
            break;
    }
}

void KpkReviewChanges::slotButtonClicked(int button)
{
    switch(button) {
        case KDialog::Cancel :
            close();
            break;
        case KDialog::Apply :
            doAction();
            break;
        default :
            KDialog::slotButtonClicked(button);
    }
}

void KpkReviewChanges::errorCode(PackageKit::Client::ErrorType error, const QString &details)
{
    KMessageBox::detailedSorry(this, KpkStrings::errorMessage(error),
            details, KpkStrings::error(error), KMessageBox::Notify);
}

void KpkReviewChanges::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateColumnsWidth();
}

bool KpkReviewChanges::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::PolishRequest:
        case QEvent::Polish:
            updateColumnsWidth(true);
            break;
        default:
            break;
    }

    return QWidget::event(event);
}

void KpkReviewChanges::updateColumnsWidth(bool force)
{
    m_viewWidth = d->ui.packageView->viewport()->width();

    if (force) {
        m_viewWidth -= style()->pixelMetric(QStyle::PM_ScrollBarExtent) + UNIVERSAL_PADDING;
    }

    d->ui.packageView->setColumnWidth(0, m_pkgDelegate->columnWidth(0, m_viewWidth));
    d->ui.packageView->setColumnWidth(1, m_pkgDelegate->columnWidth(1, m_viewWidth));
}

void KpkReviewChanges::checkChanged()
{
    if (m_pkgModelMain->selectedPackages().size() > 0) {
        enableButtonApply(true);
    } else {
        enableButtonApply(false);
    }
}

#include "KpkReviewChanges.moc"
