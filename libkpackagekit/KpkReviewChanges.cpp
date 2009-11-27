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

#include <KpkMacros.h>
#include <KMessageBox>

#include <KDebug>

#include "KpkStrings.h"
#include "KpkRequirements.h"
#include "ui_KpkReviewChanges.h"

#define UNIVERSAL_PADDING 6
#define AUTOREMOVE true

class KpkReviewChangesPrivate
{
public:
    Ui::KpkReviewChanges ui;
};

KpkReviewChanges::KpkReviewChanges(const QList<Package*> &packages, QWidget *parent)
 : KDialog(parent),
   d(new KpkReviewChangesPrivate),
   m_flags(Default)
{
    d->ui.setupUi(mainWidget());

    m_client = Client::instance();

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

int KpkReviewChanges::exec(OperationModes flags)
{
    m_flags = flags;
    if (m_flags & ShowConfirmation) {
        this->show();
    } else {
        // Starts the action without showing the dialog
        QTimer::singleShot(0, this, SLOT(doAction()));
    }

    QEventLoop loop;
    connect(this, SIGNAL(finished(int)), &loop, SLOT(quit()));
    loop.exec();

    kDebug() << result();
    return result();
}

void KpkReviewChanges::doAction()
{
    m_actions = m_client->getActions();
    // check what packages are installed and marked to be removed
    for (int i = 0; i < m_pkgModelMain->selectedPackages().size(); ++i) {
        if (m_pkgModelMain->selectedPackages().at(i)->state() == Package::StateInstalled) {
            m_remPackages << m_pkgModelMain->selectedPackages().takeAt(i);
        }
    }

    // check what packages are available and marked to be installed
    for (int i = 0; i < m_pkgModelMain->selectedPackages().size(); ++i) {
        if (m_pkgModelMain->selectedPackages().at(i)->state() == Package::StateAvailable) {
            m_addPackages << m_pkgModelMain->selectedPackages().takeAt(i);
        }
    }

    checkTask();
}

void KpkReviewChanges::checkTask()
{
    if (!m_remPackages.isEmpty()) {
        kDebug() << "task rm if";
        if (m_actions & Client::ActionRemovePackages) {
            if (m_actions & Client::ActionSimulateRemovePackages &&
                !(m_flags & HideConfirmDeps)) {
                m_reqDepPackages = m_remPackages;
                // Create the requirements transaction and it's model
                m_removePkgModel = new KpkSimulateModel(this);
                m_transactionReq = m_client->simulateRemovePackages(m_reqDepPackages);
                connect(m_transactionReq, SIGNAL(package(PackageKit::Package *)),
                        m_removePkgModel, SLOT(addPackage(PackageKit::Package *)));
                connect(m_transactionReq,
                        SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                        this,
                        SLOT(simRemFinished(PackageKit::Transaction::ExitStatus, uint)));
                // Create a Transaction dialog to don't upset the user
                KpkTransaction *kTrans = new KpkTransaction(m_transactionReq,
                                                            KpkTransaction::CloseOnFinish
                                                            | KpkTransaction::Modal,
                                                            this);
                if (!(m_flags & HideProgress)) {
                    kTrans->show();
                }
            } else {
                // As we can't check for requires don't allow deps removal
                removePackages(false);
            }
        } else {
            KMessageBox::error(this, i18n("The current backend does not support removing packages."), i18n("KPackageKit Error"));
            removeDone();
        }
    } else if (!m_addPackages.isEmpty()) {
        kDebug() << "task add else";
        if (m_actions & Client::ActionInstallPackages) {
            if (m_actions & Client::ActionSimulateInstallPackages &&
                !(m_flags & HideConfirmDeps)) {
                m_reqDepPackages = m_addPackages;
                // Create the depends transaction and it's model
                m_installPkgModel = new KpkSimulateModel(this);
                m_transactionDep = m_client->simulateInstallPackages(m_reqDepPackages);
                connect(m_transactionDep, SIGNAL(package(PackageKit::Package *)),
                        m_installPkgModel, SLOT(addPackage(PackageKit::Package *)));
                connect(m_transactionDep,
                        SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                        this,
                        SLOT(simInstFinished(PackageKit::Transaction::ExitStatus, uint)));
                // Create a Transaction dialog to don't upset the user
                KpkTransaction *kTrans = new KpkTransaction(m_transactionDep,
                                                            KpkTransaction::CloseOnFinish
                                                            | KpkTransaction::Modal,
                                                            this);
                if (!(m_flags & HideProgress)) {
                    kTrans->show();
                }
            } else {
                installPackages();
            }
        } else {
            KMessageBox::error(this, i18n("Current backend does not support installing packages."), i18n("KPackageKit Error"));
            installDone();
        }
    } else {
        kDebug() << "No more tasks to do";
        slotButtonClicked(KDialog::Ok);
    }
}

void KpkReviewChanges::simInstFinished(PackageKit::Transaction::ExitStatus status,
                                       uint runtime)
{
    Q_UNUSED(runtime)
    kDebug();
    if (status == Transaction::ExitSuccess) {
        if (m_installPkgModel->rowCount() > 0) {
            KpkRequirements *frm = new KpkRequirements(m_installPkgModel, this);
            if (frm->exec() == QDialog::Accepted) {
                installPackages();
            } else {
                close();
            }
        } else {
            installPackages();
        }
    } else {
        kDebug() << "Failed " << status;
        installDone();
    }
}

void KpkReviewChanges::simRemFinished(PackageKit::Transaction::ExitStatus status, uint runtime)
{
    Q_UNUSED(runtime)
    kDebug();
    if (status == Transaction::ExitSuccess) {
        if (m_removePkgModel->rowCount() > 0) {
            KpkRequirements *frm = new KpkRequirements(m_removePkgModel, this);
            if (frm->exec() == QDialog::Accepted) {
                removePackages();
            } else {
                close();
            }
        } else {
            // As there was no requires don't allow deps removal
            removePackages(false);
        }
    } else {
        kDebug() << "Failed " << status;
        removeDone();
    }
}

void KpkReviewChanges::installPackages()
{
    kDebug();
    SET_PROXY
    Transaction *t = m_client->installPackages(true, m_addPackages);
    if (t->error()) {
        KMessageBox::sorry(this,
                           KpkStrings::daemonError(t->error()),
                           i18n("Failed to install package"));
        installDone();
    } else {
        KpkTransaction *frm = new KpkTransaction(t, KpkTransaction::CloseOnFinish | KpkTransaction::Modal, this);
        if (m_flags & ReturnOnlyWhenFinished) {
            connect(t,
                    SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                    this,
                    SLOT(ensureInstallFinished(PackageKit::Transaction::ExitStatus, uint)));
        } else {
            connect(frm, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
                    this, SLOT(addFinished(KpkTransaction::ExitStatus)));
        }

        if (!(m_flags & HideProgress)) {
            frm->show();
        }
    }
}

void KpkReviewChanges::removePackages(bool allowDeps)
{
    kDebug();
    SET_PROXY
    Transaction *t = m_client->removePackages(m_remPackages, allowDeps, true);
    if (t->error()) {
        KMessageBox::sorry(this,
                           KpkStrings::daemonError(t->error()),
                           i18n("Failed to remove package"));
        removeDone();
    } else {
        KpkTransaction *frm = new KpkTransaction(t, KpkTransaction::CloseOnFinish | KpkTransaction::Modal, this);
        frm->setAllowDeps(allowDeps);
        if (m_flags & ReturnOnlyWhenFinished) {
            connect(t,
                    SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                    this,
                    SLOT(ensureRemoveFinished(PackageKit::Transaction::ExitStatus, uint)));
        } else {
            connect(frm, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
                    this, SLOT(remFinished(KpkTransaction::ExitStatus)));
        }

        if (!(m_flags & HideProgress)) {
            frm->show();
        }
    }
}

void KpkReviewChanges::addFinished(KpkTransaction::ExitStatus status)
{
    kDebug() << status;
    KpkTransaction *trans = (KpkTransaction *) sender();
    switch (status) {
    case KpkTransaction::Success :
        installDone();
        break;
    case KpkTransaction::Failed :
    case KpkTransaction::Cancelled :
        slotButtonClicked(KDialog::Close);
        break;
    case KpkTransaction::ReQueue :
        SET_PROXY
        trans->setTransaction(m_client->installPackages(trans->onlyTrusted(),
                                                        m_addPackages));
        break;
    }
}

void KpkReviewChanges::remFinished(KpkTransaction::ExitStatus status)
{
    KpkTransaction *trans = (KpkTransaction *) sender();
    switch (status) {
    case KpkTransaction::Success :
        removeDone();
        break;
    case KpkTransaction::Failed :
    case KpkTransaction::Cancelled :
        slotButtonClicked(KDialog::Close);
        break;
    case KpkTransaction::ReQueue :
        SET_PROXY
        trans->setTransaction(m_client->removePackages(m_remPackages,
                                                       trans->allowDeps(),
                                                       AUTOREMOVE));
        break;
    }
}

void KpkReviewChanges::ensureRemoveFinished(PackageKit::Transaction::ExitStatus status,
                                            uint runtime)
{
    Q_UNUSED(runtime)
    kDebug();
    if (status == Transaction::ExitSuccess) {
        removeDone();
    } else {
        slotButtonClicked(KDialog::Cancel);
    }
}

void KpkReviewChanges::ensureInstallFinished(PackageKit::Transaction::ExitStatus status,
                                             uint runtime)
{
    Q_UNUSED(runtime)
    kDebug();
    if (status == Transaction::ExitSuccess) {
        installDone();
    } else {
        slotButtonClicked(KDialog::Cancel);
    }
}

void KpkReviewChanges::installDone()
{
    m_addPackages.clear();
    checkTask();
}

void KpkReviewChanges::removeDone()
{
    m_remPackages.clear();
    checkTask();
}

void KpkReviewChanges::slotButtonClicked(int button)
{
    switch(button) {
    case KDialog::Cancel :
    case KDialog::Close :
        reject();
        break;
    case KDialog::Ok :
        accept();
        break;
    case KDialog::Apply :
        hide();
        doAction();
        break;
    default :
        KDialog::slotButtonClicked(button);
    }
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
