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

#include "KpkUpdate.h"
#include "KpkUpdateDetails.h"
#include "KpkDistroUpgrade.h"
#include "KpkHistory.h"
#include "KpkMacros.h"

#include <QDBusConnection>

#include <KpkStrings.h>
#include <KpkIcons.h>
#include <KpkTransactionBar.h>
#include <KpkPackageModel.h>
#include <KpkSimulateModel.h>
#include <KpkDelegate.h>
#include <KpkRequirements.h>

#include <KMessageBox>
#include <KDebug>

#define UNIVERSAL_PADDING 6

KpkUpdate::KpkUpdate(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    selectAllPB->setIcon(KpkIcons::getIcon("package-update"));
    refreshPB->setIcon(KpkIcons::getIcon("view-refresh"));
    historyPB->setIcon(KpkIcons::getIcon("view-history"));
    transactionBar->setBehaviors(KpkTransactionBar::AutoHide);

    Client::instance()->setLocale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());

    //initialize the model, delegate, client and  connect it's signals
    packageView->setItemDelegate(pkg_delegate = new KpkDelegate(packageView));
    packageView->setModel(m_pkg_model_updates = new KpkPackageModel(this, packageView));
    m_pkg_model_updates->setGrouped(true);
    connect(m_pkg_model_updates, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)),
            this, SLOT(checkEnableUpdateButton()));

    // Create a new client
    m_client = Client::instance();

    // check to see what roles the backend
    m_actions = m_client->getActions();

    // hide distro Upgrade container and line
    distroUpgradesSA->hide();
    line->hide();
}

//TODO: We should add some kind of configuration to let users show unstable distributions
//That way, by default, users only see stable ones.
void KpkUpdate::distroUpgrade(PackageKit::Client::DistroUpgradeType type, const QString &name, const QString &description)
{
    Q_UNUSED(type)
    if (verticalLayout->count()) {

        QFrame *frame = new QFrame(this);
        frame->setFrameShape(QFrame::HLine);
        verticalLayout->insertWidget(0, frame);
    }
    KpkDistroUpgrade *distro = new KpkDistroUpgrade(this);
    verticalLayout->insertWidget(0, distro);
    distro->setComment(description);
    distro->setName(name);
    distroUpgradesSA->show();
    line->show();
}

void KpkUpdate::checkEnableUpdateButton()
{
    emit changed(m_pkg_model_updates->selectedPackages().size() > 0);
    // if we don't have any upates let's disable the button
    selectAllPB->setEnabled(m_pkg_model_updates->rowCount() != 0);
}

void KpkUpdate::on_selectAllPB_clicked()
{
    m_pkg_model_updates->checkAll();
}

void KpkUpdate::load()
{
    displayUpdates(KpkTransaction::Success);
}

void KpkUpdate::applyUpdates()
{
    // If the backend supports getRequires do it
    if (m_actions & Client::ActionSimulateUpdatePackages) {

        PackageKit::Transaction *t;
        KpkSimulateModel *simulateModel = new KpkSimulateModel(this);

        t = m_client->simulateUpdatePackages(m_pkg_model_updates->selectedPackages());
        if (t->error()) {
                KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
        } else {
                connect(t, SIGNAL(package(PackageKit::Package *)),
                        simulateModel, SLOT(addPackage(PackageKit::Package *)));

                // Create a Transaction dialog to don't upset the user
                QPointer<KpkTransaction> reqFinder = new KpkTransaction(t, KpkTransaction::CloseOnFinish | KpkTransaction::Modal, this);
                reqFinder->exec();
                if (reqFinder->exitStatus() == KpkTransaction::Success) {
                    if (simulateModel->rowCount(QModelIndex()) > 0) {
                        QPointer<KpkRequirements> rq = new KpkRequirements(simulateModel, this);
                        if (rq->exec() == QDialog::Accepted) {
                            updatePackages();
                        }
                        delete rq;
                    } else {
                        updatePackages();
                    }
                }
                delete reqFinder;
        }
    } else {
        updatePackages();
    }
}

void KpkUpdate::getUpdatesFinished(Transaction::ExitStatus status, uint runtime)
{
    Q_UNUSED(status)
    Q_UNUSED(runtime)

    // If we just have one group let's expand it
    if (m_pkg_model_updates->rowCount() == 1) {
        packageView->expandAll();
    } else if (m_pkg_model_updates->rowCount() == 2) {
        int nonBlockedIndex = -1;
        if (m_pkg_model_updates->data(m_pkg_model_updates->index(0,0),
                                      KpkPackageModel::StateRole).toUInt()
            == Package::StateBlocked) {
            // We got a blocked update in index 0, so 1 is not blocked...
            nonBlockedIndex = 1;
        }
        if (m_pkg_model_updates->data(m_pkg_model_updates->index(1,0),
                                      KpkPackageModel::StateRole).toUInt()
            == Package::StateBlocked) {
            // We got a blocked update in index 1, so 0 is not blocked...
            nonBlockedIndex = 0;
        }

        if (nonBlockedIndex != -1) {
            // we have a blocked update expand the non blocked one
            packageView->expand(m_pkg_model_updates->index(nonBlockedIndex,0));
        }
    }
    checkEnableUpdateButton();
}

