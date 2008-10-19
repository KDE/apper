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

#include "KpkUpdate.h"
#include <KpkStrings.h>

#include <KMessageBox>

#define UNIVERSAL_PADDING 6
 
KpkUpdate::KpkUpdate( QWidget *parent ) : QWidget( parent )
{
    setupUi( this );
    detailsDW->hide();

    //initialize the model, delegate, client and  connect it's signals
    packageView->setItemDelegate(pkg_delegate = new KpkDelegate(this));
    packageView->setModel(m_pkg_model_updates = new KpkUpdateModel(this, packageView));
    connect( m_pkg_model_updates, SIGNAL( updatesSelected(bool) ), updatePB, SLOT( setEnabled(bool) ) );

    // Create a new client
    m_client = Client::instance();

    // check to see what roles the backend
    m_actions = m_client->getActions();

    updateFinished(KpkTransaction::Success);
}

void KpkUpdate::on_updatePB_clicked()
{
    qDebug() << "update";
    QList<Package*> packages = m_pkg_model_updates->selectedPackages();
    //check to see if the user selected all selectable packages
    if ( packages.size() == m_pkg_model_updates->selectablePackages() )
	// if so let's do system-update instead
	if ( Transaction *t = m_client->updateSystem() ) {
	    KpkTransaction *frm = new KpkTransaction(t, this);
	    connect( frm, SIGNAL( kTransactionFinished(KpkTransaction::ExitStatus) ), this, SLOT( updateFinished(KpkTransaction::ExitStatus) ) );
	    frm->show();
	}
	else
	    KMessageBox::error( this, i18n("Authentication failed"), i18n("KPackageKit") );
    else
	// else lets install only the selected ones
	if ( Transaction *t = m_client->updatePackages(packages) ) {
	    KpkTransaction *frm = new KpkTransaction(t, this);
	    connect( frm, SIGNAL( kTransactionFinished(KpkTransaction::ExitStatus) ), this, SLOT( updateFinished(KpkTransaction::ExitStatus) ) );
	    frm->show();
	}
	else
	    KMessageBox::error( this, i18n("Authentication failed"), i18n("KPackageKit") );
}

void KpkUpdate::updateFinished(KpkTransaction::ExitStatus status)
{
    if (status == KpkTransaction::Success){
        m_pkg_model_updates->clear();
	m_updatesT = m_client->getUpdates();
	connect( m_updatesT, SIGNAL( package(PackageKit::Package *) ), m_pkg_model_updates, SLOT( addPackage(PackageKit::Package *) ) );
    }
}

void KpkUpdate::on_refreshPB_clicked()
{
    if ( Transaction *t = m_client->refreshCache(true) ) {
        KpkTransaction *frm = new KpkTransaction(t, this);
        connect( frm, SIGNAL( kTransactionFinished(KpkTransaction::ExitStatus) ),
	    this, SLOT( refreshCacheFinished(KpkTransaction::ExitStatus) ) );
        frm->show();
    }
    else
        KMessageBox::error( this, i18n("Authentication failed"), i18n("KPackageKit") );
}

void KpkUpdate::refreshCacheFinished(KpkTransaction::ExitStatus status)
{
    if (status == KpkTransaction::Success) {
        KConfig config("KPackageKit");
        KConfigGroup checkUpdateGroup( &config, "CheckUpdate" );
        checkUpdateGroup.writeEntry( "lastChecked", QDateTime::currentDateTime().toTime_t() );
    }
    updateFinished(KpkTransaction::Success);
}

void KpkUpdate::on_packageView_pressed( const QModelIndex & index )
{
    if ( index.column() == 0 ) {
        Package *p = m_pkg_model_updates->package(index);
	// check to see if the backend support
	if (p && m_actions.contains(Client::ActionGetUpdateDetail) ) {
	    Transaction *t = m_client->getUpdateDetail(p);
	    connect( t, SIGNAL( updateDetail(PackageKit::Client::UpdateInfo) ),
		this, SLOT( updateDetail(PackageKit::Client::UpdateInfo) ) );
	}
    }
}

