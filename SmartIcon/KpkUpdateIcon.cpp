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

#include <KpkIcons.h>
#include <KDebug>
#include <KpkStrings.h>
#include <KpkEnum.h>

#include <QMenu>
#include <KStandardAction>
#include <KActionCollection>
#include <KAction>
#include <solid/powermanagement.h>

using namespace PackageKit;

KpkUpdateIcon::KpkUpdateIcon(QObject* parent)
    : QObject(parent),
    m_updateNotify(0),
    m_checking(false),
    m_inhibitCookie(-1)
{
    m_icon = new KSystemTrayIcon("applications-other");
    connect(m_icon, SIGNAL( activated( QSystemTrayIcon::ActivationReason ) ),
             this, SLOT( showUpdates( QSystemTrayIcon::ActivationReason ) ));
    m_updateView = 0;
    m_distroUpgradeProcess = 0;

    m_icon->actionCollection()->addAction(KStandardAction::Preferences, this, SLOT( showSettings() ));
    m_icon->contextMenu()->addAction(m_icon->actionCollection()->action(KStandardAction::name(KStandardAction::Preferences)));
}

KpkUpdateIcon::~KpkUpdateIcon()
{
    // if we do not manually delete it,
    // it will be a useless notify
    if (m_updateNotify) {
        m_updateNotify->close();
    }
}

void
KpkUpdateIcon::showSettings()
{
    KCMultiDialog* settings = new KCMultiDialog();
    settings->setWindowIcon( KIcon("applications-other") );
    settings->addModule( KCModuleInfo::KCModuleInfo("kpk_settings.desktop") );
    connect(settings, SIGNAL( finished() ),
             settings, SLOT( deleteLater() ));
    settings->show();
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
KpkUpdateIcon::updaterClosed(int exitCode)
{
    hideUpdates();
    if (exitCode == QDialog::Accepted) {
        m_icon->hide();
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
                     this, SLOT( updaterClosed(int) ));
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
    // This is really necessary to don't bother the user with
    // tons of popups
    if (!m_checking) {
        KConfig config("KPackageKit");
        KConfigGroup notifyGroup(&config, "Notify");
        if (Qt::Checked == (Qt::CheckState) notifyGroup.readEntry("notifyUpdates", (int) Qt::Checked)) {
            m_checking = true;
            m_updateList.clear();
            Transaction* t = Client::instance()->getUpdates();
            connect(t, SIGNAL(package(PackageKit::Package *)),
                    this, SLOT(updateListed(PackageKit::Package *)));
            connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                    this, SLOT(updateCheckFinished(PackageKit::Transaction::ExitStatus, uint)));
        }
    }
}

void
KpkUpdateIcon::checkDistroUpgrades()
{
    Transaction* t = Client::instance()->getDistroUpgrades();

    connect(t, SIGNAL( distroUpgrade(PackageKit::Client::UpgradeType, const QString&, const QString& ) ),
             this, SLOT( distroUpgrade(PackageKit::Client::UpgradeType, const QString&, const QString& ) ) );
}

void
KpkUpdateIcon::updateListed(PackageKit::Package* package)
{
    // Blocked updates are not instalable updates so there is no
    // reason to show them
    if (package->state() != Package::Blocked) {
        m_updateList.append(package);
    }
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
    m_updateNotify = new KNotification("ShowUpdates", m_updateView, KNotification::Persistent | KNotification::CloseWhenWidgetActivated);
    // use of QSize does the right thing
    m_updateNotify->setPixmap(icon.pixmap(QSize(128,128)));

    QString text;
    text.append( KpkStrings::infoUpdate(highState, packageGroups[highState].size()) );
    int i;
    for(i=0;i<5 && i<packageGroups[highState].size();i++)
        text.append("<br><b>"+packageGroups[highState].at(i)->name()+"</b> - "+packageGroups[highState].at(i)->summary());
    i = m_updateList.size() - i;
    if (i>0)
        text.append(i18np("<br><i>And another update</i>", "<br><i>And %1 more updates</i>", i));

    m_updateNotify->setText(text);
    QStringList actions;
    m_icon->setToolTip(i18np("%1 update available", "%1 updates available", m_updateList.size()));
    m_icon->show();
    actions << i18n("Review and update");
    actions << i18n("Remind me later");
    actions << i18n("Don't ask anymore.");
    m_updateNotify->setActions(actions);
    m_updateNotify->sendEvent();
    connect(m_updateNotify, SIGNAL(activated(uint)), this, SLOT(handleUpdateAction(uint)));
    connect(m_updateNotify, SIGNAL(closed()), this , SLOT(handleUpdateActionClosed()));
}

