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

#include "ReviewChanges.h"

#include <KMessageBox>
#include <KWindowSystem>
#include <KCategorizedSortFilterProxyModel>

#include <KDebug>

#include <Daemon>

#include "KpkMacros.h"
#include "KpkStrings.h"
#include "KpkEnum.h"
#include "KpkRequirements.h"
#include "PackageModel.h"
#include "PkTransactionDialog.h"
#include "KpkDelegate.h"

#include "ui_ReviewChanges.h"

class ReviewChangesPrivate
{
public:
    Ui::ReviewChanges ui;

    PackageModel *mainPkgModel;
    KpkDelegate *pkgDelegate;

    QList<Package> remPackages;
    QList<Package> addPackages;

    Transaction::Roles actions;

    PkTransactionDialog *transactionDialog;
};

ReviewChanges::ReviewChanges(const QList<Package> &packages,
                             QWidget *parent)
 : QWidget(parent),
   d(new ReviewChangesPrivate),
   m_flags(Default)
{
    d->ui.setupUi(this);

    d->transactionDialog = 0;

    //initialize the model, delegate, client and  connect it's signals
    d->ui.packageView->viewport()->setAttribute(Qt::WA_Hover);
    d->mainPkgModel = new PackageModel(this);
    KCategorizedSortFilterProxyModel *changedProxy = new KCategorizedSortFilterProxyModel(this);
    changedProxy->setSourceModel(d->mainPkgModel);
    changedProxy->setCategorizedModel(true);
    changedProxy->sort(0);
    changedProxy->setDynamicSortFilter(true);
    changedProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    changedProxy->setSortRole(PackageModel::SortRole);
    d->ui.packageView->setModel(changedProxy);
    d->pkgDelegate = new KpkDelegate(d->ui.packageView);
    d->pkgDelegate->setExtendPixmapWidth(0);
    d->ui.packageView->setItemDelegate(d->pkgDelegate);
    d->mainPkgModel->addPackages(packages, true);
    connect(d->mainPkgModel, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)),
            this, SLOT(checkChanged()));
}

ReviewChanges::~ReviewChanges()
{
    // Make sure the dialog is deleted in case we are not it's parent
    if (d->transactionDialog) {
        d->transactionDialog->deleteLater();
    }

    delete d;
}

QString ReviewChanges::title() const
{
    return i18np("The following package was found",
                 "The following packages were found",
                 d->mainPkgModel->rowCount());
}

int ReviewChanges::exec(OperationModes flags)
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

    return QDialog::Accepted;
}

void ReviewChanges::doAction()
{
    // Fix up the parent when this class is not shown
    QWidget *transParent = qobject_cast<QWidget*>(parent());
    if (m_flags & ShowConfirmation) {
        transParent = this;
    }

    d->actions = Daemon::actions();
    d->remPackages.clear();
    d->addPackages.clear();

    foreach (const Package &p, d->mainPkgModel->selectedPackages()) {
        if (p.info() == Package::InfoInstalled ||
            p.info() == Package::InfoCollectionInstalled) {
            // check what packages are installed and marked to be removed
            d->remPackages << p;
        } else if (p.info() == Package::InfoAvailable ||
                   p.info() == Package::InfoCollectionAvailable) {
            // check what packages are available and marked to be installed
            d->addPackages << p;
        }
    }

    if (!d->addPackages.isEmpty() || !d->remPackages.isEmpty()) {
        d->transactionDialog = new PkTransactionDialog(0,
                                                  PkTransactionDialog::Modal,
                                                  transParent);
        connect(d->transactionDialog, SIGNAL(finished(PkTransaction::ExitStatus)),
                this, SLOT(transactionFinished(PkTransaction::ExitStatus)));

        d->transactionDialog->show();

        checkTask();
    } else {
//        reject();
    }
}

void ReviewChanges::checkTask()
{
    if (!d->remPackages.isEmpty()) {
        d->transactionDialog->transaction()->removePackages(d->remPackages);
    } else if (!d->addPackages.isEmpty()) {
        d->transactionDialog->transaction()->installPackages(d->addPackages);
    } else {
        slotButtonClicked(KDialog::Ok);
    }
}

void ReviewChanges::transactionFinished(PkTransaction::ExitStatus status)
{
    PkTransactionDialog *trans = qobject_cast<PkTransactionDialog*>(sender());
    if (status == PkTransaction::Success) {
        switch (trans->transaction()->role()) {
        case Transaction::RoleRemovePackages:
            emit successfullyRemoved();
            taskDone(trans->transaction()->role());
            break;
        case Transaction::RoleInstallPackages:
            emit successfullyInstalled();
            taskDone(trans->transaction()->role());
            break;
        default:
            kWarning() << "Role not Handled" << trans->transaction()->role();
            break;
        }
    } else {
        slotButtonClicked(KDialog::Cancel);
    }
}

void ReviewChanges::taskDone(Transaction::Role role)
{
    if (role == Transaction::RoleRemovePackages) {
        d->remPackages.clear();
    } else if (role == Transaction::RoleInstallPackages) {
        d->addPackages.clear();
    }
    checkTask();
}

void ReviewChanges::slotButtonClicked(int button)
{
    switch(button) {
    case KDialog::Cancel :
    case KDialog::Close :
//        reject();
        break;
    case KDialog::Ok :
//        accept();
        break;
    case KDialog::Apply :
        hide();
        doAction();
        break;
//    default :
//        KDialog::slotButtonClicked(button);
    }
}

void ReviewChanges::checkChanged()
{
    if (d->mainPkgModel->selectedPackages().size() > 0) {
//        enableButtonApply(true);
    } else {
//        enableButtonApply(false);
    }
}

#include "ReviewChanges.moc"
