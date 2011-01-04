/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
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

#include "Settings.h"

#include "OriginModel.h"

#include <KpkEnum.h>

#include <KMessageBox>
#include <KPixmapSequence>
#include <KToolInvocation>

#include <config.h>

#include <KDebug>

using namespace PackageKit;

Settings::Settings(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    m_roles = Client::instance()->actions();

    if (!(m_roles & Enum::RoleRefreshCache)) {
        intervalL->setEnabled(false);
        intervalCB->setEnabled(false);
    }

    m_originModel = new OriginModel(this);
    originTV->setModel(m_originModel);
    originTV->header()->setDefaultAlignment(Qt::AlignCenter);
    if (m_roles & Enum::RoleGetRepoList) {
        // The data will be loaded when Load is called
        connect(m_originModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
                this, SLOT(checkChanges()));
    } else {
        // Disables the group box
        originTV->setEnabled(false);
        showOriginsCB->setEnabled(false);
    }

    intervalCB->addItem(i18nc("Hourly refresh the package cache", "Hourly"),  KpkEnum::Hourly);
    intervalCB->addItem(i18nc("Daily refresh the package cache", "Daily"),   KpkEnum::Daily);
    intervalCB->addItem(i18nc("Weekly refresh the package cache", "Weekly"),  KpkEnum::Weekly);
    intervalCB->addItem(i18nc("Monthly refresh the package cache", "Monthly"), KpkEnum::Monthly);
    intervalCB->addItem(i18nc("Never refresh package cache", "Never"), KpkEnum::Never);

    autoCB->addItem(i18n("Security only"), KpkEnum::Security);
    autoCB->addItem(i18n("All updates"),   KpkEnum::All);
    autoCB->addItem(i18nc("No updates will be automatically installed", "None"),          KpkEnum::None);

    connect(autoConfirmCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(appLauncherCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(notifyUpdatesCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(intervalCB, SIGNAL(currentIndexChanged(int)), this, SLOT(checkChanges()));
    connect(autoCB, SIGNAL(currentIndexChanged(int)), this, SLOT(checkChanges()));

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
    Transaction *transaction = new Transaction(QString());
    connect(transaction, SIGNAL(repoDetail(const QString &, const QString &, bool)),
            m_originModel, SLOT(addOriginItem(const QString &, const QString &, bool)));
    connect(transaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            m_originModel, SLOT(finished()));
    connect(transaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            m_busySeq, SLOT(stop()));

    if (state == Qt::Checked) {
        transaction->getRepoList(Enum::NoFilter);
    } else {
        transaction->getRepoList(Enum::FilterNotDevelopment);
    }

    if (!transaction->error()) {
        m_busySeq->start();
    }
}

bool Settings::hasChanges() const
{
    KConfig config("KPackageKit");

    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    KConfigGroup transaction(&config, "Transaction");
    KConfigGroup notifyGroup(&config, "Notify");
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    if (notifyUpdatesCB->checkState() !=
        static_cast<Qt::CheckState>(notifyGroup.readEntry("notifyUpdates", (int) Qt::Checked))
        ||
        intervalCB->itemData(intervalCB->currentIndex()).toUInt() !=
        static_cast<uint>(checkUpdateGroup.readEntry("interval", KpkEnum::TimeIntervalDefault))
        ||
        autoCB->itemData(autoCB->currentIndex()).toUInt() !=
        static_cast<uint>(checkUpdateGroup.readEntry("autoUpdate", KpkEnum::AutoUpdateDefault))
        ||
        ((m_roles & Enum::RoleGetRepoList) ? m_originModel->changed() : false)
        ||
        autoConfirmCB->isChecked() != !requirementsDialog.readEntry("autoConfirm", false)
        ||
        appLauncherCB->isChecked() != transaction.readEntry("ShowApplicationLauncher", true)) {
        return true;
    } else {
        return false;
    }
}

void Settings::checkChanges()
{
    emit changed(hasChanges());

    // Check if interval update is never
    bool enabled = intervalCB->itemData(intervalCB->currentIndex()).toUInt() != KpkEnum::Never;
    autoInsL->setEnabled(enabled);
    notifyUpdatesCB->setEnabled(enabled);
    autoCB->setEnabled(enabled);
}

void Settings::load()
{
    KConfig config("KPackageKit");

    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    autoConfirmCB->setChecked(!requirementsDialog.readEntry("autoConfirm", false));

    KConfigGroup transaction(&config, "Transaction");
    appLauncherCB->setChecked(transaction.readEntry("ShowApplicationLauncher", true));

    KConfigGroup notifyGroup(&config, "Notify");
    notifyUpdatesCB->setCheckState(static_cast<Qt::CheckState>(notifyGroup.readEntry("notifyUpdates",
        static_cast<int>(Qt::Checked))));

    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    uint interval = checkUpdateGroup.readEntry("interval", KpkEnum::TimeIntervalDefault);
    int ret = intervalCB->findData(interval);
    if (ret == -1) {
        // this is if someone change the file by hand...
        intervalCB->addItem(KGlobal::locale()->prettyFormatDuration(interval * 1000), interval);
        intervalCB->setCurrentIndex(intervalCB->count() - 1);
    } else {
        intervalCB->setCurrentIndex(ret);
    }

    uint autoUpdate = checkUpdateGroup.readEntry("autoUpdate", KpkEnum::AutoUpdateDefault);
    ret = autoCB->findData(autoUpdate);
    if (ret == -1) {
        // this is if someone change the file by hand...
        autoCB->setCurrentIndex( autoCB->findData(KpkEnum::AutoUpdateDefault) );
    } else {
        autoCB->setCurrentIndex(ret);
    }

    // Load origns list
    if (m_roles & Enum::RoleGetRepoList) {
        on_showOriginsCB_stateChanged(Qt::Unchecked);
    }
}

void Settings::save()
{
    KConfig config("KPackageKit");

    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    requirementsDialog.writeEntry("autoConfirm", !autoConfirmCB->isChecked());

    KConfigGroup transaction(&config, "Transaction");
    transaction.writeEntry("ShowApplicationLauncher", appLauncherCB->isChecked());

    KConfigGroup notifyGroup(&config, "Notify");
    // not used anymore
    notifyGroup.deleteEntry("notifyLongTasks");
    notifyGroup.writeEntry("notifyUpdates", static_cast<int>(notifyUpdatesCB->checkState()));
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    checkUpdateGroup.writeEntry("interval", intervalCB->itemData(intervalCB->currentIndex()).toUInt());
    checkUpdateGroup.writeEntry("autoUpdate", autoCB->itemData(autoCB->currentIndex()).toUInt());
    // check to see if the backend support this
    if (m_roles & Enum::RoleGetRepoList) {
        if (!m_originModel->save()) {
            KMessageBox::sorry(this,
                               i18n("You do not have the necessary privileges to perform this action."),
                               i18n("Failed to set origin data"));
            QTimer::singleShot(1, this, SLOT(checkChanges()));
        }
        on_showOriginsCB_stateChanged(showOriginsCB->checkState());
    }
}

void Settings::defaults()
{
    autoConfirmCB->setChecked(true);
    appLauncherCB->setChecked(true);
    notifyUpdatesCB->setCheckState(Qt::Checked);
    intervalCB->setCurrentIndex(intervalCB->findData(KpkEnum::TimeIntervalDefault));
    autoCB->setCurrentIndex(autoCB->findData(KpkEnum::AutoUpdateDefault) );
    m_originModel->clearChanges();
    checkChanges();
}

#include "Settings.moc"