void
KpkUpdateIcon::updateCheckFinished(PackageKit::Transaction::ExitStatus, uint runtime)
{
    Q_UNUSED(runtime);
    if (m_updateList.size() > 0) {
        kDebug() << "Found " << m_updateList.size() << " updates";
        Package::State highState = Package::Installed;
        //FIXME: This assumes that PackageKit shares our priority ranking.
        foreach(Package* p, m_updateList)
            if (p->state() > highState) {
                highState = p->state();
            }
        m_icon->setIcon(KpkIcons::packageIcon(highState));
//         m_icon->setToolTip("");
//         m_icon->show();
        KConfig config("KPackageKit");
        KConfigGroup checkUpdateGroup( &config, "CheckUpdate" );
        uint updateType = (uint) checkUpdateGroup.readEntry( "autoUpdate", KpkEnum::AutoUpdateDefault );
        if (updateType == KpkEnum::None) {
            kDebug() << "None.";
            notifyUpdates();
        } else {
            if (updateType == KpkEnum::All) {
                kDebug() << "All";
                if (Transaction* t = Client::instance()->updateSystem()) {
                    connect(t, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint) ),
                             this, SLOT( updatesFinished(PackageKit::Transaction::ExitStatus, uint) ) );
                             //autoUpdatesInstalling(t);
                             KNotification* autoInstallNotify = new KNotification("AutoInstallingUpdates");
                             autoInstallNotify->setText(i18n("Updates are being automatically installed."));
                             // use of QSize does the right thing
                             KIcon icon("plasmagik");
                             autoInstallNotify->setPixmap(icon.pixmap(QSize(128,128)));
                             autoInstallNotify->sendEvent();
                } else {
                    kDebug() << "All Trans failed.";
                    notifyUpdates();
                }
            } else {
                // Defaults to security
                kDebug() << "Security";
                QList<PackageKit::Package*> updateList;
                foreach(PackageKit::Package* package, m_updateList) {
                    if (package->state()==PackageKit::Package::Security)
                        updateList.append(package);
                }
                if (updateList.size()>0) {
                    if (Transaction* t = Client::instance()->updatePackages(updateList)) {
                        suppressSleep(true);
                        connect(t, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint) ),
                                this, SLOT( updatesFinished(PackageKit::Transaction::ExitStatus, uint) ) );
                        //autoUpdatesInstalling(t);
                        KNotification* autoInstallNotify = new KNotification("AutoInstallingUpdates");
                        autoInstallNotify->setText(i18n("Security updates are being automatically installed."));
                        // use of QSize does the right thing
                        autoInstallNotify->setPixmap(KpkIcons::packageIcon(highState).pixmap(QSize(128,128)));
                        autoInstallNotify->sendEvent();
                    } else {
                        kDebug() << "security Trans failed.";
                        notifyUpdates();
                    }
                } else {
                    kDebug() << "No security updates.";
                    notifyUpdates();
                }
            }
        }
    } else {
        m_checking = false;
    }
}

void KpkUpdateIcon::updatesFinished(PackageKit::Transaction::ExitStatus status, uint runtime)
{
    Q_UNUSED(runtime)
    KNotification* notify = new KNotification("UpdatesComplete");
    suppressSleep(false);
    if (status == Transaction::Success) {
        KIcon icon("task-complete");
        // use of QSize does the right thing
        notify->setPixmap(icon.pixmap(QSize(128,128)));
        notify->setText(i18n("System update was successful!"));
        notify->sendEvent();
        m_checking = false;
        // check for updates to see if there are updates that
        // couldn't be automatically installed
        checkUpdates();
    } else if (status==Transaction::Failed) {
        KIcon icon("dialog-cancel");
        // use of QSize does the right thing
        notify->setPixmap(icon.pixmap(QSize(128,128)));
        notify->setText(i18n("The software update failed.")); //TODO: Point the user to the logs, or give more detail.
        notify->sendEvent();
        m_checking = false;
    }
}

void KpkUpdateIcon::handleUpdateAction(uint action)
{
    qDebug() << "action" << action;
    switch(action) {
        case 1:
            showUpdates();
            break;
        case 2:
            break;
        case 3:
            KConfig config("KPackageKit");
            KConfigGroup smartIconGroup(&config, "Notify");
            smartIconGroup.writeEntry("notifyUpdates", 0);
            // TODO emit goingToQuit
            m_icon->hide();
            break;
    }
    // Manually calling close as the KNotification does not call it
    // when using persistant
    m_updateNotify->close();
}

