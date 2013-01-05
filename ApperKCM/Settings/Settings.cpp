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

#include "ui_Settings.h"

#include "Settings.h"

#include "OriginModel.h"

#include <Enum.h>

#include <QTimer>
#include <QSortFilterProxyModel>

#include <KMessageBox>
#include <KPixmapSequence>
#include <KToolInvocation>

#include <Solid/Device>
#include <Solid/Battery>

#include <config.h>

#include <KDebug>

using namespace PackageKit;

Settings::Settings(Transaction::Roles roles, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Settings),
    m_roles(roles)
{
    ui->setupUi(this);

    if (!(m_roles & Transaction::RoleRefreshCache)) {
        ui->intervalL->setEnabled(false);
        ui->intervalCB->setEnabled(false);
    }

    if (!(m_roles & Transaction::RoleGetDistroUpgrades)) {
        ui->distroIntervalCB->setEnabled(false);
    }

    m_originModel = new OriginModel(this);
    QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
    proxy->setDynamicSortFilter(true);
    proxy->setSourceModel(m_originModel);
    ui->originTV->setModel(proxy);
    ui->originTV->header()->setDefaultAlignment(Qt::AlignCenter);
    // This is needed to keep the oring right
    ui->originTV->header()->setSortIndicator(0, Qt::AscendingOrder);
    proxy->sort(0);
    if (m_roles & Transaction::RoleGetRepoList) {
        // The data will be loaded when Load is called
        connect(m_originModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(checkChanges()));
    } else {
        // Disables the group box
        ui->originTV->setEnabled(false);
        ui->showOriginsCB->setEnabled(false);
    }

    ui->distroIntervalCB->addItem(i18nc("Inform about distribution upgrades",
                                        "Never"),
                                  Enum::DistroNever);
    ui->distroIntervalCB->addItem(i18nc("Inform about distribution upgrades",
                                        "Only stable"),
                                  Enum::DistroStable);
    // Ignore unstable distros upgrades for now
#ifndef false
    ui->distroIntervalCB->addItem(i18nc("Inform about distribution upgrades",
                                        "Stable and development"),
                                  Enum::DistroDevelopment);
#endif

    ui->intervalCB->addItem(i18nc("Hourly refresh the package cache", "Hourly"), Enum::Hourly);
    ui->intervalCB->addItem(i18nc("Daily refresh the package cache", "Daily"), Enum::Daily);
    ui->intervalCB->addItem(i18nc("Weekly refresh the package cache", "Weekly"), Enum::Weekly);
    ui->intervalCB->addItem(i18nc("Monthly refresh the package cache", "Monthly"), Enum::Monthly);
    ui->intervalCB->addItem(i18nc("Never refresh package cache", "Never"), Enum::Never);

    ui->autoCB->addItem(i18nc("No updates will be automatically installed", "None"), Enum::None);
    ui->autoCB->addItem(i18n("Download only"), Enum::DownloadOnly);
    ui->autoCB->addItem(i18n("Security only"), Enum::Security);
    ui->autoCB->addItem(i18n("All updates"), Enum::All);

    connect(ui->autoConfirmCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(ui->appLauncherCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(ui->distroIntervalCB, SIGNAL(currentIndexChanged(int)), this, SLOT(checkChanges()));
    connect(ui->intervalCB, SIGNAL(currentIndexChanged(int)), this, SLOT(checkChanges()));
    connect(ui->checkUpdatesBatteryCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(ui->checkUpdatesMobileCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(ui->autoCB, SIGNAL(currentIndexChanged(int)), this, SLOT(checkChanges()));
    connect(ui->installUpdatesBatteryCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(ui->installUpdatesMobileCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));

    // Setup the busy cursor
    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(ui->originTV->viewport());

#ifndef EDIT_ORIGNS_DESKTOP_NAME
    ui->editOriginsPB->hide();
#endif //EDIT_ORIGNS_DESKTOP_NAME
}

Settings::~Settings()
{
    delete ui;
}

void Settings::on_editOriginsPB_clicked()
{
#ifdef EDIT_ORIGNS_DESKTOP_NAME
    KToolInvocation::startServiceByDesktopName(EDIT_ORIGNS_DESKTOP_NAME);
#endif //EDIT_ORIGNS_DESKTOP_NAME
}

// TODO update the repo list connecting to repo changed signal
void Settings::on_showOriginsCB_stateChanged(int state)
{
    Transaction *transaction = new Transaction(this);
    connect(transaction, SIGNAL(repoDetail(QString,QString,bool)),
            m_originModel, SLOT(addOriginItem(QString,QString,bool)));
    connect(transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            m_originModel, SLOT(finished()));
    connect(transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            m_busySeq, SLOT(stop()));
    connect(transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(checkChanges()));

    if (state == Qt::Checked) {
        transaction->getRepoList(Transaction::FilterNone);
    } else {
        transaction->getRepoList(Transaction::FilterNotDevel);
    }

    if (!transaction->error()) {
        m_busySeq->start();
    }

    KConfig config("apper");
    KConfigGroup originsDialog(&config, "originsDialog");
    bool showDevel = originsDialog.readEntry("showDevel", false);
    if (showDevel != ui->showOriginsCB->isChecked()) {
        originsDialog.writeEntry("showDevel", ui->showOriginsCB->isChecked());
    }
}

bool Settings::hasChanges() const
{
    KConfig config("apper");

    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    KConfigGroup transaction(&config, "Transaction");
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    if (ui->distroIntervalCB->itemData(ui->distroIntervalCB->currentIndex()).toUInt() !=
            static_cast<uint>(checkUpdateGroup.readEntry(CFG_DISTRO_UPGRADE, Enum::DistroUpgradeDefault))
        ||
            ui->intervalCB->itemData(ui->intervalCB->currentIndex()).toUInt() !=
            static_cast<uint>(checkUpdateGroup.readEntry(CFG_INTERVAL, Enum::TimeIntervalDefault))
        ||
        ui->checkUpdatesBatteryCB->isChecked() != checkUpdateGroup.readEntry(CFG_CHECK_UP_BATTERY, DEFAULT_CHECK_UP_BATTERY)
        ||
        ui->checkUpdatesMobileCB->isChecked() != checkUpdateGroup.readEntry(CFG_CHECK_UP_MOBILE, DEFAULT_CHECK_UP_MOBILE)
        ||
        ui->autoCB->itemData(ui->autoCB->currentIndex()).toUInt() !=
        static_cast<uint>(checkUpdateGroup.readEntry(CFG_AUTO_UP, Enum::AutoUpdateDefault))
        ||
        ui->installUpdatesBatteryCB->isChecked()  != checkUpdateGroup.readEntry(CFG_INSTALL_UP_BATTERY, DEFAULT_INSTALL_UP_BATTERY)
        ||
        ui->installUpdatesMobileCB->isChecked() != checkUpdateGroup.readEntry(CFG_INSTALL_UP_MOBILE, DEFAULT_INSTALL_UP_MOBILE)
        ||
        ((m_roles & Transaction::RoleGetRepoList) ? m_originModel->changed() : false)
        ||
        ui->autoConfirmCB->isChecked() != !requirementsDialog.readEntry("autoConfirm", false)
        ||
        ui->appLauncherCB->isChecked() != transaction.readEntry("ShowApplicationLauncher", true)) {
        return true;
    }
    return false;
}

void Settings::checkChanges()
{
    emit changed(hasChanges());

    // Check if interval update is never
    bool enabled = ui->intervalCB->itemData(ui->intervalCB->currentIndex()).toUInt() != Enum::Never;
    ui->checkUpdatesBatteryCB->setEnabled(enabled);
    ui->checkUpdatesMobileCB->setEnabled(enabled);

    ui->autoInsL->setEnabled(enabled);
    ui->autoCB->setEnabled(enabled);
    if (enabled) {
        enabled = ui->autoCB->itemData(ui->autoCB->currentIndex()).toUInt() != Enum::None;
    }
    ui->installUpdatesMobileCB->setEnabled(enabled);
    ui->installUpdatesBatteryCB->setEnabled(enabled);
}

void Settings::load()
{
    KConfig config("apper");

    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    ui->autoConfirmCB->setChecked(!requirementsDialog.readEntry("autoConfirm", false));

    KConfigGroup transaction(&config, "Transaction");
    ui->appLauncherCB->setChecked(transaction.readEntry("ShowApplicationLauncher", true));

    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    uint distroUpgrade = checkUpdateGroup.readEntry(CFG_DISTRO_UPGRADE, Enum::DistroUpgradeDefault);
    int ret = ui->distroIntervalCB->findData(distroUpgrade);
    if (ret == -1) {
        ui->distroIntervalCB->setCurrentIndex(ui->distroIntervalCB->findData(Enum::DistroUpgradeDefault));
    } else {
        ui->distroIntervalCB->setCurrentIndex(ret);
    }

    uint interval = checkUpdateGroup.readEntry(CFG_INTERVAL, Enum::TimeIntervalDefault);
    ret = ui->intervalCB->findData(interval);
    if (ret == -1) {
        // this is if someone change the file by hand...
        ui->intervalCB->addItem(KGlobal::locale()->prettyFormatDuration(interval * 1000), interval);
        ui->intervalCB->setCurrentIndex(ui->intervalCB->count() - 1);
    } else {
        ui->intervalCB->setCurrentIndex(ret);
    }
    ui->checkUpdatesBatteryCB->setChecked(checkUpdateGroup.readEntry(CFG_CHECK_UP_BATTERY, DEFAULT_CHECK_UP_BATTERY));
    ui->checkUpdatesMobileCB->setChecked(checkUpdateGroup.readEntry(CFG_CHECK_UP_MOBILE, DEFAULT_CHECK_UP_MOBILE));

    uint autoUpdate = checkUpdateGroup.readEntry(CFG_AUTO_UP, Enum::AutoUpdateDefault);
    ret = ui->autoCB->findData(autoUpdate);
    if (ret == -1) {
        // this is if someone change the file by hand...
        ui->autoCB->setCurrentIndex(ui->autoCB->findData(Enum::AutoUpdateDefault));
    } else {
        ui->autoCB->setCurrentIndex(ret);
    }
    ui->installUpdatesBatteryCB->setChecked(checkUpdateGroup.readEntry(CFG_INSTALL_UP_BATTERY, DEFAULT_INSTALL_UP_BATTERY));
    ui->installUpdatesMobileCB->setChecked(checkUpdateGroup.readEntry(CFG_INSTALL_UP_MOBILE, DEFAULT_INSTALL_UP_MOBILE));

    // Load origns list
    if (m_roles & Transaction::RoleGetRepoList) {
        KConfigGroup originsDialog(&config, "originsDialog");
        bool showDevel = originsDialog.readEntry("showDevel", false);
        ui->showOriginsCB->setChecked(showDevel);
        on_showOriginsCB_stateChanged(ui->showOriginsCB->checkState());
        ui->originTV->setEnabled(true);
    } else {
        ui->originTV->setEnabled(false);
    }

    // hide battery options if we are on a desktop computer
    const QList<Solid::Device> listBattery = Solid::Device::listFromType(Solid::DeviceInterface::Battery, QString());
    bool notFound = true;
    foreach (const Solid::Device &device, listBattery) {
        const Solid::Battery *battery = device.as<Solid::Battery>();
        if (battery && battery->type() == Solid::Battery::PrimaryBattery) {
            notFound = false;
            break;
        }
    }

    if (notFound) {
        ui->checkUpdatesBatteryCB->hide();
        ui->installUpdatesBatteryCB->hide();
    }
}

void Settings::save()
{
    KConfig config("apper");

    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    requirementsDialog.writeEntry("autoConfirm", !ui->autoConfirmCB->isChecked());

    KConfigGroup transaction(&config, "Transaction");
    transaction.writeEntry("ShowApplicationLauncher", ui->appLauncherCB->isChecked());

    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    checkUpdateGroup.writeEntry("distroUpgrade", ui->distroIntervalCB->itemData(ui->distroIntervalCB->currentIndex()).toUInt());
    checkUpdateGroup.writeEntry("interval", ui->intervalCB->itemData(ui->intervalCB->currentIndex()).toUInt());
    checkUpdateGroup.writeEntry("checkUpdatesOnBattery", ui->checkUpdatesBatteryCB->isChecked());
    checkUpdateGroup.writeEntry("checkUpdatesOnMobile", ui->checkUpdatesMobileCB->isChecked());

    checkUpdateGroup.writeEntry("autoUpdate", ui->autoCB->itemData(ui->autoCB->currentIndex()).toUInt());
    checkUpdateGroup.writeEntry("installUpdatesOnBattery", ui->installUpdatesBatteryCB->isChecked());
    checkUpdateGroup.writeEntry("installUpdatesOnMobile", ui->installUpdatesMobileCB->isChecked());
    // check to see if the backend support this
    if (m_roles & Transaction::RoleGetRepoList) {
        on_showOriginsCB_stateChanged(ui->showOriginsCB->checkState());
        if (!m_originModel->save()) {
            KMessageBox::sorry(this,
                               i18n("You do not have the necessary privileges to perform this action."),
                               i18n("Failed to set origin data"));
        }
    }
}

void Settings::defaults()
{
    ui->autoConfirmCB->setChecked(true);
    ui->appLauncherCB->setChecked(true);
    ui->distroIntervalCB->setCurrentIndex(ui->distroIntervalCB->findData(Enum::DistroUpgradeDefault));
    ui->intervalCB->setCurrentIndex(ui->intervalCB->findData(Enum::TimeIntervalDefault));
    ui->autoCB->setCurrentIndex(ui->autoCB->findData(Enum::AutoUpdateDefault) );
    m_originModel->clearChanges();
    checkChanges();
}

void Settings::showGeneralSettings()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void Settings::showRepoSettings()
{
    ui->stackedWidget->setCurrentIndex(1);
}

#include "Settings.moc"
