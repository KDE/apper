/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "KpkSettings.h"
#include "../libkpackagekit/KpkEnum.h"

#include <KDebug>
#include <KConfig>
#include <KLocale>
#include <KMessageBox>

using namespace PackageKit;

KpkSettings::KpkSettings( QWidget *parent ) :
QWidget( parent ), m_originModel(0)
{
    setupUi( this );

    Client::instance()->setLocale(KGlobal::locale()->language() + "." + KGlobal::locale()->encoding());
    m_actions = Client::instance()->getActions();

    if ( !m_actions.contains(Client::ActionRefreshCache) ) {
        intervalL->setEnabled(false);
        intervalCB->setEnabled(false);
    }

    m_originModel = new KpkModelOrigin(this);
    originLW->setModel(m_originModel);
    if ( m_actions.contains(Client::ActionGetRepoList) ) {
        m_trasaction = Client::instance()->getRepoList(PackageKit::Client::FilterNotDevelopment);
        connect(m_trasaction, SIGNAL(repoDetail(const QString &, const QString &, bool)),
                m_originModel, SLOT(addOriginItem(const QString &, const QString &, bool)));
        connect(m_trasaction, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                m_originModel, SLOT(finished()));
        connect(m_originModel, SIGNAL(stateChanged()), this, SLOT(checkChanges()));
    }
    else {
        originGB->setEnabled(false);
    }

    intervalCB->addItem(i18n("Hourly"),  KpkEnum::Hourly);
    intervalCB->addItem(i18n("Daily"),   KpkEnum::Daily);
    intervalCB->addItem(i18n("Weekly"),  KpkEnum::Weekly);
    intervalCB->addItem(i18n("Monthly"), KpkEnum::Monthly);
    intervalCB->addItem(i18n("Never"),   KpkEnum::Never);

    autoCB->addItem(i18n("Security Only"), KpkEnum::Security);
    autoCB->addItem(i18n("All Updates"),   KpkEnum::All);
    autoCB->addItem(i18n("None"),          KpkEnum::None);

    connect(notifyUpdatesCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(notifyLongTasksCB, SIGNAL(stateChanged(int)), this, SLOT(checkChanges()));
    connect(intervalCB, SIGNAL(currentIndexChanged(int)), this, SLOT(checkChanges()));
    connect(autoCB, SIGNAL(currentIndexChanged(int)), this, SLOT(checkChanges()));
}

void KpkSettings::on_showOriginsCB_stateChanged(int state)
{
    m_trasaction = Client::instance()->getRepoList(
        state == Qt::Checked ? Client::NoFilter : Client::FilterNotDevelopment );
    connect(m_trasaction, SIGNAL(repoDetail(const QString &, const QString &, bool)),
            m_originModel, SLOT(addOriginItem(const QString &, const QString &, bool)));
    connect(m_trasaction, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
            m_originModel, SLOT(finished()));
    connect(m_originModel, SIGNAL(stateChanged()), this, SLOT(checkChanges()));
}

void KpkSettings::checkChanges()
{
    KConfig config("KPackageKit");
    KConfigGroup notifyGroup( &config, "Notify" );
    KConfigGroup checkUpdateGroup( &config, "CheckUpdate" );
    if (notifyUpdatesCB->checkState() !=
        (Qt::CheckState) notifyGroup.readEntry("notifyUpdates", (int) Qt::Checked)
        ||
        notifyLongTasksCB->checkState() !=
        (Qt::CheckState) notifyGroup.readEntry("notifyLongTasks", (int) Qt::Checked)
        ||
        intervalCB->itemData( intervalCB->currentIndex() ).toUInt() !=
        (uint) checkUpdateGroup.readEntry("interval", KpkEnum::TimeIntervalDefault)
        ||
        autoCB->itemData( autoCB->currentIndex() ).toUInt() !=
        (uint) checkUpdateGroup.readEntry("autoUpdate", KpkEnum::AutoUpdateDefault)
        ||
        ( m_actions.contains(Client::ActionGetRepoList) ? m_originModel->changed() : false))
        emit(changed(true));
    else
        emit(changed(false));
}

void KpkSettings::load()
{
    KConfig config("KPackageKit");
    KConfigGroup notifyGroup( &config, "Notify" );
    notifyUpdatesCB->setCheckState((Qt::CheckState) notifyGroup.readEntry("notifyUpdates",
        (int) Qt::Checked));
    notifyLongTasksCB->setCheckState((Qt::CheckState) notifyGroup.readEntry("notifyLongTasks",
        (int) Qt::Checked));

    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    uint interval = checkUpdateGroup.readEntry("interval", KpkEnum::TimeIntervalDefault);
    int ret = intervalCB->findData(interval);
    if (ret == -1) {
        // this is if someone change the file by hand...
        intervalCB->addItem(KGlobal::locale()->formatDuration(interval * 1000), interval);
        intervalCB->setCurrentIndex(intervalCB->count() - 1);
    } else {
        intervalCB->setCurrentIndex(ret);
    }

    uint autoUpdate = checkUpdateGroup.readEntry("autoUpdate", KpkEnum::AutoUpdateDefault);
    ret = autoCB->findData(autoUpdate);
    if ( ret == -1 )
        // this is if someone change the file by hand...
        autoCB->setCurrentIndex( autoCB->findData(KpkEnum::AutoUpdateDefault) );
    else
        autoCB->setCurrentIndex(ret);

    if ( m_actions.contains(Client::ActionGetRepoList) ) {
        m_trasaction = Client::instance()->getRepoList(PackageKit::Client::FilterNotDevelopment);
        connect(m_trasaction, SIGNAL(repoDetail(const QString &, const QString &, bool)),
                m_originModel, SLOT(addOriginItem(const QString &, const QString &, bool)));
        connect(m_trasaction, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                m_originModel, SLOT(finished()));
        connect(m_originModel, SIGNAL(stateChanged()), this, SLOT(checkChanges()));
    }
}

void KpkSettings::save()
{
    KConfig config("KPackageKit");
    KConfigGroup notifyGroup( &config, "Notify" );
    notifyGroup.writeEntry("notifyUpdates", (int) notifyUpdatesCB->checkState());
    notifyGroup.writeEntry("notifyLongTasks", (int) notifyLongTasksCB->checkState());
    KConfigGroup checkUpdateGroup( &config, "CheckUpdate");
    checkUpdateGroup.writeEntry("interval", intervalCB->itemData(intervalCB->currentIndex()).toUInt());
    checkUpdateGroup.writeEntry("autoUpdate", autoCB->itemData(autoCB->currentIndex()).toUInt());
    // check to see if the backend support this
    if ( m_actions.contains(Client::ActionGetRepoList) ) {
        if ( !m_originModel->save() ) {
            KMessageBox::error(this, i18n("Authentication failed"), i18n("KPackageKit"));
            QTimer::singleShot(1, this, SLOT(checkChanges()));
        }
        on_showOriginsCB_stateChanged(showOriginsCB->checkState());
    }
}

void KpkSettings::defaults()
{
    notifyUpdatesCB->setCheckState(Qt::Checked);
    notifyLongTasksCB->setCheckState(Qt::Checked);
    intervalCB->setCurrentIndex(intervalCB->findData(KpkEnum::TimeIntervalDefault));
    autoCB->setCurrentIndex(autoCB->findData(KpkEnum::AutoUpdateDefault) );
    m_originModel->clearChanges();
    emit checkChanges();
}

#include "KpkSettings.moc"