void KpkUpdate::updateDetail(PackageKit::Client::UpdateInfo info)
{
    //format and show description
    QString description;
    description += "<b>" + i18n("New version") + ":</b> " + info.package->name() + "-" + info.package->version() + "<br />";
    if ( info.updates.size() ) {
	QStringList updates;
	foreach (Package *p, info.updates) updates << p->name() + "-" + p->version();
        description += "<b>" + i18n("Updates") + ":</b> " + updates.join(", ") + "<br />";
    }
    if ( info.obsoletes.size() ) {
	QStringList obsoletes;
	foreach (Package *p, info.obsoletes) obsoletes << p->id() + "-" + p->version();
        description += "<b>" + i18n("Obsoletes") + ":</b> " + obsoletes.join(", ") + "<br />";
    }
    if ( !info.updateText.isEmpty() )
        description += "<b>" + i18n("Details") + ":</b> " + info.updateText + "<br />";
    if ( !info.vendorUrl.isEmpty() )
        description += "<b>" + i18n("Vendor Home Page") + ":</b> <a href=\"" + info.vendorUrl.split(";").at(0) + "\">" + info.vendorUrl.split(";").at(1) + "</a><br />";
    if ( !info.bugzillaUrl.isEmpty() )
        description += "<b>" + i18n("Bugzilla Home Page") + ":</b> <a href=\"" + info.bugzillaUrl.split(";").at(0) + "\">" + info.bugzillaUrl.split(";").at(1) + "</a><br />";
    if ( !info.cveUrl.isEmpty() )
        description += "<b>" + i18n("CVE Home Page") + ":</b> <a href=\"" + info.cveUrl.split(";").at(0) + "\">" + info.cveUrl.split(";").at(1) + "</a><br />";
    if ( !info.changelog.isEmpty() )
        description += "<b>" + i18n("Change Log") + ":</b> " + info.changelog + "<br />";
    if ( info.state != Client::UnknownUpgradeType)
        description += "<b>" + i18n("State") + ":</b> " + KpkStrings::updateState(info.state) + "<br />";
    if ( info.restart != Client::UnknownRestartType)
        description += "<b>" + i18n("Restart") + ":</b> " + KpkStrings::restartTypeFuture(info.restart) + "<br />";
    if ( !info.issued.toString().isEmpty() )
        description += "<b>" + i18n("Issued") + ":</b> " + KGlobal::locale()->formatDate(info.issued.date(), KLocale::ShortDate) + "<br />";
    if ( !info.updated.toString().isEmpty() )
        description += "<b>" + i18n("Updated") + ":</b> " + KGlobal::locale()->formatDate(info.updated.date(), KLocale::ShortDate) + "<br />";
    descriptionKTB->setHtml(description);
    detailsDW->show();
}

void KpkUpdate::on_historyPB_clicked()
{
//     QStringList history;
    QString text;
//     history << KGlobal::locale()->formatDuration( m_daemon->getTimeSinceAction(Role::Refresh_cache) * 1000);
    text.append("Time since last cache refresh: " + KGlobal::locale()->formatDuration( m_client->getTimeSinceAction(Client::ActionRefreshCache) * 1000) );
// // TODO improve this to show more info
// //     text.append("transactions:");
    KMessageBox::information( this, text, i18n("History") );
// //     KMessageBox::informationList( this, text, history, i18n("History") );
}

void KpkUpdate::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateColumnsWidth();
}

bool KpkUpdate::event(QEvent *event)
{
    switch (event->type()) {
	case QEvent::Paint:
        case QEvent::PolishRequest:
        case QEvent::Polish:
            updateColumnsWidth(true);
            break;
        default:
            break;
    }

    return QWidget::event(event);
}

void KpkUpdate::updateColumnsWidth(bool force)
{
    int m_viewWidth = packageView->viewport()->width();

    if (force) {
        m_viewWidth -= style()->pixelMetric(QStyle::PM_ScrollBarExtent) + UNIVERSAL_PADDING;
    }

    packageView->setColumnWidth(0, pkg_delegate->columnWidth(0, m_viewWidth));
    packageView->setColumnWidth(1, pkg_delegate->columnWidth(1, m_viewWidth));
}

#include "KpkUpdate.moc" 
