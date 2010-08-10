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

#include "KpkReviewChanges.h"

#include <KpkMacros.h>
#include <KMessageBox>

#include <KDebug>

#include "KpkStrings.h"
#include "KpkEnum.h"
#include "KpkRequirements.h"
#include "KpkSimulateModel.h"
#include "KpkPackageModel.h"
#include "KpkTransaction.h"
#include "KpkDelegate.h"

#include "ui_KpkReviewChanges.h"

#define UNIVERSAL_PADDING 6
#define AUTOREMOVE true

class KpkReviewChangesPrivate
{
public:
    Ui::KpkReviewChanges ui;

    KpkPackageModel *mainPkgModel;
    KpkSimulateModel *installPkgModel, *removePkgModel;
    KpkDelegate *pkgDelegate;

    Client *client;

    QList<QSharedPointer<PackageKit::Package> > remPackages;
    QList<QSharedPointer<PackageKit::Package> > addPackages;
    QList<QSharedPointer<PackageKit::Package> > reqDepPackages;

    Enum::Roles actions;
};

KpkReviewChanges::KpkReviewChanges(const QList<QSharedPointer<PackageKit::Package> > &packages, QWidget *parent)
 : KDialog(parent),
   d(new KpkReviewChangesPrivate),
   m_flags(Default)
{
    d->ui.setupUi(mainWidget());

    d->client = Client::instance();

    //initialize the model, delegate, client and  connect it's signals
    d->ui.packageView->setItemDelegate(d->pkgDelegate = new KpkDelegate(d->ui.packageView));
    d->ui.packageView->setModel(d->mainPkgModel = new KpkPackageModel(packages, this, d->ui.packageView));
//     d->mainPkgModel->checkAll();//TODO check this
    d->ui.packageView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(d->mainPkgModel, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)),
            this, SLOT(checkChanged()));

    // Set Apply and Cancel buttons
    setButtons(KDialog::Apply | KDialog::Cancel);

    // Count how many items will be installed and removed to set
    // better apply text and description text
    int countRemove  = 0;
    int countInstall = 0;
    foreach (QSharedPointer<PackageKit::Package>package, packages) {
        // If the package is installed we are going to remove it
        if (package->info() == Enum::InfoInstalled) {
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
        show();
    } else {
        // Starts the action without showing the dialog
        QTimer::singleShot(0, this, SLOT(doAction()));
    }

    QEventLoop loop;
    connect(this, SIGNAL(finished(int)), &loop, SLOT(quit()));
    loop.exec();

    return result();
}

void KpkReviewChanges::doAction()
{
    d->actions = d->client->actions();
    // check what packages are installed and marked to be removed
    for (int i = 0; i < d->mainPkgModel->selectedPackages().size(); ++i) {
        if (d->mainPkgModel->selectedPackages().at(i)->info() == Enum::InfoInstalled) {
            d->remPackages << d->mainPkgModel->selectedPackages().takeAt(i);
        }
    }

    // check what packages are available and marked to be installed
    for (int i = 0; i < d->mainPkgModel->selectedPackages().size(); ++i) {
        if (d->mainPkgModel->selectedPackages().at(i)->info() == Enum::InfoAvailable) {
            d->addPackages << d->mainPkgModel->selectedPackages().takeAt(i);
        }
    }

    checkTask();
}

