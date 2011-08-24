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

#include "Settings.h"

#include "OriginModel.h"

#include <Enum.h>

#include <QTimer>

#include <KMessageBox>
#include <KPixmapSequence>
#include <KToolInvocation>

#include <Solid/Device>

#include <config.h>

#include <KDebug>

using namespace PackageKit;

Settings::Settings(Transaction::Roles roles, QWidget *parent) :
    QWidget(parent),
    m_roles(roles)
{
    setupUi(this);

    if (!(m_roles & Transaction::RoleRefreshCache)) {
        intervalL->setEnabled(false);
        intervalCB->setEnabled(false);
    }

    m_originModel = new OriginModel(this);
    originTV->setModel(m_originModel);
    originTV->header()->setDefaultAlignment(Qt::AlignCenter);
    if (m_roles & Transaction::RoleGetRepoList) {
        // The data will be loaded when Load is called
        connect(m_originModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(checkChanges()));
    } else {
        // Disables the group box
        originTV->setEnabled(false);
        showOriginsCB->setEnabled(false);
    }

    intervalCB->addItem(i18nc("Hourly refresh the package cache", "Hourly"),  Enum::Hourly);
    intervalCB->addItem(i18nc("Daily refresh the package cache", "Daily"),   Enum::Daily);
    intervalCB->addItem(i18nc("Weekly refresh the package cache", "Weekly"),  Enum::Weekly);
    intervalCB->addItem(i18nc("Monthly refresh the package cache", "Monthly"), Enum::Monthly);
    intervalCB->addItem(i18nc("Never refresh package cache", "Never"), Enum::Never);

    autoCB->addItem(i18n("Security only"), Enum::Security);
    autoCB->addItem(i18n("All updates"),   Enum::All);
    autoCB->addItem(i18nc("No updates will be automatically installed", "None"),          Enum::None);

    connect(autoConfirmCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(appLauncherCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(intervalCB, SIGNAL(currentIndexChanged(int)), this, SLOT(checkChanges()));
    connect(checkUpdatesBatteryCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(checkUpdatesMobileCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(autoCB, SIGNAL(currentIndexChanged(int)), this, SLOT(checkChanges()));
    connect(installUpdatesBatteryCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(installUpdatesMobileCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));

    // Setup the busy cursor
    m_busySeq = new KPixmapSequenceOverlayPainter(this);
    m_busySeq->setSequence(KPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySeq->setWidget(originTV->viewport());

#ifndef EDIT_ORIGNS_DESKTOP_NAME
    editOriginsPB->hide();
#endif //EDIT_ORIGNS_DESKTOP_NAME
}

Settings::~Settings()
{
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
    connect(transaction, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
            m_originModel, SLOT(finished()));
    connect(transaction, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
            m_busySeq, SLOT(stop()));
    connect(transaction, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
            this, SLOT(checkChanges()));

    if (state == Qt::Checked) {
        transaction->getRepoList(Transaction::FilterNone);
    } else {
        transaction->getRepoList(Transaction::FilterNotDevel);
    }

    if (!transaction->error()) {
        m_busySeq->start();
    }
}

bool Settings::hasChanges() const
{
    KConfig config("apper");

    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    KConfigGroup transaction(&config, "Transaction");
    KConfigGroup notifyGroup(&config, "Notify");
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    if (intervalCB->itemData(intervalCB->currentIndex()).toUInt() !=
        static_cast<uint>(checkUpdateGroup.readEntry("interval", Enum::TimeIntervalDefault))
        ||
        checkUpdatesBatteryCB->isChecked() != checkUpdateGroup.readEntry("checkUpdatesOnBattery", false)
        ||
        checkUpdatesMobileCB->isChecked() != checkUpdateGroup.readEntry("checkUpdatesOnMobile", false)
        ||    
        autoCB->itemData(autoCB->currentIndex()).toUInt() !=
        static_cast<uint>(checkUpdateGroup.readEntry("autoUpdate", Enum::AutoUpdateDefault))
        ||
        installUpdatesBatteryCB->isChecked()  != checkUpdateGroup.readEntry("installUpdatesOnBattery", false)
        ||
        installUpdatesMobileCB->isChecked() != checkUpdateGroup.readEntry("installUpdatesOnMobile", false)
        ||
        ((m_roles & Transaction::RoleGetRepoList) ? m_originModel->changed() : false)
        ||
        autoConfirmCB->isChecked() != !requirementsDialog.readEntry("autoConfirm", false)
        ||
        appLauncherCB->isChecked() != transaction.readEntry("ShowApplicationLauncher", true)) {
        return true;
    }
    return false;
}

void Settings::checkChanges()
{
    emit changed(hasChanges());

    // Check if interval update is never
    bool enabled = intervalCB->itemData(intervalCB->currentIndex()).toUInt() != Enum::Never;
    checkUpdatesBatteryCB->setEnabled(enabled);
    checkUpdatesMobileCB->setEnabled(enabled);

    autoInsL->setEnabled(enabled);
    autoCB->setEnabled(enabled);
    if (enabled) {
        enabled = autoCB->itemData(autoCB->currentIndex()).toUInt() != Enum::None;
    }
    installUpdatesMobileCB->setEnabled(enabled);
    installUpdatesBatteryCB->setEnabled(enabled);
}

void Settings::load()
{
    KConfig config("apper");

    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    autoConfirmCB->setChecked(!requirementsDialog.readEntry("autoConfirm", false));

    KConfigGroup transaction(&config, "Transaction");
    appLauncherCB->setChecked(transaction.readEntry("ShowApplicationLauncher", true));

    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    uint interval = checkUpdateGroup.readEntry("interval", Enum::TimeIntervalDefault);
    int ret = intervalCB->findData(interval);
    if (ret == -1) {
        // this is if someone change the file by hand...
        intervalCB->addItem(KGlobal::locale()->prettyFormatDuration(interval * 1000), interval);
        intervalCB->setCurrentIndex(intervalCB->count() - 1);
    } else {
        intervalCB->setCurrentIndex(ret);
    }
    checkUpdatesBatteryCB->setChecked(checkUpdateGroup.readEntry("checkUpdatesOnBattery", false));
    checkUpdatesMobileCB->setChecked(checkUpdateGroup.readEntry("checkUpdatesOnMobile", false));

    uint autoUpdate = checkUpdateGroup.readEntry("autoUpdate", Enum::AutoUpdateDefault);
    ret = autoCB->findData(autoUpdate);
    if (ret == -1) {
        // this is if someone change the file by hand...
        autoCB->setCurrentIndex( autoCB->findData(Enum::AutoUpdateDefault) );
    } else {
        autoCB->setCurrentIndex(ret);
    }
    installUpdatesBatteryCB->setChecked(checkUpdateGroup.readEntry("installUpdatesOnBattery", false));
    installUpdatesMobileCB->setChecked(checkUpdateGroup.readEntry("installUpdatesOnMobile", false));

    // Load origns list
    if (m_roles & Transaction::RoleGetRepoList) {
        on_showOriginsCB_stateChanged(Qt::Unchecked);
    }

    // hide battery options if we are on a desktop computer
    const QList<Solid::Device> listBattery = Solid::Device::listFromType(Solid::DeviceInterface::Battery, QString());
    if (listBattery.isEmpty()) {
        checkUpdatesBatteryCB->hide();
        installUpdatesBatteryCB->hide();
    }
}

void Settings::save()
{
    KConfig config("apper");

    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    requirementsDialog.writeEntry("autoConfirm", !autoConfirmCB->isChecked());

    KConfigGroup transaction(&config, "Transaction");
    transaction.writeEntry("ShowApplicationLauncher", appLauncherCB->isChecked());

    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    checkUpdateGroup.writeEntry("interval", intervalCB->itemData(intervalCB->currentIndex()).toUInt());
    checkUpdateGroup.writeEntry("checkUpdatesOnBattery", checkUpdatesBatteryCB->isChecked());
    checkUpdateGroup.writeEntry("checkUpdatesOnMobile", checkUpdatesMobileCB->isChecked());

    checkUpdateGroup.writeEntry("autoUpdate", autoCB->itemData(autoCB->currentIndex()).toUInt());
    checkUpdateGroup.writeEntry("installUpdatesOnBattery", installUpdatesBatteryCB->isChecked());
    checkUpdateGroup.writeEntry("installUpdatesOnMobile", installUpdatesMobileCB->isChecked());
    // check to see if the backend support this
    if (m_roles & Transaction::RoleGetRepoList) {
        if (!m_originModel->save()) {
            KMessageBox::sorry(this,
                               i18n("You do not have the necessary privileges to perform this action."),
                               i18n("Failed to set origin data"));
        }
        on_showOriginsCB_stateChanged(showOriginsCB->checkState());
    }
}

void Settings::defaults()
{
    autoConfirmCB->setChecked(true);
    appLauncherCB->setChecked(true);
    intervalCB->setCurrentIndex(intervalCB->findData(Enum::TimeIntervalDefault));
    autoCB->setCurrentIndex(autoCB->findData(Enum::AutoUpdateDefault) );
    m_originModel->clearChanges();
    checkChanges();
}

void Settings::changeCurrentPage(int page)
{
    stackedWidget->setCurrentIndex(page);
}

#include "Settings.moc"
