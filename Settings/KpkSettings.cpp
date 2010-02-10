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

#include "KpkSettings.h"
#include <KpkEnum.h>
#include "KpkModelOrigin.h"

#include <KDebug>
#include <KConfig>
#include <KLocale>
#include <KMessageBox>
#include <KpkTransactionBar.h>

using namespace PackageKit;

KpkSettings::KpkSettings(QWidget *parent)
  : QWidget(parent), m_originModel(0)
{
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

    connect(notifyUpdatesCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(intervalCB, SIGNAL(currentIndexChanged(int)), this, SLOT(checkChanges()));
    connect(autoCB, SIGNAL(currentIndexChanged(int)), this, SLOT(checkChanges()));
}

// TODO update the repo list connecting to repo changed signal
void KpkSettings::on_showOriginsCB_stateChanged(int state)
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

void KpkSettings::checkChanges()
{
    KConfig config("KPackageKit");
    KConfigGroup notifyGroup( &config, "Notify" );
    KConfigGroup checkUpdateGroup( &config, "CheckUpdate" );
    if (notifyUpdatesCB->checkState() !=
        (Qt::CheckState) notifyGroup.readEntry("notifyUpdates", (int) Qt::Checked)
        ||
        intervalCB->itemData(intervalCB->currentIndex()).toUInt() !=
        (uint) checkUpdateGroup.readEntry("interval", KpkEnum::TimeIntervalDefault)
        ||
        autoCB->itemData(autoCB->currentIndex()).toUInt() !=
        (uint) checkUpdateGroup.readEntry("autoUpdate", KpkEnum::AutoUpdateDefault)
        ||
        ((m_roles & Enum::RoleGetRepoList) ? m_originModel->changed() : false)) {
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

void KpkSettings::load()
{
    KConfig config("KPackageKit");
    KConfigGroup notifyGroup( &config, "Notify" );
    notifyUpdatesCB->setCheckState((Qt::CheckState) notifyGroup.readEntry("notifyUpdates",
        (int) Qt::Checked));

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
    if (ret == -1)
        // this is if someone change the file by hand...
        autoCB->setCurrentIndex( autoCB->findData(KpkEnum::AutoUpdateDefault) );
    else
        autoCB->setCurrentIndex(ret);

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

void KpkSettings::save()
{
    KConfig config("KPackageKit");
    KConfigGroup notifyGroup(&config, "Notify");
    // not used anymore
    notifyGroup.deleteEntry("notifyLongTasks");
    notifyGroup.writeEntry("notifyUpdates", (int) notifyUpdatesCB->checkState());
    KConfigGroup checkUpdateGroup( &config, "CheckUpdate");
    checkUpdateGroup.writeEntry("interval", intervalCB->itemData(intervalCB->currentIndex()).toUInt());
    checkUpdateGroup.writeEntry("autoUpdate", autoCB->itemData(autoCB->currentIndex()).toUInt());
    // check to see if the backend support this
    if (m_roles & Enum::RoleGetRepoList) {
        if ( !m_originModel->save() ) {
            KMessageBox::sorry(this,
                               i18n("You do not have the necessary privileges to perform this action."),
                               i18n("Failed to set origin data"));
            QTimer::singleShot(1, this, SLOT(checkChanges()));
        }
        on_showOriginsCB_stateChanged(showOriginsCB->checkState());
    }
}

void KpkSettings::defaults()
{
    notifyUpdatesCB->setCheckState(Qt::Checked);
    intervalCB->setCurrentIndex(intervalCB->findData(KpkEnum::TimeIntervalDefault));
    autoCB->setCurrentIndex(autoCB->findData(KpkEnum::AutoUpdateDefault) );
    m_originModel->clearChanges();
    emit checkChanges();
}

#include "KpkSettings.moc"
