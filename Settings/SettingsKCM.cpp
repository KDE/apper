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

#include "SettingsKCM.h"

#include "KpkModelOrigin.h"

#include <KpkEnum.h>
#include <KpkTransactionBar.h>
#include <version.h>

#include <KConfig>
#include <KLocale>
#include <KMessageBox>
#include <KGenericFactory>
#include <KAboutData>

#include <KDebug>

using namespace PackageKit;

K_PLUGIN_FACTORY(KPackageKitFactory, registerPlugin<SettingsKCM>();)
K_EXPORT_PLUGIN(KPackageKitFactory("kcm_kpk_settings"))

SettingsKCM::SettingsKCM(QWidget *parent, const QVariantList &args)
    : KCModule(KPackageKitFactory::componentData(), parent, args)
{
    KAboutData *aboutData;
    aboutData = new KAboutData("kpackagekit",
                               "kpackagekit",
                               ki18n("KPackageKit settings"),
                               KPK_VERSION,
                               ki18n("KPackageKit settings"),
                               KAboutData::License_GPL,
                               ki18n("(C) 2008-2010 Daniel Nicoletti"));
    setAboutData(aboutData);
    KGlobal::locale()->insertCatalog("kpackagekit");
    setupUi(this);

    transactionBar->setBehaviors(KpkTransactionBar::AutoHide);
    QString locale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
    Client::instance()->setHints("locale=" + locale);
    m_roles = Client::instance()->actions();

    if (!(m_roles & Enum::RoleRefreshCache)) {
        intervalL->setEnabled(false);
        intervalCB->setEnabled(false);
    }

    m_originModel = new KpkModelOrigin(this);
    originLW->setModel(m_originModel);
    if (m_roles & Enum::RoleGetRepoList) {
        m_trasaction = Client::instance()->getRepoList(PackageKit::Enum::FilterNotDevelopment);
        connect(m_trasaction, SIGNAL(repoDetail(const QString &, const QString &, bool)),
                m_originModel, SLOT(addOriginItem(const QString &, const QString &, bool)));
        connect(m_trasaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                m_originModel, SLOT(finished()));
        connect(m_originModel, SIGNAL(stateChanged()), this, SLOT(checkChanges()));
    }
    else {
        originGB->setEnabled(false);
    }

    intervalCB->addItem(i18nc("Hourly refresh the package cache", "Hourly"),  KpkEnum::Hourly);
    intervalCB->addItem(i18nc("Daily refresh the package cache", "Daily"),   KpkEnum::Daily);
    intervalCB->addItem(i18nc("Weekly refresh the package cache", "Weekly"),  KpkEnum::Weekly);
    intervalCB->addItem(i18nc("Monthly refresh the package cache", "Monthly"), KpkEnum::Monthly);
    intervalCB->addItem(i18nc("Never refresh package cache", "Never"), KpkEnum::Never);

    autoCB->addItem(i18n("Security only"), KpkEnum::Security);
    autoCB->addItem(i18n("All updates"),   KpkEnum::All);
    autoCB->addItem(i18nc("None updates will be automatically installed", "None"),          KpkEnum::None);

    connect(autoConfirmCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(notifyUpdatesCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(intervalCB, SIGNAL(currentIndexChanged(int)), this, SLOT(checkChanges()));
    connect(autoCB, SIGNAL(currentIndexChanged(int)), this, SLOT(checkChanges()));
}

// TODO update the repo list connecting to repo changed signal
void SettingsKCM::on_showOriginsCB_stateChanged(int state)
{
    m_trasaction = Client::instance()->getRepoList(
        state == Qt::Checked ? Enum::NoFilter : Enum::FilterNotDevelopment);
    connect(m_trasaction, SIGNAL(repoDetail(const QString &, const QString &, bool)),
            m_originModel, SLOT(addOriginItem(const QString &, const QString &, bool)));
    connect(m_trasaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            m_originModel, SLOT(finished()));
    connect(m_originModel, SIGNAL(stateChanged()), this, SLOT(checkChanges()));
    transactionBar->addTransaction(m_trasaction);
}

void SettingsKCM::checkChanges()
{
    KConfig config("KPackageKit");

    KConfigGroup requirementsDialog(&config, "requirementsDialog");
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
        autoConfirmCB->isChecked() != !requirementsDialog.readEntry("autoConfirm", false)) {
        emit(changed(true));
    } else {
        emit(changed(false));
    }

    // Check if interval update is never
    bool enabled = intervalCB->itemData(intervalCB->currentIndex()).toUInt() != KpkEnum::Never;
    autoInsL->setEnabled(enabled);
    notifyUpdatesCB->setEnabled(enabled);
    autoCB->setEnabled(enabled);
}

void SettingsKCM::load()
{
    KConfig config("KPackageKit");

    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    autoConfirmCB->setChecked(!requirementsDialog.readEntry("autoConfirm", false));

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

    if (m_roles & Enum::RoleGetRepoList) {
        m_trasaction = Client::instance()->getRepoList(PackageKit::Enum::FilterNotDevelopment);
        connect(m_trasaction, SIGNAL(repoDetail(const QString &, const QString &, bool)),
                m_originModel, SLOT(addOriginItem(const QString &, const QString &, bool)));
        connect(m_trasaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                m_originModel, SLOT(finished()));
        connect(m_originModel, SIGNAL(stateChanged()), this, SLOT(checkChanges()));
        transactionBar->addTransaction(m_trasaction);
    }
}

void SettingsKCM::save()
{
    KConfig config("KPackageKit");

    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    requirementsDialog.writeEntry("autoConfirm", !autoConfirmCB->isChecked());

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

void SettingsKCM::defaults()
{
    autoConfirmCB->setChecked(true);
    notifyUpdatesCB->setCheckState(Qt::Checked);
    intervalCB->setCurrentIndex(intervalCB->findData(KpkEnum::TimeIntervalDefault));
    autoCB->setCurrentIndex(autoCB->findData(KpkEnum::AutoUpdateDefault) );
    m_originModel->clearChanges();
    emit checkChanges();
}

#include "SettingsKCM.moc"