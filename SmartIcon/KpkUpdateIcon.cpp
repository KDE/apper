/**************************************************************************
*   Copyright (C) 2008 by Trever Fischer                                  *
*   wm161@wm161.net                                                       *
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

#include "KpkUpdateIcon.h"
#include <KNotification>
#include <KpkIcons.h>
#include <KDebug>
#include <KpkStrings.h>

//TODO: These constants are scattered around the source. Put them somewhere nice.
#define NONE 0
#define SECURITY 1
#define ALL 2

using namespace PackageKit;

KpkUpdateIcon::KpkUpdateIcon(QObject* parent)
    : QObject(parent)
{
    m_icon = new KSystemTrayIcon("applications-other");
    connect(m_icon, SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ),
             this, SLOT( showUpdates( QSystemTrayIcon::ActivationReason ) ));
    m_updateView = 0;
}

void
KpkUpdateIcon::hideUpdates()
{
    if (m_updateView!=0) {
        m_updateView->deleteLater();
        m_updateView = 0;
    }
}

void
KpkUpdateIcon::showUpdates(QSystemTrayIcon::ActivationReason reason)
{
    if (reason==QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::Unknown) {
        if (m_updateView==0) {
            m_updateView = new KCMultiDialog();
            m_updateView->setWindowIcon( KIcon("applications-other") );
            m_updateView->addModule( KCModuleInfo::KCModuleInfo("kpk_update.desktop") );
            connect(m_updateView, SIGNAL( finished(int) ),
                    this, SLOT( hideUpdates() ));
            m_updateView->raise();
            m_updateView->show();
        } else {
            hideUpdates();
        }
    }
}

void
KpkUpdateIcon::checkUpdates()
{
    m_updateList.clear();
    Transaction* t = Client::instance()->getUpdates();
    connect(t, SIGNAL( package(PackageKit::Package*) ),
             this, SLOT( updateListed(PackageKit::Package*) ) );
    connect(t, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint) ),
            this, SLOT( updateCheckFinished(PackageKit::Transaction::ExitStatus, uint ) ));
}

void
KpkUpdateIcon::updateListed(PackageKit::Package* package)
{
    m_updateList.append(package);
}

//TODO: Notification contexts depending on highest priority update. Eg, Security, BugFix, Feature, etc
void
KpkUpdateIcon::notifyUpdates()
{
    KConfig config("KPackageKit");
    KConfigGroup checkUpdateGroup( &config, "CheckUpdate" );
    Qt::CheckState notifyUpdates = (Qt::CheckState) checkUpdateGroup.readEntry( "notifyUpdates",(int) Qt::Checked );
    if (notifyUpdates==Qt::Unchecked)
        return;
    
    Package::State highState = Package::Installed;
    QHash<Package::State, QList<Package*> > packageGroups;

    foreach(Package* p, m_updateList)
        packageGroups[p->state()].append(p);

    foreach(Package::State state, packageGroups.keys())
        if (state>highState)
            highState = state;

    KIcon icon = KpkIcons::packageIcon(highState);
    KNotification* updateNotify = new KNotification("ShowUpdates", m_updateView, KNotification::Persistent | KNotification::CloseWhenWidgetActivated);
    updateNotify->setPixmap(icon.pixmap(64, 64));
    
    QString text;
    text.append( KpkStrings::infoUpdate(highState, packageGroups[highState].size()) );
    int i;
    for(i=0;i<5 && i<packageGroups[highState].size();i++)
        text.append("<br><b>"+packageGroups[highState].at(i)->name()+"</b> - "+packageGroups[highState].at(i)->summary());
    i = m_updateList.size() - i;
    if (i>0)
        text.append(i18n("<br><i>And %1 more updates</i>").arg(i));
    
    updateNotify->setText(text);
    QStringList actions;
    
    actions << i18n("Review and update");
    actions << i18n("Remind me later");
    actions << i18n("Don't ask anymore.");
    updateNotify->setActions(actions);
    updateNotify->sendEvent();
    connect(updateNotify, SIGNAL(activated(uint)), this, SLOT(handleUpdateAction(uint)));
}

void
KpkUpdateIcon::updateCheckFinished(PackageKit::Transaction::ExitStatus, uint runtime)
{
    Q_UNUSED(runtime);
    if (m_updateList.size()>0) {
        kDebug() << "Found " << m_updateList.size() << " updates";
        Package::State highState = Package::Installed;
        //FIXME: This assumes that PackageKit shares our priority ranking.
        foreach(Package* p, m_updateList)
            if (p->state()>highState)
                highState = p->state();
        m_icon->setIcon(KpkIcons::packageIcon(highState));
        m_icon->show();
        KConfig config("KPackageKit");
        KConfigGroup checkUpdateGroup( &config, "CheckUpdate" );
        uint updateType = checkUpdateGroup.readEntry( "autoUpdate", SECURITY );
        if (updateType == NONE) {
            notifyUpdates();
        } else {
            if (updateType == SECURITY) {
                QList<PackageKit::Package*> updateList;
                foreach(PackageKit::Package* package, m_updateList) {
                    if (package->state()==PackageKit::Package::Security)
                        updateList.append(package);
                }
                if (updateList.size()>0) {
                    Transaction* t = Client::instance()->updatePackages(updateList);
                    if (t) {
                        connect(t, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint) ),
                                this, SLOT( updatesFinished(PackageKit::Transaction::ExitStatus, uint) ) );
                        //autoUpdatesInstalling(t);
                        KNotification* autoInstallNotify = new KNotification("AutoInstallingUpdates");
                        autoInstallNotify->setText(i18n("Security updates are being automatically installed."));
                        autoInstallNotify->setPixmap(KpkIcons::packageIcon(highState).pixmap(64,64));
                        autoInstallNotify->sendEvent();
                    } else {
                        notifyUpdates();
                    }
                } else {
                    notifyUpdates();
                }
            } else {
                if (Transaction* t = Client::instance()->updateSystem()) {
                    connect(t, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint) ),
                             this, SLOT( updatesFinished(PackageKit::Transaction::ExitStatus, uint) ) );
                             //autoUpdatesInstalling(t);
                             KNotification* autoInstallNotify = new KNotification("AutoInstallingUpdates");
                             autoInstallNotify->setText(i18n("Updates are being automatically installed."));
                             autoInstallNotify->setPixmap(KpkIcons::packageIcon(highState).pixmap(64,64));
                             autoInstallNotify->sendEvent();
                } else {
                    notifyUpdates();
                }
            }
        }
    }
}

void KpkUpdateIcon::updatesFinished(PackageKit::Transaction::ExitStatus status, uint runtime)
{
    Q_UNUSED(runtime)
    KNotification* notify = new KNotification("UpdatesComplete");
    if (status == Transaction::Success) {
        KIcon icon("task-complete");
        notify->setPixmap(icon.pixmap(64, 64));
        notify->setText(i18n("System update was successful!"));
    } else if (status==Transaction::Failed) {
        KIcon icon("dialog-cancel");
        notify->setPixmap(icon.pixmap(64, 64));
        notify->setText(i18n("The software update failed.")); //TODO: Point the user to the logs, or give more detail.
    }
    notify->sendEvent();
}

void KpkUpdateIcon::handleUpdateAction(uint action)
{
    switch(action) {
        case 1:
            showUpdates();
            break;
        case 2:
            break;
        case 3:
            KConfig config("KPackageKit");
            KConfigGroup smartIconGroup( &config, "SmartIcon" );
            smartIconGroup.writeEntry( "notifyUpdates", 0 );
            break;
    }
}
