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

#include "UpdateKCM.h"

#include "KpkUpdateDetails.h"
#include "KpkDistroUpgrade.h"
#include "KpkMacros.h"
#include "KpkCheckableHeader.h"

#include <version.h>

#include <KMenu>
#include <KGenericFactory>
#include <KPixmapSequence>
#include <KAboutData>

#include <ApplicationsDelegate.h>
#include <KpkStrings.h>
#include <KpkIcons.h>
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

K_PLUGIN_FACTORY(KPackageKitFactory, registerPlugin<UpdateKCM>();)
K_EXPORT_PLUGIN(KPackageKitFactory("kcm_kpk_update"))

UpdateKCM::UpdateKCM(QWidget *&parent, const QVariantList &args)
    : KCModule(KPackageKitFactory::componentData(), parent, args),
      m_updatesT(0)
{
    KAboutData *aboutData;
    aboutData = new KAboutData("kpackagekit",
                               "kpackagekit",
                               ki18n("Update Software"),
                               KPK_VERSION,
                               ki18n("Review and Update Software"),
                               KAboutData::License_GPL,
                               ki18n("(C) 2008-2010 Daniel Nicoletti"));
    setAboutData(aboutData);
    setButtons(Apply);
    KGlobal::locale()->insertCatalog("kpackagekit");

    m_selected = !args.isEmpty();
    setupUi(this);

    QString locale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
    Client::instance()->setHints("locale=" + locale);

    //initialize the model, delegate, client and  connect it's signals
    m_header = new KpkCheckableHeader(Qt::Horizontal, this);
    m_header->setCheckBoxVisible(false);
    packageView->setHeader(m_header);
    packageView->setHeaderHidden(true);

    m_updatesModel = new KpkPackageModel(this, packageView);
    m_updatesModel->setCheckable(true);
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(m_updatesModel);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setSortRole(KpkPackageModel::SortRole);
    packageView->setModel(proxyModel);

    m_delegate = new ApplicationsDelegate(packageView);
    m_delegate->setCheckable(true);
    packageView->setItemDelegate(m_delegate);
    packageView->sortByColumn(0, Qt::AscendingOrder);
    connect(m_header, SIGNAL(toggled(bool)),
            m_updatesModel, SLOT(setAllChecked(bool)));
    connect(m_updatesModel, SIGNAL(changed(bool)),
            this, SLOT(checkEnableUpdateButton()));

    // This must be set AFTER the model is set, otherwise it doesn't work
    m_header->setResizeMode(0, QHeaderView::ResizeToContents);
    m_header->setResizeMode(1, QHeaderView::ResizeToContents);
    m_header->setStretchLastSection(true);

    // Setup the busy cursor
    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(packageView->viewport());

    // Create a new client
    m_client = Client::instance();

    // check to see what roles the backend has
    m_roles = m_client->actions();

    // hide distro Upgrade container and line
    distroUpgradesSA->hide();
    line->hide();

    KConfig config("KPackageKit");
    KConfigGroup viewGroup(&config, "ViewGroup");
    m_showPackageVersion = new QAction(i18n("Show Versions"), this);
    m_showPackageVersion->setCheckable(true);
    connect(m_showPackageVersion, SIGNAL(toggled(bool)),
            this, SLOT(showVersions(bool)));
    m_showPackageVersion->setChecked(viewGroup.readEntry("ShowVersions", false));
    showVersions(m_showPackageVersion->isChecked());

    // Arch
    m_showPackageArch = new QAction(i18n("Show Architectures"), this);
    m_showPackageArch->setCheckable(true);
    connect(m_showPackageArch, SIGNAL(toggled(bool)),
            this, SLOT(showArchs(bool)));
    m_showPackageArch->setChecked(viewGroup.readEntry("ShowArchs", false));
    showArchs(m_showPackageArch->isChecked());

    checkUpdatesPB->setIcon(KIcon("view-refresh"));
    connect(checkUpdatesPB, SIGNAL(clicked(bool)),
            this, SLOT(refreshCache()));
}

UpdateKCM::~UpdateKCM()
{
    KConfig config("KPackageKit");
    KConfigGroup viewGroup(&config, "ViewGroup");
    viewGroup.writeEntry("ShowVersions", m_showPackageVersion->isChecked());
    viewGroup.writeEntry("ShowArchs", m_showPackageArch->isChecked());
}

void UpdateKCM::showVersions(bool enabled)
{
    packageView->header()->setSectionHidden(1, !enabled);
}

void UpdateKCM::showArchs(bool enabled)
{
    packageView->header()->setSectionHidden(2, !enabled);
}