void KpkUpdateIcon::handleUpdateActionClosed()
{
    kDebug();
    m_updateNotify = 0;
    m_checking = false;
}

void KpkUpdateIcon::distroUpgrade(PackageKit::Client::UpgradeType type, const QString& name, const QString& description)
{
    kDebug() << "Distro upgrade found!" << name << description;
    Q_UNUSED(type);
    KNotification* notify = new KNotification("DistroUpgradeAvailable", m_updateView, KNotification::Persistent | KNotification::CloseWhenWidgetActivated);

    QString text;

    text =  i18n("Distribution upgrade available") + "<br/>";
    text += "<b>" + name + "</b><br/>";
    text += description;

    notify->setText(text);

    QStringList actions;
    actions << i18n("Start upgrade now");
    notify->setActions(actions);
    connect(notify, SIGNAL(activated(uint)), this, SLOT(handleDistroUpgradeAction(uint)));
    notify->sendEvent();
}

void KpkUpdateIcon::handleDistroUpgradeAction(uint action)
{
    switch(action) {
        case 1:
            if ( m_distroUpgradeProcess ) {
                return;
            }
            m_distroUpgradeProcess = new QProcess;
            connect (m_distroUpgradeProcess, SIGNAL (error ( QProcess::ProcessError )),
                this, SLOT(distroUpgradeError( QProcess::ProcessError  ) ));
            connect (m_distroUpgradeProcess, SIGNAL (finished(int, QProcess::ExitStatus)),
                this, SLOT(distroUpgradeFinished(int, QProcess::ExitStatus  ) ));

            m_distroUpgradeProcess->start("/usr/share/PackageKit/pk-upgrade-distro.sh");
            suppressSleep(true);
            break;
        // perhaps more actions needed in the future
    }
}

void KpkUpdateIcon::distroUpgradeFinished( int exitCode, QProcess::ExitStatus exitStatus )
{
    KNotification* notify = new KNotification("DistroUpgradeFinished", m_updateView, KNotification::Persistent | KNotification::CloseWhenWidgetActivated);
    if ( exitStatus == QProcess::NormalExit && exitCode == 0 ) {
        notify->setPixmap(KIcon("security-high").pixmap(64, 64));
        notify->setText(i18n("Distribution upgrade finished. "));
    } else if ( exitStatus == QProcess::NormalExit ) {
        notify->setPixmap(KIcon("dialog-warning").pixmap(64, 64));
        notify->setText(i18n("Distribution upgrade process exited with code %1.", exitCode));
    }/* else {
        notify->setText(i18n("Distribution upgrade didn't exit normally, the process probably crashed. "));
    }*/
    notify->sendEvent();
    m_distroUpgradeProcess->deleteLater();
    m_distroUpgradeProcess = 0;
    suppressSleep(false);
}


void KpkUpdateIcon::distroUpgradeError( QProcess::ProcessError error )
{
    QString text;

    KNotification* notify = new KNotification("DistroUpgradeError", m_updateView, KNotification::Persistent | KNotification::CloseWhenWidgetActivated);
    switch(error) {
        case QProcess::FailedToStart:
            text = i18n("The distribution upgrade process failed to start."); 
            break;
        case QProcess::Crashed:
            text = i18n("The distribution upgrade process crashed some time after starting successfully.") ;
            break;
        default:
            text = i18n("The distribution upgrade process failed with an unknown error.");
            break;
    }
    notify->setPixmap(KIcon("dialog-error").pixmap(64,64));
    notify->setText(text);
    notify->sendEvent();
}

void KpkUpdateIcon::suppressSleep(bool enable)
{
    if ( enable ) {
        kDebug() << "Disabling powermanagement sleep";
        m_inhibitCookie = Solid::PowerManagement::beginSuppressingSleep( i18n("Installing updates.") );
        if (m_inhibitCookie == -1)
            kDebug() << "Sleep suppression denied!";
    } else {
        kDebug() << "Enable powermanagement sleep";
        if (m_inhibitCookie == -1)
            if ( ! Solid::PowerManagement::stopSuppressingSleep( m_inhibitCookie ))
                kDebug() << "Enable failed: invalid cookie.";
    }
}
