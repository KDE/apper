/***************************************************************************
 *   Copyright (C) 2008 by Trever Fischer                                  *
 *   wm161@wm161.net                                                       *
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

#include "KpkUpdateIcon.h"

#include <KpkIcons.h>
#include <KpkStrings.h>
#include <KpkEnum.h>
#include <KpkMacros.h>

#include <QMenu>
#include <KStandardAction>
#include <KActionCollection>
#include <KAction>

#include <KDebug>

#include <solid/powermanagement.h>

using namespace PackageKit;

KpkUpdateIcon::KpkUpdateIcon(QObject* parent)
    : KpkAbstractIsRunning(parent),
    m_updateNotify(0)
{
    // TODO this needs better thinking, it's not fine
    // to open various kcm at the same time (only one is kept open)
    // ALSO the default KSystemTrayIcon shows a quit that will QUIT
    // our smart-icon which won't be good 
//     m_icon = new KSystemTrayIcon("applications-other");
//     connect(m_icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
//             this, SLOT(showUpdates(QSystemTrayIcon::ActivationReason)));
// //     m_updateView = 0;
// 
//     m_icon->actionCollection()->addAction(KStandardAction::Preferences,
//                                           this, SLOT(showSettings()));
//     m_icon->contextMenu()->addAction(m_icon->actionCollection()->action(KStandardAction::name(KStandardAction::Preferences)));

//     int m_inhibitCookie = Solid::PowerManagement::beginSuppressingSleep( i18n("Installing updates.") );
//     if (m_inhibitCookie == -1)
//         kDebug() << "Sleep suppression denied!";
}

KpkUpdateIcon::~KpkUpdateIcon()
{
    // if we do not manually close it,
    // it will be a useless notify when PERSISTENT
    if (m_updateNotify) {
        m_updateNotify->close();
    }
}

void KpkUpdateIcon::showSettings()
{
    QProcess::execute("kpackagekit", QStringList() << "--settings");
//     KCMultiDialog* settings = new KCMultiDialog();
//     settings->setWindowIcon( KIcon("applications-other") );
//     settings->addModule( KCModuleInfo("kpk_settings.desktop") );
//     connect(settings, SIGNAL( finished() ),
//              settings, SLOT( deleteLater() ));
//     settings->show();
}

// void
// KpkUpdateIcon::hideUpdates()
// {
//     if (m_updateView!=0) {
//         m_updateView->deleteLater();
//         m_updateView = 0;
//     }
// }

// void
// KpkUpdateIcon::updaterClosed(int exitCode)
// {
//     hideUpdates();
//     if (exitCode == QDialog::Accepted) {
//         m_icon->hide();
//     }
// }

void KpkUpdateIcon::showUpdates(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::Unknown) {
        QProcess::execute("kpackagekit", QStringList() << "--updates");
//         if (m_updateView==0) {
//             m_updateView = new KCMultiDialog();
//             m_updateView->setWindowIcon( KIcon("applications-other") );
//             m_updateView->addModule( KCModuleInfo("kpk_update.desktop") );
//             connect(m_updateView, SIGNAL( finished(int) ),
//                      this, SLOT( updaterClosed(int) ));
//             m_updateView->raise();
//             m_updateView->show();
//         } else {
//             hideUpdates();
//         }
    }
}

// refresh the cache and try to update,
// if it can't automatically update show
// a notification about updates available
void KpkUpdateIcon::refreshAndUpdate(bool refresh)
{
    // This is really necessary to don't bother the user with
    // tons of popups
    if (!isRunning()) {
        if (refresh) {
            SET_PROXY
            Transaction *t = Client::instance()->refreshCache(true);
            if (t->error()) {
                KNotification *notify = new KNotification("TransactionError");
                notify->setText(KpkStrings::daemonError(t->error()));
                notify->setPixmap(KIcon("dialog-error").pixmap(KPK_ICON_SIZE, KPK_ICON_SIZE));
                notify->sendEvent();
            } else {
                connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                        this, SLOT(update()));
                // don't be interactive to not upset an idle user
                emit watchTransaction(t->tid(), false);
                increaseRunning();
                return; // to not reach the signal below
            }
        } else {
            update();
            increaseRunning();
        }
    }
}

void KpkUpdateIcon::update()
{
    KConfig config("KPackageKit");
    KConfigGroup notifyGroup(&config, "Notify");
    if (Qt::Checked == static_cast<Qt::CheckState>(notifyGroup.readEntry("notifyUpdates", static_cast<int>(Qt::Checked)))) {
        m_updateList.clear();
        Transaction *t = Client::instance()->getUpdates();
        connect(t, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                this, SLOT(updateListed(QSharedPointer<PackageKit::Package>)));
        connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                this, SLOT(updateCheckFinished(PackageKit::Transaction::ExitStatus, uint)));
    } else {
        decreaseRunning();
    }
}

void KpkUpdateIcon::updateListed(QSharedPointer<PackageKit::Package>package)
{
    // Blocked updates are not instalable updates so there is no
    // reason to show/count them
    if (package->info() != Enum::InfoBlocked) {
        m_updateList.append(package);
    }
}

//TODO: Notification contexts depending on highest priority update. Eg, Security, BugFix, Feature, etc
void KpkUpdateIcon::notifyUpdates()
{
    KConfig config("KPackageKit");
    KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
    Qt::CheckState notifyUpdates = static_cast<Qt::CheckState>(checkUpdateGroup.readEntry("notifyUpdates", static_cast<int>(Qt::Checked)));
    if (notifyUpdates == Qt::Unchecked) {
        return;
    }

    Enum::Info highState = Enum::InfoInstalled;
    QHash<Enum::Info, QList<QSharedPointer<PackageKit::Package> > > packageGroups;

    foreach(QSharedPointer<PackageKit::Package> p, m_updateList) {
        packageGroups[p->info()].append(p);
        if (p->info() > highState) {
            highState = p->info();
        }
    }

    KIcon icon = KpkIcons::packageIcon(highState);
    m_updateNotify = new KNotification("ShowUpdates", 0, KNotification::Persistent);
    // use of QSize does the right thing
    m_updateNotify->setPixmap(icon.pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));

    QString text;
    text = i18n("You have %1", KpkStrings::infoUpdate(highState, packageGroups[highState].size()));
    int i;
    for (i = 0; i < 5 && i < packageGroups[highState].size(); i++) {
        text.append("<br /><b>" + packageGroups[highState].at(i)->name() + "</b>" +
                    (packageGroups[highState].at(i)->summary().isEmpty()
                    ? "" : " - "+packageGroups[highState].at(i)->summary()));
    }
    i = m_updateList.size() - i;
    if (i > 0) {
        text.append(i18np("<br /><i>And another update</i>", "<br /><i>And %1 more updates</i>", i));
    }

    m_updateNotify->setText(text);

    QStringList actions;
    actions << i18n("Review and update");
    actions << i18n("Do not ask again");
    m_updateNotify->setActions(actions);
    m_updateNotify->sendEvent();
    connect(m_updateNotify, SIGNAL(activated(uint)),
            this, SLOT(handleUpdateAction(uint)));
    connect(m_updateNotify, SIGNAL(closed()),
            this , SLOT(handleUpdateActionClosed()));
    increaseRunning();
}

void KpkUpdateIcon::updateCheckFinished(PackageKit::Enum::Exit, uint runtime)
{
    Q_UNUSED(runtime)
    if (m_updateList.size() > 0) {
//         kDebug() << "Found " << m_updateList.size() << " updates";
        Enum::Info highState = Enum::InfoInstalled;
        //FIXME: This assumes that PackageKit shares our priority ranking.
        foreach(QSharedPointer<PackageKit::Package>p, m_updateList) {
            if (p->info() > highState) {
                highState = p->info();
            }
        }
//         m_icon->setIcon(KpkIcons::packageIcon(highState));
//         m_icon->setToolTip("");
//         m_icon->show();
        KConfig config("KPackageKit");
        KConfigGroup checkUpdateGroup(&config, "CheckUpdate");
        uint updateType = static_cast<uint>(checkUpdateGroup.readEntry("autoUpdate", KpkEnum::AutoUpdateDefault));
        if (updateType == KpkEnum::None) {
            // update none
            notifyUpdates();
        } else {
            if (updateType == KpkEnum::All) {
                // update all
                SET_PROXY
                Transaction* t = Client::instance()->updateSystem(true);
                if (t->error()) {
                    // update all failed
                    notifyUpdates();
                } else {
                    connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                            this, SLOT(updatesFinished(PackageKit::Transaction::ExitStatus, uint)));
                    // don't be interactive to not upset an idle user
                    emit watchTransaction(t->tid(), false);
                    //autoUpdatesInstalling(t);
                    KNotification *autoInstallNotify = new KNotification("AutoInstallingUpdates");
                    autoInstallNotify->setText(i18n("Updates are being automatically installed."));
                    // use of QSize does the right thing
                    KIcon icon("plasmagik");
                    autoInstallNotify->setPixmap(icon.pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
                    autoInstallNotify->sendEvent();
                    increaseRunning();
                }
            } else {
                // Defaults to security
                QList<QSharedPointer<PackageKit::Package> > updateList;
                foreach(QSharedPointer<PackageKit::Package>package, m_updateList) {
                    if (package->info() == Enum::InfoSecurity) {
                        updateList.append(package);
                    }
                }
                if (updateList.size() > 0) {
                    SET_PROXY
                    Transaction *t = Client::instance()->updatePackages(true, updateList);
                    if (t->error()) {
                        // security Trans failed.
                        notifyUpdates();
                    } else {
//                         suppressSleep(true);
                        connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                                this, SLOT(updatesFinished(PackageKit::Transaction::ExitStatus, uint)));
                        // don't be interactive to not upset an idle user
                        emit watchTransaction(t->tid(), false);
                        //autoUpdatesInstalling(t);
                        KNotification *autoInstallNotify = new KNotification("AutoInstallingUpdates");
                        autoInstallNotify->setText(i18n("Security updates are being automatically installed."));
                        // use of QSize does the right thing
                        autoInstallNotify->setPixmap(KpkIcons::packageIcon(highState).pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
                        autoInstallNotify->sendEvent();
                        increaseRunning();
                    }
                } else {
//                     kDebug() << "No security updates.";
                    notifyUpdates();
                }
            }
        }
    }
    decreaseRunning();
}

void KpkUpdateIcon::updatesFinished(PackageKit::Enum::Exit status, uint runtime)
{
    Q_UNUSED(runtime)
    // decrease first only because we want to check for updates again
    decreaseRunning();
    KNotification *notify = new KNotification("UpdatesComplete");
//     suppressSleep(false);
    if (status == Enum::ExitSuccess) {
        KIcon icon("task-complete");
        // use of QSize does the right thing
        notify->setPixmap(icon.pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
        notify->setText(i18n("System update was successful."));
        notify->sendEvent();
        // check for updates to see if there are updates that
        // couldn't be automatically installed
        update();
    } else {
        KIcon icon("dialog-cancel");
        // use of QSize does the right thing
        notify->setPixmap(icon.pixmap(QSize(KPK_ICON_SIZE, KPK_ICON_SIZE)));
        notify->setText(i18n("The software update failed.")); //TODO: Point the user to the logs, or give more detail.
        notify->sendEvent();
    }
}

void KpkUpdateIcon::handleUpdateAction(uint action)
{
//     kDebug() << "action" << action;
    switch(action) {
        case 1:
            showUpdates();
            break;
        case 2:
            KConfig config("KPackageKit");
            KConfigGroup smartIconGroup(&config, "Notify");
            smartIconGroup.writeEntry("notifyUpdates", 0);
            // TODO emit goingToQuit
//             m_icon->hide();
            break;
    }
    // Manually calling close as the KNotification does not call it
    // when using persistant
    m_updateNotify->close();
}

void KpkUpdateIcon::handleUpdateActionClosed()
{
//     kDebug();
    m_updateNotify = 0;
    decreaseRunning();
}

// TODO move this to KpkSmartIcon to handle all hidden transactions
// and create a static method to see if the role of the transaction is suficient
// to suppressSleep
// void KpkUpdateIcon::suppressSleep(bool enable)
// {
//     if (enable) {
//         kDebug() << "Disabling powermanagement sleep";
//         m_inhibitCookie = Solid::PowerManagement::beginSuppressingSleep( i18n("Installing updates.") );
//         if (m_inhibitCookie == -1)
//             kDebug() << "Sleep suppression denied!";
//     } else {
//         kDebug() << "Enable powermanagement sleep";
//         if (m_inhibitCookie == -1)
//             if ( ! Solid::PowerManagement::stopSuppressingSleep( m_inhibitCookie ))
//                 kDebug() << "Enable failed: invalid cookie.";
//     }
// }
