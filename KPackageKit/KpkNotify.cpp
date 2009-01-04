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

#include "KpkNotify.h"
#include <KpkStrings.h>
#include <KpkTransaction.h>

#include <KLocale>
#include <KIcon>
#include <QSize>
#include <KConfig>
#include <KConfigGroup>
#include <KRun>

// #include <kworkspace/kworkspace.h>

#include <KDebug>

#define NONE 0
#define SECURITY 1
#define ALL 2

namespace kpackagekit {

KpkNotify::KpkNotify( QObject *parent ) : QObject( parent ),
 m_showingRestartReq(false),
 m_showingAutoUpdate(false),
 m_showingUpdates(false)
{
    Client::instance()->setLocale(KGlobal::locale()->language() + "." + KGlobal::locale()->encoding());
}

KpkNotify::~KpkNotify()
{
}

void KpkNotify::rebootRequired(bool required)
{
    if (!m_showingRestartReq) {
	m_showingRestartReq = true;
	m_notifyRestartReq = new KNotification("RestartRequired");
	QString text;
	text.append( i18n("<b>The update has completed</b>") );
	if (required) {
	    text.append( i18n("<br />A system reboot is required") );
	    QStringList actions( i18n("Restart Computer Now") );
	    m_notifyRestartReq->setActions(actions);
	    connect( m_notifyRestartReq, SIGNAL( action1Activated() ), this , SLOT( rebootRequiredAction() ) );
	    KIcon icon("system-restart");
	    m_notifyRestartReq->setPixmap( icon.pixmap( QSize(64,64) ) );
	}
	else {
	    KIcon icon("dialog-ok");
	    m_notifyRestartReq->setPixmap( icon.pixmap( QSize(32,32) ) );
	}
	m_notifyRestartReq->setText(text);
	connect( m_notifyRestartReq, SIGNAL( closed() ), this , SLOT( rebootRequiredClosed() ) );
	m_notifyRestartReq->sendEvent();
    }
}

void KpkNotify::rebootRequiredAction()
{
    kDebug() << "Restarting";
emit showUpdatesUi();
// KWorkSpace::ShutdownConfirm confirm = KWorkSpace::ShutdownConfirmDefault;
// KWorkSpace::ShutdownType type = KWorkSpace::ShutdownTypeReboot;
//  KWorkSpace::requestShutDown(confirm,type)
//
// org::kde::KSMServerInterface ksmserver("org.kde.ksmserver", "/KSMServer", QDBusConnection::sessionBus());
// 170         QDBusReply<void> reply = ksmserver.logout((int)confirm,  (int)sdtype,  (int)sdmode);
// 171         return (reply.isValid());

    m_notifyRestartReq->close();
}

void KpkNotify::rebootRequiredClosed()
{
    kDebug() << "closed reboot req";
    m_showingRestartReq = false;
    emit appClose();
}

void KpkNotify::installingAutoUpdates(Transaction *transaction)
{
    kDebug() << "installingAutoUpdates";
    KConfig config("KPackageKit");
    KConfigGroup checkUpdateGroup( &config, "Notify" );
    if ( checkUpdateGroup.readEntry( "notifyAutoUpdate", true ) ) {
	if ( m_showingAutoUpdate ) {
	    return;
	}
	else {
	    m_notifyAutoUpdate = new KNotification("AutoInstallingUpdates");
	    m_showingAutoUpdate = true;
	}
	kDebug() << "installingAutoUpdates SHOW";
	m_transactionAutoUpdates = transaction;
	QString text;
	text.append( i18n("<b>Updates are being installed</b>") );
	text.append( i18n("<br />Updates are being automatically installed") );
	m_notifyAutoUpdate->setText(text);
	KIcon icon("plasmagik");
	m_notifyAutoUpdate->setPixmap( icon.pixmap( QSize(64,64) ) );
	QStringList actions;
	if ( transaction->allowCancel() )
	    actions << i18n("Cancel Update");
	actions << i18n("Don't notify me again");
	m_notifyAutoUpdate->setActions(actions);
	connect( m_notifyAutoUpdate, SIGNAL( activated(uint) ), this , SLOT( autoUpdatesActions(uint) ) );
	connect( m_notifyAutoUpdate, SIGNAL( closed() ), this , SLOT( autoUpdatesClosed() ) );
	m_notifyAutoUpdate->sendEvent();
    }
}

void KpkNotify::autoUpdatesActions(uint action)
{
    kDebug() << "auto updates actions " << m_notifyAutoUpdate->actions() << ":::" << action;
    if ( m_notifyAutoUpdate->actions().size() - action == 0 ) {
	KConfig config("KPackageKit");
	KConfigGroup smartIconGroup( &config, "Notify" );
	smartIconGroup.writeEntry( "notifyAutoUpdate", false );
	m_notifyAutoUpdate->close();
    }
    else {
	m_transactionAutoUpdates->cancel();
	m_notifyAutoUpdate->close();
    }
}

void KpkNotify::autoUpdatesClosed()
{
    kDebug() << "autoUpdatesActionsClosed";
    m_showingAutoUpdate = false;
    emit appClose();
}

void KpkNotify::showUpdates()
{
    KConfig config("KPackageKit");
    KConfigGroup notifyGroup( &config, "Notify" );
    if ( Qt::Checked == (Qt::CheckState) notifyGroup.readEntry("notifyUpdates", (int) Qt::Checked ) ) {
	kDebug() << "SHOW UPDATES";
	if (!m_showingUpdates){
	    m_showingUpdates = true;
	    m_showUpdatesNotify = new KNotification("ShowUpdates");
	    Transaction *t = Client::instance()->getUpdates();
	    connect( t, SIGNAL( package(PackageKit::Package *) ),
		this, SLOT( gotPackage(PackageKit::Package *) ) );
	    connect( t, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint) ),
		this, SLOT( getUpdatesFinished(PackageKit::Transaction::ExitStatus, uint)) );
	}
    }
}

