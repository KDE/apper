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

#include "KcmKpkUpdate.h"

#include "KpkUpdateDetails.h"
#include "KpkDistroUpgrade.h"
#include "KpkHistory.h"
#include "KpkMacros.h"
#include "KpkCheckableHeader.h"

#include <version.h>

#include <KGenericFactory>
#include <KAboutData>
#include <KpkStrings.h>
#include <KpkIcons.h>
#include <KpkTransactionBar.h>
#include <KpkTransaction.h>
#include <KpkSimulateModel.h>
#include <KpkRequirements.h>
#include <KpkPackageModel.h>
#include <KpkDelegate.h>
#include <QSortFilterProxyModel>
#include <QDBusConnection>

#include <KMessageBox>
#include <KDebug>

#define UNIVERSAL_PADDING 6

K_PLUGIN_FACTORY(KPackageKitFactory, registerPlugin<KcmKpkUpdate>();)
K_EXPORT_PLUGIN(KPackageKitFactory("kcm_kpk_update"))

KcmKpkUpdate::KcmKpkUpdate(QWidget *&parent, const QVariantList &args)
    : KCModule(KPackageKitFactory::componentData(), parent, args),
      m_updatesT(0)
{
    KAboutData *aboutData;
    aboutData = new KAboutData("kpackagekit",
                               "kpackagekit",
                               ki18n("Software update"),
                               KPK_VERSION,
                               ki18n("KDE interface for updating software"),
                               KAboutData::License_GPL,
                               ki18n("(C) 2008-2010 Daniel Nicoletti"));
    setAboutData(aboutData);
//     setButtons(Apply);
    KGlobal::locale()->insertCatalog("kpackagekit");

    m_selected = !args.isEmpty();
    setupUi(this);

    refreshPB->setIcon(KpkIcons::getIcon("view-refresh"));
    historyPB->setIcon(KpkIcons::getIcon("view-history"));
    transactionBar->setBehaviors(KpkTransactionBar::AutoHide);

    QString locale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
    Client::instance()->setHints("locale=" + locale);

    //initialize the model, delegate, client and  connect it's signals
    m_header = new KpkCheckableHeader(Qt::Horizontal, this);
    m_header->setResizeMode(QHeaderView::Stretch);
    m_header->setCheckBoxEnabled(false);

    m_updatesModel = new KpkPackageModel(this, packageView);
    m_updatesModel->setCheckable(true);
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_updatesModel);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setSortRole(KpkPackageModel::SortRole);
    packageView->setHeader(m_header);
    packageView->setItemDelegate(m_delegate = new KpkDelegate(packageView));
    packageView->setModel(proxyModel);
    packageView->sortByColumn(0, Qt::AscendingOrder);
    connect(m_delegate, SIGNAL(showExtendItem(const QModelIndex &)),
            this, SLOT(showExtendItem(const QModelIndex &)));
    connect(m_header, SIGNAL(sectionClicked(int)),
            this, SLOT(contractAll()));
    connect(m_header, SIGNAL(toggled(bool)),
            m_updatesModel, SLOT(setAllChecked(bool)));
    connect(m_updatesModel, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)),
            this, SLOT(checkEnableUpdateButton()));

    // Create a new client
    m_client = Client::instance();

    // check to see what roles the backend has
    m_roles = m_client->actions();

    // hide distro Upgrade container and line
    distroUpgradesSA->hide();
    line->hide();
}

//TODO: We should add some kind of configuration to let users show unstable distributions
//That way, by default, users only see stable ones.
void KcmKpkUpdate::distroUpgrade(PackageKit::Enum::DistroUpgrade type, const QString &name, const QString &description)
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

void KcmKpkUpdate::checkEnableUpdateButton()
{
    emit changed(m_updatesModel->selectedPackages().size() > 0);
    int selectedSize = m_updatesModel->selectedPackages().size();
    int updatesSize = m_updatesModel->rowCount();
    if (selectedSize == 0) {
        m_header->setCheckState(Qt::Unchecked);
    } else if (selectedSize == updatesSize) {
        m_header->setCheckState(Qt::Checked);
    } else {
        m_header->setCheckState(Qt::PartiallyChecked);
    }

    // if we don't have any upates let's disable the button
    m_header->setCheckBoxEnabled(m_updatesModel->rowCount() != 0);
}

void KcmKpkUpdate::load()
{
    // set focus on the updates view
    packageView->setFocus(Qt::OtherFocusReason);
    getUpdates();
}

void KcmKpkUpdate::getUpdatesFinished(Enum::Exit status)
{
    Q_UNUSED(status)
    m_updatesT = 0;
    checkEnableUpdateButton();
}

void KcmKpkUpdate::save()
{
    // If the backend supports getRequires do it
    m_transDialog = 0;
    if (m_roles & Enum::RoleSimulateUpdatePackages) {
        QList<QSharedPointer<PackageKit::Package> > selectedPackages;
        selectedPackages = m_updatesModel->selectedPackages();
        Transaction *t = m_client->simulateUpdatePackages(selectedPackages);
        if (t->error()) {
            KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
        } else {
            m_transDialog = new KpkTransaction(0, KpkTransaction::Modal, this);
            m_transDialog->setPackages(selectedPackages);
            m_transDialog->setTransaction(t); // Set it after the packages so they get skiped
            connect(m_transDialog, SIGNAL(finished(KpkTransaction::ExitStatus)),
                    this, SLOT(transactionFinished(KpkTransaction::ExitStatus)));
            m_transDialog->show();
        }
    } else {
        updatePackages();
    }
    QTimer::singleShot(0, this, SLOT(checkEnableUpdateButton()));
}