void KpkUpdate::updatePackages()
{
    QList<Package*> packages = m_pkg_model_updates->selectedPackages();

    SET_PROXY
    Transaction *t = m_client->updatePackages(true, packages);
    if (t->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
    } else {
        KpkTransaction *frm = new KpkTransaction(t, KpkTransaction::Modal | KpkTransaction::CloseOnFinish, this);
        frm->setPackages(packages);
        connect(frm, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
                this, SLOT(displayUpdates(KpkTransaction::ExitStatus)));
        frm->exec();
    }
}

void KpkUpdate::refresh()
{
    SET_PROXY
    Transaction *t = m_client->refreshCache(true);
    if (t->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
    } else {
        KpkTransaction *frm = new KpkTransaction(t, KpkTransaction::Modal | KpkTransaction::CloseOnFinish, this);
        connect(frm, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
                 this, SLOT(displayUpdates(KpkTransaction::ExitStatus)));
        frm->show();
    }
}

void KpkUpdate::displayUpdates(KpkTransaction::ExitStatus status)
{
    checkEnableUpdateButton();
    KpkTransaction *trans = (KpkTransaction *) sender();
    if (status == KpkTransaction::Success) {
        // contract to delete all update details widgets
        pkg_delegate->contractAll();
        // clears the model
        m_pkg_model_updates->clear();
        m_pkg_model_updates->uncheckAll();
        m_updatesT = m_client->getUpdates();
        if (m_updatesT->error()) {
            KMessageBox::sorry(this, KpkStrings::daemonError(m_updatesT->error()));
        } else {
            transactionBar->addTransaction(m_updatesT);
            connect(m_updatesT, SIGNAL(package(PackageKit::Package *)),
                    m_pkg_model_updates, SLOT(addPackage(PackageKit::Package *)));
            connect(m_updatesT, SIGNAL(errorCode(PackageKit::Client::ErrorType, const QString &)),
                    this, SLOT(errorCode(PackageKit::Client::ErrorType, const QString &)));
            connect(m_updatesT, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                    this, SLOT(getUpdatesFinished(PackageKit::Transaction::ExitStatus, uint) ) );
        }

        // Clean the distribution upgrades area
        QLayoutItem *child;
        while ((child = verticalLayout->takeAt(0)) != 0) {
            delete child->widget();
            delete child;
        }
        distroUpgradesSA->hide();
        line->hide();

        // Check for distribution Upgrades
        Transaction *t = m_client->getDistroUpgrades();
        if (!t->error()) {
            transactionBar->addTransaction(t);
            connect(t, SIGNAL(distroUpgrade(PackageKit::Client::DistroUpgradeType, const QString &, const QString &)),
                    this, SLOT(distroUpgrade(PackageKit::Client::DistroUpgradeType, const QString &, const QString &)));
        }
    } else if (status == KpkTransaction::ReQueue) {
        SET_PROXY
        Transaction *t = m_client->updatePackages(trans->onlyTrusted(), trans->packages());
        if (t->error()) {
                KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
                trans->deleteLater();
        } else {
                trans->setTransaction(t);
        }
    }
}

void KpkUpdate::on_refreshPB_clicked()
{
    refresh();
}

void KpkUpdate::on_packageView_pressed(const QModelIndex &index)
{
    if (index.column() == 0) {
        Package *p = m_pkg_model_updates->package(index);
        // check to see if the backend support
        if (p && (m_actions & Client::ActionGetUpdateDetail)) {
            if (pkg_delegate->isExtended(index)) {
                pkg_delegate->contractItem(index);
            } else {
                pkg_delegate->extendItem(new KpkUpdateDetails(p), index);
            }
        }
    }
}

void KpkUpdate::on_historyPB_clicked()
{
    QPointer<KpkHistory> frm = new KpkHistory(this);
    frm->exec();
    delete frm;
}

void KpkUpdate::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateColumnsWidth();
}

bool KpkUpdate::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::Paint:
        case QEvent::PolishRequest:
        case QEvent::Polish:
            updateColumnsWidth(true);
            break;
        default:
            break;
    }

    return QWidget::event(event);
}

void KpkUpdate::errorCode(PackageKit::Client::ErrorType error, const QString &details)
{
    KMessageBox::detailedSorry(this,
                               KpkStrings::errorMessage(error),
                               details,
                               KpkStrings::error(error),
                               KMessageBox::Notify);
}

void KpkUpdate::updateColumnsWidth(bool force)
{
    int m_viewWidth = packageView->viewport()->width();

    if (force) {
        m_viewWidth -= style()->pixelMetric(QStyle::PM_ScrollBarExtent) + UNIVERSAL_PADDING;
    }

    packageView->setColumnWidth(0, pkg_delegate->columnWidth(0, m_viewWidth));
    packageView->setColumnWidth(1, pkg_delegate->columnWidth(1, m_viewWidth));
}

#include "KpkUpdate.moc"