void KpkNotify::gotPackage(PackageKit::Package *p)
{
    if ( p->state() != Package::Blocked )
	packages << p;
}

void KpkNotify::getUpdatesFinished(PackageKit::Transaction::ExitStatus status, uint /*runtime*/)
{
    qDebug() << "trans getUpdates finished: " << status ;
    if (status == Transaction::Success && packages.size() && m_showingUpdates) {
	kDebug() << "We have updates let's show...";
	// here we check if they are too many
	// i think 5 is the maximun number to show them detailed
	if ( packages.size() <= 5 ) {
	    QString text;
	    text.append( i18np("<b>You have one update:</b>", "<b>You have %1 updates:</b>", packages.size() ) );
	    for ( int i = 0; i < packages.size(); i++) {
		text.append( "<br><b>" + packages.at(i)->name() + "</b> - " + packages.at(i)->summary() );
	    }
	    m_showUpdatesNotify->setText(text);
	}
	else {
	    QString text;
	    text.append( i18np("<b>You have one update:</b>", "<b>You have %1 updates:</b>", packages.size() ) );

	    QHash<Package::State, int> state;
	    for ( int i = 0; i < packages.size(); i++) {
		state[ packages.at(i)->state() ] = ++state[ packages.at(i)->state() ];
	    }

	    QHashIterator<Package::State, int> i(state);
	    while (i.hasNext()) {
		i.next();
		text.append( "<br>" + KpkStrings::infoUpdate( i.key(), i.value() ) );
	    }

	    m_showUpdatesNotify->setText(text);
	}
	KIcon icon("security-high");
	m_showUpdatesNotify->setPixmap( icon.pixmap( QSize(128,128) ) );
	QStringList actions( i18n("Update") );
	actions << i18n("Not now");
	actions << i18n("Don't ask anymore");
	m_showUpdatesNotify->setActions(actions);
	m_showUpdatesNotify->setFlags( KNotification::Persistent );
	connect( m_showUpdatesNotify, SIGNAL( activated(uint) ), this , SLOT( updatesActions(uint) ) );
	connect( m_showUpdatesNotify, SIGNAL( closed() ), this , SLOT( showUpdatesClosed() ) );
	m_showUpdatesNotify->sendEvent();
    }
    else {
	m_showUpdatesNotify->close();
    }
}

void KpkNotify::showUpdatesClosed()
{
    kDebug() << "closed";
    packages.clear();
    m_showingUpdates = false;
    emit appClose();
}

void KpkNotify::updatesActions(uint action)
{
    kDebug() << "update Action " << action << " updatesNotification = " << m_showUpdatesNotify;
    switch (action) {
	case 1:
	    if ( packages.size() <= 5 ) {
		// we already show the user all the packages to be updated
		Client::instance()->updateSystem();
	    }
	    else {
		emit showUpdatesUi();
	    }
	    break;
	case 2:
	    // Not now action
	    emit showUpdatesTrayIcon();
	    break;
	case 3:
	    KConfig config("KPackageKit");
	    KConfigGroup smartIconGroup( &config, "SmartIcon" );
	    smartIconGroup.writeEntry( "notifyUpdates", 0 );
            break;
    }
    // we have to manualy call close (otherwise will cause memory leak)
    m_showUpdatesNotify->close();
}


}

#include "KpkNotify.moc"