void KcmKpkUpdate::transactionFinished(KpkTransaction::ExitStatus status)
{
    if (status == KpkTransaction::Success &&
        m_transDialog->role() == Enum::RoleSimulateUpdatePackages) {
        if (m_transDialog->simulateModel()->rowCount() > 0) {
            KpkRequirements *req = new KpkRequirements(m_transDialog->simulateModel(),
                                                       m_transDialog);
            connect(req, SIGNAL(accepted()), this, SLOT(updatePackages()));
            connect(req, SIGNAL(rejected()), m_transDialog, SLOT(reject()));
            req->show();
        } else {
            updatePackages();
        }
    } else {
        getUpdates();
        m_transDialog->deleteLater();
        checkEnableUpdateButton();
    }
}

#include <QDBusMessage>
void KcmKpkUpdate::updatePackages()
{
    QList<QSharedPointer<PackageKit::Package> > packages;
    if (m_transDialog) {
        packages = m_transDialog->packages();
    } else {
        packages = m_updatesModel->selectedPackages();
        m_transDialog = new KpkTransaction(0, KpkTransaction::Modal, this);
        connect(m_transDialog, SIGNAL(finished(KpkTransaction::ExitStatus)),
                this, SLOT(transactionFinished(KpkTransaction::ExitStatus)));
        m_transDialog->setPackages(packages); // Set Packages for requeue
    }

    SET_PROXY
    QString socket;
    socket = "/tmp/kpk_debconf_" + QString::number(QCoreApplication::applicationPid());
    m_client->setHints("frontend-socket=" + socket);
    Transaction *t = m_client->updatePackages(true, packages);
    if (t->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
    } else {
        m_transDialog->setTransaction(t);
        m_transDialog->setupDebconfDialog(socket);
        m_transDialog->show();
    }
}

void KcmKpkUpdate::getUpdates()
{
    kDebug() << sender();
    if (m_updatesT) {
        // There is a getUpdates running ignore this call
        return;
    }

    // contract to delete all update details widgets
    m_delegate->contractAll();

    // clears the model
    m_updatesModel->clear();
    m_updatesModel->setAllChecked(false);
    m_updatesT = m_client->getUpdates();
    if (m_updatesT->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(m_updatesT->error()));
    } else {
        transactionBar->addTransaction(m_updatesT);
        if (m_selected) {
            connect(m_updatesT, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                m_updatesModel, SLOT(addSelectedPackage(QSharedPointer<PackageKit::Package>)));
        } else {
            connect(m_updatesT, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                m_updatesModel, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
        }

        connect(m_updatesT, SIGNAL(errorCode(PackageKit::Enum::Error, const QString &)),
                this, SLOT(errorCode(PackageKit::Enum::Error, const QString &)));
        connect(m_updatesT, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                this, SLOT(getUpdatesFinished(PackageKit::Enum::Exit)));
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
        connect(t, SIGNAL(distroUpgrade(PackageKit::Enum::DistroUpgrade, const QString &, const QString &)),
                this, SLOT(distroUpgrade(PackageKit::Enum::DistroUpgrade, const QString &, const QString &)));
    }
}

void KcmKpkUpdate::on_refreshPB_clicked()
{
    SET_PROXY
    Transaction *t = m_client->refreshCache(true);
    if (t->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
    } else {
        KpkTransaction *frm = new KpkTransaction(t, KpkTransaction::Modal | KpkTransaction::CloseOnFinish, this);
        connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                this, SLOT(getUpdates()));
        frm->show();
    }
}

void KcmKpkUpdate::showExtendItem(const QModelIndex &index)
{
    const QSortFilterProxyModel *proxy;
    const KpkPackageModel *model;
    proxy = qobject_cast<const QSortFilterProxyModel*>(index.model());
    model = qobject_cast<const KpkPackageModel*>(proxy->sourceModel());
    QModelIndex origIndex = proxy->mapToSource(index);
    QSharedPointer<PackageKit::Package> package = model->package(origIndex);
    // check to see if the backend support
    if (package && (m_roles & Enum::RoleGetUpdateDetail)) {
        if (m_delegate->isExtended(index)) {
            m_delegate->contractItem(index);
        } else {
            m_delegate->extendItem(new KpkUpdateDetails(package), index);
        }
    }
}

void KcmKpkUpdate::on_historyPB_clicked()
{
    QPointer<KpkHistory> frm = new KpkHistory(this);
    frm->exec();
    delete frm;
}

void KcmKpkUpdate::contractAll()
{
    // This is a HACK so that the extenders don't stay visible when
    // the user sorts the view
    m_delegate->contractAll();
}

void KcmKpkUpdate::errorCode(PackageKit::Enum::Error error, const QString &details)
{
    KMessageBox::detailedSorry(this,
                               KpkStrings::errorMessage(error),
                               details,
                               KpkStrings::error(error),
                               KMessageBox::Notify);
}

#include "KcmKpkUpdate.moc"