void UpdateKCM::on_packageView_clicked(const QModelIndex &index)
{
    QString    pkgId   = index.data(KpkPackageModel::IdRole).toString();
    Enum::Info pkgInfo = static_cast<Enum::Info>(index.data(KpkPackageModel::InfoRole).toUInt());
    updateDetails->setPackage(pkgId, pkgInfo);
}

//TODO: We should add some kind of configuration to let users show unstable distributions
//That way, by default, users only see stable ones.
void UpdateKCM::distroUpgrade(PackageKit::Enum::DistroUpgrade type, const QString &name, const QString &description)
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

void UpdateKCM::checkEnableUpdateButton()
{
    emit changed(m_updatesModel->hasChanges());
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
    m_header->setCheckBoxVisible(m_updatesModel->rowCount() != 0);
    packageView->setHeaderHidden(m_updatesModel->rowCount() == 0);
}

void UpdateKCM::load()
{
    // set focus on the updates view
    packageView->setFocus(Qt::OtherFocusReason);
    // If the model already has some packages
    // let's just clear the selection
    if (m_updatesModel->rowCount()) {
        m_updatesModel->setAllChecked(false);
    } else {
        getUpdates();
    }
}

void UpdateKCM::getUpdatesFinished(Enum::Exit status)
{
    Q_UNUSED(status)
    m_updatesT = 0;
    m_updatesModel->clearSelectedNotPresent();
    checkEnableUpdateButton();
}

void UpdateKCM::save()
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

void UpdateKCM::transactionFinished(KpkTransaction::ExitStatus status)
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

void UpdateKCM::updatePackages()
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

void UpdateKCM::getUpdates()
{
    if (m_updatesT) {
        // There is a getUpdates running ignore this call
        return;
    }

    // clears the model
    packageView->setHeaderHidden(true);
    m_updatesModel->clear();
    updateDetails->hide();
    m_updatesT = new Transaction(QString());
    if (m_selected) {
        connect(m_updatesT, SIGNAL(package(const QSharedPointer<PackageKit::Package> &)),
                m_updatesModel, SLOT(addSelectedPackage(const QSharedPointer<PackageKit::Package> &)));
    } else {
        connect(m_updatesT, SIGNAL(package(const QSharedPointer<PackageKit::Package> &)),
                m_updatesModel, SLOT(addPackage(const QSharedPointer<PackageKit::Package> &)));
    }
    connect(m_updatesT, SIGNAL(errorCode(PackageKit::Enum::Error, const QString &)),
            this, SLOT(errorCode(PackageKit::Enum::Error, const QString &)));
    connect(m_updatesT, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            this, SLOT(getUpdatesFinished(PackageKit::Enum::Exit)));
    connect(m_updatesT, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            m_busySeq, SLOT(stop()));
    connect(m_updatesT, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            m_updatesModel, SLOT(finished()));
    // get all updates
    m_updatesT->getUpdates();

    if (m_updatesT->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(m_updatesT->error()));
    } else {
        m_busySeq->start();
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
    Transaction *t = new Transaction(QString());
    connect(t, SIGNAL(distroUpgrade(PackageKit::Enum::DistroUpgrade, const QString &, const QString &)),
            this, SLOT(distroUpgrade(PackageKit::Enum::DistroUpgrade, const QString &, const QString &)));
    t->getDistroUpgrades();
}

void UpdateKCM::refreshCache()
{
    SET_PROXY
    Transaction *t = new Transaction(QString());
    connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            this, SLOT(getUpdates()));
    t->refreshCache(true);

    KpkTransaction *frm = new KpkTransaction(t,
                                             KpkTransaction::Modal | KpkTransaction::CloseOnFinish,
                                             this);
    if (t->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
        delete frm;
    } else {
        frm->show();
    }
}

void UpdateKCM::on_packageView_customContextMenuRequested(const QPoint &pos)
{
    KMenu *menu = new KMenu(this);
    menu->addAction(m_showPackageVersion);
    menu->addAction(m_showPackageArch);
    QAction *action;
    action = menu->addAction(i18n("Check for new Updates"));
    action->setIcon(KIcon("view-refresh"));
    connect(action, SIGNAL(triggered(bool)),
            this, SLOT(refreshCache()));
    menu->exec(packageView->mapToGlobal(pos));
    delete menu;
}

void UpdateKCM::errorCode(PackageKit::Enum::Error error, const QString &details)
{
    KMessageBox::detailedSorry(this,
                               KpkStrings::errorMessage(error),
                               details,
                               KpkStrings::error(error),
                               KMessageBox::Notify);
}

#include "UpdateKCM.moc"