void KpkReviewChanges::checkTask()
{
    if (!d->remPackages.isEmpty()) {
        if (d->actions & Enum::RoleRemovePackages) {
            if (d->actions & Enum::RoleSimulateRemovePackages &&
                !(m_flags & HideConfirmDeps)) {
                d->reqDepPackages = d->remPackages;
                d->removePkgModel = new KpkSimulateModel(this, d->reqDepPackages);
                // Create a Transaction dialog to don't upset the user
                QPointer<KpkTransaction> kTrans = new KpkTransaction(0,
                                                                     KpkTransaction::CloseOnFinish
                                                                     | KpkTransaction::Modal,
                                                                     this);
                // Create the requirements transaction and it's model
                Transaction *trans;
                trans = d->client->simulateRemovePackages(d->reqDepPackages, true);
                if (trans->error()) {
                    KMessageBox::sorry(this,
                                       KpkStrings::daemonError(trans->error()),
                                       i18n("Failed to simulate package removal"));
                    taskDone(Enum::RoleRemovePackages);
                } else {
                    kTrans->setTransaction(trans);
                    connect(trans, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                            d->removePkgModel, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
                    connect(trans, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                            this, SLOT(simulateFinished(PackageKit::Enum::Exit)));

                    if (!(m_flags & HideProgress)) {
                        kTrans->exec();
                    }
                    delete kTrans;
                }
            } else {
                // As we can't check for requires don't allow deps removal
                removePackages(false);
            }
        } else {
            KMessageBox::error(this, i18n("The current backend does not support removing packages."), i18n("KPackageKit Error"));
            taskDone(Enum::RoleRemovePackages);
        }
    } else if (!d->addPackages.isEmpty()) {
        if (d->actions & Enum::RoleInstallPackages) {
            if (d->actions & Enum::RoleSimulateInstallPackages &&
                !(m_flags & HideConfirmDeps)) {
                d->reqDepPackages = d->addPackages;
                d->installPkgModel = new KpkSimulateModel(this, d->reqDepPackages);
                // Create a Transaction dialog to don't upset the user
                QPointer<KpkTransaction> kTrans = new KpkTransaction(0,
                                                                     KpkTransaction::CloseOnFinish
                                                                     | KpkTransaction::Modal,
                                                                     this);
                // Create the depends transaction and it's model
                Transaction *trans;
                trans = d->client->simulateInstallPackages(d->reqDepPackages);
                if (trans->error()) {
                    KMessageBox::sorry(this,
                                       KpkStrings::daemonError(trans->error()),
                                       i18n("Failed to simulate package install"));
                    taskDone(Enum::RoleInstallPackages);
                } else {
                    kTrans->setTransaction(trans);
                    connect(trans, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                            d->installPkgModel, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
                    connect(trans, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                            this, SLOT(simulateFinished(PackageKit::Enum::Exit)));

                    if (!(m_flags & HideProgress)) {
                        kTrans->exec();
                    }
                }
                delete kTrans;
            } else {
                installPackages();
            }
        } else {
            KMessageBox::error(this, i18n("Current backend does not support installing packages."), i18n("KPackageKit Error"));
            taskDone(Enum::RoleInstallPackages);
        }
    } else {
        slotButtonClicked(KDialog::Ok);
    }
}

void KpkReviewChanges::simulateFinished(PackageKit::Enum::Exit status)
{
    Transaction *trans = qobject_cast<Transaction *>(sender());
    if (status == Enum::ExitSuccess) {
        switch (trans->role()) {
        case Enum::RoleSimulateRemovePackages:
            {
                if (d->removePkgModel->rowCount() > 0) {
                    QPointer<KpkRequirements> frm = new KpkRequirements(d->removePkgModel, this);
                    if (frm->exec() == QDialog::Accepted) {
                        removePackages();
                    } else {
                        reject();
                    }
                    delete frm;
                } else {
                    // As there was no requires don't allow deps removal
                    removePackages(false);
                }
            }
            break;
        case Enum::RoleSimulateInstallPackages:
            {
                if (d->installPkgModel->rowCount() > 0) {
                    QPointer<KpkRequirements> frm = new KpkRequirements(d->installPkgModel, this);
                    if (frm->exec() == QDialog::Accepted) {
                        installPackages();
                    } else {
                        reject();
                    }
                    delete frm;
                } else {
                    installPackages();
                }
            }
            break;

        default:
            break;
        }
    } else {
        taskDone(trans->role());
    }
}

void KpkReviewChanges::installPackages()
{
    SET_PROXY
    Transaction *t = d->client->installPackages(true, d->addPackages);
    if (t->error()) {
        KMessageBox::sorry(this,
                           KpkStrings::daemonError(t->error()),
                           i18n("Failed to install package"));
        taskDone(Enum::RoleInstallPackages);
    } else {
        QPointer<KpkTransaction> frm = new KpkTransaction(t, KpkTransaction::CloseOnFinish | KpkTransaction::Modal, this);
        if (m_flags & ReturnOnlyWhenFinished) {
            connect(t,
                    SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                    this,
                    SLOT(ensureTransactionFinished(PackageKit::Enum::Exit)));
        } else {
            connect(frm, SIGNAL(requeue()),
                    this, SLOT(requeueTransaction()));
        }

        if (!(m_flags & HideProgress)) {
            // for modal dialogs to work correctly exec must be called
            frm->exec();
            switch (frm->exitStatus()) {
            case KpkTransaction::Success:
                delete frm;
                taskDone(Enum::RoleInstallPackages);
                break;
            case KpkTransaction::Cancelled:
                delete frm;
                slotButtonClicked(KDialog::Close);
                break;
            default:
                delete frm;
            }
        }
    }
}

void KpkReviewChanges::removePackages(bool allowDeps)
{
    SET_PROXY
    Transaction *t = d->client->removePackages(d->remPackages, allowDeps, true);
    if (t->error()) {
        KMessageBox::sorry(this,
                           KpkStrings::daemonError(t->error()),
                           i18n("Failed to remove package"));
        taskDone(Enum::RoleRemovePackages);
    } else {
        QPointer<KpkTransaction> frm = new KpkTransaction(t, KpkTransaction::CloseOnFinish | KpkTransaction::Modal, this);
        frm->setAllowDeps(allowDeps);
        if (m_flags & ReturnOnlyWhenFinished) {
            connect(t,
                    SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                    this,
                    SLOT(ensureTransactionFinished(PackageKit::Enum::Exit)));
        } else {
            connect(frm, SIGNAL(requeue()),
                    this, SLOT(requeueTransaction()));
        }

        if (!(m_flags & HideProgress)) {
            // for modal dialogs to work correctly exec must be called
            frm->exec();
            switch (frm->exitStatus()) {
            case KpkTransaction::Success:
                delete frm;
                taskDone(Enum::RoleRemovePackages);
                break;
            case KpkTransaction::Cancelled:
                delete frm;
                slotButtonClicked(KDialog::Close);
                break;
            default:
                delete frm;
            }
        }
    }
}

void KpkReviewChanges::requeueTransaction()
{
    KpkTransaction *trans = qobject_cast<KpkTransaction *>(sender());
    SET_PROXY
    if (trans->role() == Enum::RoleRemovePackages) {
        trans->setTransaction(d->client->removePackages(d->remPackages,
                                                       trans->allowDeps(),
                                                       AUTOREMOVE));
    } else if (trans->role() == Enum::RoleInstallPackages) {
        trans->setTransaction(d->client->installPackages(trans->onlyTrusted(),
                                                        d->addPackages));
    }
}

void KpkReviewChanges::ensureTransactionFinished(PackageKit::Enum::Exit status)
{
    Transaction *trans = qobject_cast<Transaction *>(sender());
    if (status == Enum::ExitSuccess) {
        taskDone(trans->role());
    } else {
        slotButtonClicked(KDialog::Cancel);
    }
}

void KpkReviewChanges::taskDone(PackageKit::Enum::Role role)
{
    if (role == Enum::RoleRemovePackages) {
        d->remPackages.clear();
    } else if (role == Enum::RoleInstallPackages) {
        d->addPackages.clear();
    }
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

// void KpkReviewChanges::resizeEvent(QResizeEvent *event)
// {
//     QWidget::resizeEvent(event);
//     updateColumnsWidth();
// }
// 
// bool KpkReviewChanges::event(QEvent *event)
// {
//     switch (event->type()) {
//         case QEvent::PolishRequest:
//         case QEvent::Polish:
//             updateColumnsWidth(true);
//             break;
//         default:
//             break;
//     }
// 
//     return QWidget::event(event);
// }
// 
// void KpkReviewChanges::updateColumnsWidth(bool force)
// {
//     int viewWidth = d->ui.packageView->viewport()->width();
// 
//     if (force) {
//         viewWidth -= style()->pixelMetric(QStyle::PM_ScrollBarExtent) + UNIVERSAL_PADDING;
//     }
// 
//     d->ui.packageView->setColumnWidth(0, d->pkgDelegate->columnWidth(0, viewWidth));
//     d->ui.packageView->setColumnWidth(1, d->pkgDelegate->columnWidth(1, viewWidth));
// }

void KpkReviewChanges::checkChanged()
{
    if (d->mainPkgModel->selectedPackages().size() > 0) {
        enableButtonApply(true);
    } else {
        enableButtonApply(false);
    }
}

#include "KpkReviewChanges.moc"
