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

#include <KLocale>
#include <KStandardDirs>
#include <KMessageBox>

#include <QPalette>
#include <QColor>

#include "KpkAddRm.h"
#include "KpkReviewChanges.h"
#include <KpkStrings.h>

#define UNIVERSAL_PADDING 6

KpkAddRm::KpkAddRm( QWidget *parent )
 : QWidget( parent ),m_mTransRuning(false), m_findIcon("edit-find"),
   m_cancelIcon("dialog-cancel")
{
    setupUi( this );

    // Create a new daemon
    m_client = Client::instance();

    //initialize the model, delegate, client and  connect it's signals
    packageView->setItemDelegate(pkg_delegate = new KpkDelegate(this));
    packageView->setModel( m_pkg_model_main = new KpkPackageModel(this, packageView) );
    packageView->viewport()->setAttribute(Qt::WA_Hover);

    // check to see if the backend support these actions
    m_actions = m_client->getActions();
    if ( m_actions.contains(Client::ActionInstallPackages) || m_actions.contains(Client::ActionRemovePackages) )
        connect( m_pkg_model_main, SIGNAL( dataChanged(const QModelIndex, const QModelIndex) ), this, SLOT( checkChanged() ) );

    if ( m_actions.contains(Client::ActionGetDetails) )
	connect(this, SIGNAL( getInfo(PackageKit::Package *) ), this, SLOT( getDetails(PackageKit::Package *) ) );
    else
	tabWidget->setTabEnabled(0, false);

    if ( m_actions.contains(Client::ActionGetFiles) )
	connect(this, SIGNAL( getInfo(PackageKit::Package *) ), this, SLOT( getFiles(PackageKit::Package *) ) );
    else
        tabWidget->setTabEnabled(1, false);

    if ( m_actions.contains(Client::ActionGetDepends) ) {
	dependsOnLV->setModel( m_pkg_model_dep = new KpkPackageModel(this, packageView) );
	connect(this, SIGNAL( getInfo(PackageKit::Package *) ), this, SLOT( getDepends(PackageKit::Package *) ) );
    }
    else
        tabWidget->setTabEnabled(2, false);

    if ( m_actions.contains(Client::ActionGetRequires) ) {
	requiredByLV->setModel( m_pkg_model_req = new KpkPackageModel(this, packageView ));
	connect(this, SIGNAL( getInfo(PackageKit::Package *) ), this, SLOT( getRequires(PackageKit::Package *) ) );
    }
    else
        tabWidget->setTabEnabled(3, false);

    if ( !m_actions.contains(Client::ActionSearchName) )
        findPB->setEnabled(false);

    if ( !m_actions.contains(Client::ActionSearchGroup) )
        groupsCB->setEnabled(false);

    //initialize the groups
    foreach (Client::Group group, m_client->getGroups() ) {
	groupsCB->addItem( KpkStrings::groupsIcon(group), KpkStrings::groups(group), group );
    }

    // install the backend filters
    filterMenu( m_client->getFilters() );

    // connect the notify
    connect( &m_notifyT, SIGNAL( timeout() ), this, SLOT( notifyUpdate() ) );

    // set fucus on the search lineEdit
    searchKLE->setFocus(Qt::OtherFocusReason);
    findPB->setDefault(true);
    findPB->setIcon(m_findIcon);

    // hides the description to have more space.
    descriptionDW->setVisible(false);
    notifyF->hide();
}

void KpkAddRm::checkChanged()
{
    if (m_pkg_model_main->selectedPackages().size()>0)
      emit changed(true);
    else
      emit changed(false);
}

void KpkAddRm::getDetails(PackageKit::Package *p)
{
    // create the description transaction
    Transaction *t = m_client->getDetails(p);
    connect( t, SIGNAL(details(PackageKit::Package *) ),
        this, SLOT( description(PackageKit::Package *) ) );
    connect( t, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint) ),
	this, SLOT( getInfoFinished(PackageKit::Transaction::ExitStatus, uint) ) );
}

void KpkAddRm::getFiles(PackageKit::Package *p)
{
    // create the files transaction
    Transaction *t = m_client->getFiles(p);
    connect( t, SIGNAL( files(PackageKit::Package *, const QStringList &) ),
	this, SLOT( files(PackageKit::Package *, const QStringList &) ) );
    connect( t, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint) ),
	this, SLOT( getInfoFinished(PackageKit::Transaction::ExitStatus, uint) ) );
}

void KpkAddRm::getDepends(PackageKit::Package *p)
{
    // create a transaction for the dependecies, and its model.
    Transaction *t = m_client->getDepends(p);
    m_pkg_model_dep->clear();
    connect( t, SIGNAL( package(PackageKit::Package *) ),
	m_pkg_model_dep, SLOT( addPackage(PackageKit::Package *) ) );
    connect( t, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint) ),
	this, SLOT( getInfoFinished(PackageKit::Transaction::ExitStatus, uint) ) );
}

void KpkAddRm::getRequires(PackageKit::Package *p)
{  
    // create a transaction for the requirements, and its model.
    Transaction *t = m_client->getRequires(p);
    m_pkg_model_req->clear();
    connect( t, SIGNAL( package(PackageKit::Package *) ),
	m_pkg_model_req, SLOT( addPackage(PackageKit::Package *) ) );
    connect( t, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint) ),
	this, SLOT( getInfoFinished(PackageKit::Transaction::ExitStatus, uint) ) );
}

void KpkAddRm::getInfoFinished(PackageKit::Transaction::ExitStatus status, uint runtime)
{
    Q_UNUSED(runtime);
    if (status == Transaction::Success)
	descriptionDW->setVisible(true);
}

void KpkAddRm::on_packageView_pressed( const QModelIndex & index )
{
    if ( index.column() == 0 ) {
        Package *p = m_pkg_model_main->package(index);
        if (p)
            emit getInfo(p);
    }
}

void KpkAddRm::statusChanged(PackageKit::Transaction::Status status)
{
    notifyF->show();
    notifyL->setText( KpkStrings::status(status) );
    busyPB->setMaximum(0);
}

void KpkAddRm::progressChanged(PackageKit::Transaction::ProgressInfo info)
{
    busyPB->setMaximum(100);
    busyPB->setValue(info.percentage);
}

void KpkAddRm::errorCode(PackageKit::Client::ErrorType error, const QString &details)
{
    KMessageBox::detailedSorry( this, KpkStrings::errorMessage(error), details, KpkStrings::error(error), KMessageBox::Notify );
}

void KpkAddRm::resizeEvent ( QResizeEvent * event )
{
    QWidget::resizeEvent(event);
    updateColumnsWidth();
}

bool KpkAddRm::event ( QEvent * event )
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

void KpkAddRm::updateColumnsWidth(bool force)
{
    int m_viewWidth = packageView->viewport()->width();

    if (force) {
        m_viewWidth -= style()->pixelMetric(QStyle::PM_ScrollBarExtent) + UNIVERSAL_PADDING;
    }

    packageView->setColumnWidth(0, pkg_delegate->columnWidth(0, m_viewWidth));
    packageView->setColumnWidth(1, pkg_delegate->columnWidth(1, m_viewWidth));
}

KpkAddRm::~KpkAddRm()
{
}

void KpkAddRm::on_findPB_clicked()
{
    if ( m_mTransRuning ) {
        m_pkClient_main->cancel();
    }
    else if ( !searchKLE->text().isEmpty() ) {
	// cache the search
	m_searchAction = Client::ActionSearchName;
	m_searchString = searchKLE->text();
	m_searchFilters = filters();
	// select "All Packages"
	groupsCB->setCurrentIndex(0);
	// create the main transaction
	search();
    }
}

void KpkAddRm::on_groupsCB_currentIndexChanged( int index )
{
    if ( groupsCB->itemData( index, Qt::UserRole ).isValid() ) {
	// cache the search
	m_searchAction = Client::ActionSearchGroup;
	m_searchGroup = (Client::Group) groupsCB->itemData( index, Qt::UserRole ).toUInt();
	m_searchFilters = filters();
	// create the main transaction
	search();
    }
}

void KpkAddRm::search()
{
    // search
    if ( m_searchAction == Client::ActionSearchGroup )
	m_pkClient_main = m_client->searchGroup( m_searchGroup, m_searchFilters );
    else if ( m_searchAction == Client::ActionSearchName )
	m_pkClient_main = m_client->searchName( m_searchString, m_searchFilters );
    connectTransaction(m_pkClient_main);
    statusChanged( m_pkClient_main->status() );
    // hides the description to have more space.
    descriptionDW->setVisible(false);
    notifyF->hide();
    // cleans the models
    m_pkg_model_main->clear();
    busyPB->setMaximum(0);
    busyPB->setValue(0);
    m_mTransRuning = true;
    findPB->setText( i18n("&Cancel") );
    findPB->setIcon(m_cancelIcon);
    findPB->setEnabled(false);
}

void KpkAddRm::connectTransaction(Transaction *transaction)
{
    connect( transaction, SIGNAL( package(PackageKit::Package *)),
	m_pkg_model_main, SLOT( addPackage(PackageKit::Package *)) );
    connect( transaction, SIGNAL( finished(PackageKit::Transaction::ExitStatus, uint)),
	this, SLOT( finished(PackageKit::Transaction::ExitStatus, uint)) );
    connect( transaction, SIGNAL( errorCode(PackageKit::Client::ErrorType, const QString&) ),
	this, SLOT( errorCode(PackageKit::Client::ErrorType, const QString &) ) );
    connect( transaction, SIGNAL( message(PackageKit::Client::MessageType, const QString&) ),
	this, SLOT( message(PackageKit::Client::MessageType, const QString &) ) );
    connect( transaction, SIGNAL( statusChanged(PackageKit::Transaction::Status) ),
	this, SLOT( statusChanged(PackageKit::Transaction::Status) ) );
    connect( transaction, SIGNAL( allowCancelChanged(bool) ),
	findPB, SLOT( setEnabled(bool) ) );
    connect( transaction, SIGNAL( progressChanged(PackageKit::Transaction::ProgressInfo) ),
	this, SLOT( progressChanged(PackageKit::Transaction::ProgressInfo) ) );
}

void KpkAddRm::message(PackageKit::Client::MessageType message, const QString &details)
{
    qDebug() << "Error code: " << message << " two: " << details;
}

void KpkAddRm::save()
{
    KpkReviewChanges *frm = new KpkReviewChanges( m_pkg_model_main->selectedPackages(), this);
    if ( frm->exec() == QDialog::Accepted )
        m_pkg_model_main->uncheckAll();
    else
	QTimer::singleShot(1, this, SLOT( checkChanged() ) );
    delete frm;
    search();
}

void KpkAddRm::load()
{
    m_pkg_model_main->uncheckAll();
}

void KpkAddRm::finished(PackageKit::Transaction::ExitStatus status, uint runtime)
{
    notifyF->show();
    QPalette teste;
    busyPB->setMaximum(100);
    busyPB->setValue(100);
    m_mTransRuning = false;
    findPB->setEnabled(true);
    findPB->setText( i18n("&Find") );
    findPB->setIcon(m_findIcon);
    switch(status) {
        case Transaction::Success :
	    notifyL->setText(i18n("Search finished in %1", KGlobal::locale()->formatDuration(runtime)) );
            teste.setColor( QPalette::Normal, QPalette::Window, QColor(0,255,0,150));
            notifyL->setPalette(teste);
            notifyL->setAutoFillBackground(true);
            m_notifyT.start(100);
	    break;
	case Transaction::Failed :
	    notifyL->setText(i18n("Search failed"));
            teste.setColor(QPalette::Normal, QPalette::Window, QColor(255,0,0,150));
            notifyL->setPalette(teste);
            notifyL->setAutoFillBackground(true);
            m_notifyT.start(50);
	    break;
	case Transaction::Cancelled :
            notifyL->setText(i18n("Search canceled"));
            teste.setColor( QPalette::Normal, QPalette::Window, QColor(0,255,0,150));
            notifyL->setPalette(teste);
            notifyL->setAutoFillBackground(true);
            m_notifyT.start(100);
            break;
	case Transaction::KeyRequired :
            notifyL->setText(i18n("Search finished in %1",KGlobal::locale()->formatDuration(runtime)) );
            teste.setColor( QPalette::Normal, QPalette::Window, QColor(0,255,0,150));
            notifyL->setPalette(teste);
            notifyL->setAutoFillBackground(true);
            m_notifyT.start(100);
            break;
	case Transaction::EulaRequired :
            notifyL->setText(i18n("Search finished in %1", KGlobal::locale()->formatDuration(runtime)) );
            teste.setColor( QPalette::Normal, QPalette::Window, QColor(0,255,0,150));
            notifyL->setPalette(teste);
            notifyL->setAutoFillBackground(true);
            m_notifyT.start(100);
            break;
	case Transaction::Killed :
            notifyL->setText(i18n("Search killed"));
            teste.setColor( QPalette::Normal, QPalette::Window, QColor(0,255,0,150));
            notifyL->setPalette(teste);
            notifyL->setAutoFillBackground(true);
            m_notifyT.start(100);
            break;
	case Transaction::UnknownExitStatus :
            notifyL->setText(i18n("Search finished with unknown status"));
            teste.setColor( QPalette::Normal, QPalette::Window, QColor(0,255,0,150));
            notifyL->setPalette(teste);
            notifyL->setAutoFillBackground(true);
            m_notifyT.start(100);
            break;
    }
}

void KpkAddRm::notifyUpdate()
{
    QPalette palleteN(notifyL->palette());
    QColor colorN(palleteN.color(QPalette::Normal, QPalette::Window));
    if ( colorN.alpha() <= 0 ) {
        m_notifyT.stop();
        notifyL->setAutoFillBackground(false);
        notifyF->hide();
    }
    else {
        colorN.setAlpha(colorN.alpha() - 5);
        palleteN.setColor(QPalette::Normal, QPalette::Window, colorN);
        notifyL->setPalette(palleteN);
    }
}

void KpkAddRm::description(PackageKit::Package *p)
{
    //format and show description
    Package::Details *details = p->details();
    QString description;
    description += "<b>" + i18n("Package Name") + ":</b> " + p->name() + "<br />";
    if ( details->license() != Package::UnknownLicense )
        description += "<b>" + i18n("License") + ":</b> " + details->license() + "<br />";
    if ( details->group() != Client::UnknownGroup )
        description += "<b>" + i18n("Group") + ":</b> " +
	KpkStrings::groups( details->group() ) + "<br />";
    if ( !details->description().isEmpty() )
        description += "<b>" + i18n("Details") + ":</b> " + details->description() + "<br />";
    if ( !details->url().isEmpty() )
        description += "<b>" + i18n("Home Page") + ":</b> <a href=\"" + details->url() + "\">" + details->url() + "</a><br />";
    if ( details->size() > 0 )
        description += "<b>" + i18n("Size") + ":</b> " + KGlobal::locale()->formatByteSize( details->size() );
    descriptionKTB->setHtml(description);
}

void KpkAddRm::files(PackageKit::Package *package, const QStringList &files)
{
    Q_UNUSED(package);
    filesPTE->clear();
    for (int i = 0; i < files.size(); ++i)
        filesPTE->appendPlainText(files.at(i));
}

void KpkAddRm::filterMenu(Client::Filters filters)
{
    m_filtersQM = new QMenu(this);
    filtersTB->setMenu(m_filtersQM);
    
    if(!filters.isEmpty()) {

        if ( filters.contains(Client::FilterInstalled)  || filters.contains(Client::FilterNotInstalled) ) {
            // Installed
            QMenu *menuInstalled = new QMenu(i18n("Installed"), m_filtersQM);
            m_filtersQM->addMenu(menuInstalled);
            QActionGroup *installedGroup = new QActionGroup(menuInstalled);
            installedGroup->setExclusive(true);

//             if ( filters.contains(Client::FilterInstalled) ) {
		QAction *installedTrue = new QAction(i18n("Only installed"), installedGroup);
		installedTrue->setCheckable(true);
		m_filtersAction[installedTrue] = Client::FilterInstalled;
		installedGroup->addAction(installedTrue);
		menuInstalled->addAction(installedTrue);
		actions << installedTrue;
// 	    }

//             if ( filters.contains(Client::FilterNotInstalled) ) {
		QAction *installedFalse = new QAction(i18n("Only avaliable"), installedGroup);
		installedFalse->setCheckable(true);
		m_filtersAction[installedFalse] = Client::FilterNotInstalled;
		installedGroup->addAction(installedFalse);
		menuInstalled->addAction(installedFalse);
		actions << installedFalse;
// 	    }

            QAction *installedNone = new QAction(i18n("No filter"), installedGroup);
            installedNone->setCheckable(true);
            installedNone->setChecked(true);
            installedGroup->addAction(installedNone);
            menuInstalled->addAction(installedNone);
            actions << installedNone;
        }
        if ( filters.contains(Client::FilterDevelopment) || filters.contains(Client::FilterNotDevelopment) ) {
            // Development
            QMenu *menuDevelopment = new QMenu(i18n("Development"), m_filtersQM);
            m_filtersQM->addMenu(menuDevelopment);
            QActionGroup *developmentGroup = new QActionGroup(menuDevelopment);
            developmentGroup->setExclusive(true);

//             if ( filters.contains(Client::FilterDevelopment) ) {
		QAction *developmentTrue = new QAction(i18n("Only development"), developmentGroup);
		developmentTrue->setCheckable(true);
		m_filtersAction[developmentTrue] = Client::FilterDevelopment;
		developmentGroup->addAction(developmentTrue);
		menuDevelopment->addAction(developmentTrue);
		actions << developmentTrue;
// 	    }

//             if ( filters.contains(Client::FilterNotDevelopment) ) {
		QAction *developmentFalse = new QAction(i18n("Only end user files"), developmentGroup);
		developmentFalse->setCheckable(true);
		m_filtersAction[developmentFalse] = Client::FilterNotDevelopment;
		developmentGroup->addAction(developmentFalse);
		menuDevelopment->addAction(developmentFalse);
		actions << developmentFalse;
// 	    }

            QAction *developmentNone = new QAction(i18n("No filter"), developmentGroup);
            developmentNone->setCheckable(true);
            developmentNone->setChecked(true);
            developmentGroup->addAction(developmentNone);
            menuDevelopment->addAction(developmentNone);
            actions << developmentNone;
        }
        if ( filters.contains(Client::FilterGui) || filters.contains(Client::FilterNotGui) ) {
            // Graphical
            QMenu *menuGui = new QMenu(i18n("Graphical"), m_filtersQM);
            m_filtersQM->addMenu(menuGui);
            QActionGroup *guiGroup = new QActionGroup(menuGui);
            guiGroup->setExclusive(true);

//             if ( filters.contains(Client::FilterGui) ) {
		QAction *guiTrue = new QAction(i18n("Only graphical"), guiGroup);
		guiTrue->setCheckable(true);
		m_filtersAction[guiTrue] = Client::FilterGui;
		guiGroup->addAction(guiTrue);
		menuGui->addAction(guiTrue);
		actions << guiTrue;
// 	    }

//             if ( filters.contains(Client::FilterNotGui) ) {
		QAction *guiFalse = new QAction(i18n("Only text"), guiGroup);
		guiFalse->setCheckable(true);
		m_filtersAction[guiFalse] = Client::FilterNotGui;
		guiGroup->addAction(guiFalse);
		menuGui->addAction(guiFalse);
		actions << guiFalse;
// 	    }
	    
            QAction *guiNone = new QAction(i18n("No filter"), guiGroup);
            guiNone->setCheckable(true);
            guiNone->setChecked(true);
            guiGroup->addAction(guiNone);
            menuGui->addAction(guiNone);
            actions << guiNone;
        }
        if ( filters.contains(Client::FilterFree) || filters.contains(Client::FilterNotFree) ) {
            // Free
            QMenu *menuFree = new QMenu(i18n("Free"), m_filtersQM);
            m_filtersQM->addMenu(menuFree);
            QActionGroup *freeGroup = new QActionGroup(menuFree);
            freeGroup->setExclusive(true);

//             if ( filters.contains(Client::FilterFree) ) {
		QAction *freeTrue = new QAction(i18n("Only free software"), freeGroup);
		freeTrue->setCheckable(true);
		m_filtersAction[freeTrue] = Client::FilterFree;
		freeGroup->addAction(freeTrue);
		menuFree->addAction(freeTrue);
		actions << freeTrue;
// 	    }

//             if ( filters.contains(Client::FilterNotFree) ) {
		QAction *freeFalse = new QAction(i18n("Only non-free software"), freeGroup);
		freeFalse->setCheckable(true);
		m_filtersAction[freeFalse] = Client::FilterNotFree;
		freeGroup->addAction(freeFalse);
		menuFree->addAction(freeFalse);
		actions << freeFalse;
// 	    }

            QAction *freeNone = new QAction(i18n("No filter"), freeGroup);
            freeNone->setCheckable(true);
            freeNone->setChecked(true);
            freeGroup->addAction(freeNone);
            menuFree->addAction(freeNone);
            actions << freeNone;
        }
        if ( filters.contains(Client::FilterArch) || filters.contains(Client::FilterNotArch) ) {
            // Arch
            QMenu *menuArch = new QMenu(i18n("Architectures"), m_filtersQM);
            m_filtersQM->addMenu(menuArch);
            QActionGroup *archGroup = new QActionGroup(menuArch);
            archGroup->setExclusive(true);

//             if ( filters.contains(Client::FilterArch) ) {
		QAction *archTrue = new QAction(i18n("Only native architectures"), archGroup);
		archTrue->setCheckable(true);
		m_filtersAction[archTrue] = Client::FilterArch;
		archGroup->addAction(archTrue);
		menuArch->addAction(archTrue);
		actions << archTrue;
// 	    }

//             if ( filters.contains(Client::FilterNotArch) ) {
		QAction *archFalse = new QAction(i18n("Only non-native architectures"), archGroup);
		archFalse->setCheckable(true);
		m_filtersAction[archFalse] = Client::FilterNotArch;
		archGroup->addAction(archFalse);
		menuArch->addAction(archFalse);
		actions << archFalse;
// 	    }

            QAction *archNone = new QAction(i18n("No filter"), archGroup);
            archNone->setCheckable(true);
            archNone->setChecked(true);
            archGroup->addAction(archNone);
            menuArch->addAction(archNone);
            actions << archNone;
        }
        if ( filters.contains(Client::FilterSource) || filters.contains(Client::FilterNotSource) ) {
            // Source
            QMenu *menuSource = new QMenu(i18n("Source"), m_filtersQM);
            m_filtersQM->addMenu(menuSource);
            QActionGroup *sourceGroup = new QActionGroup(menuSource);
            sourceGroup->setExclusive(true);

//             if ( filters.contains(Client::FilterSource) ) {
		QAction *sourceTrue = new QAction(i18n("Only sourcecode"), sourceGroup);
		sourceTrue->setCheckable(true);
		m_filtersAction[sourceTrue] = Client::FilterSource;
		sourceGroup->addAction(sourceTrue);
		menuSource->addAction(sourceTrue);
		actions << sourceTrue;
// 	    }

//             if ( filters.contains(Client::FilterNotSource) ) {
		QAction *sourceFalse = new QAction(i18n("Only non-sourcecode"), sourceGroup);
		sourceFalse->setCheckable(true);
		m_filtersAction[sourceFalse] = Client::FilterNotSource;
		sourceGroup->addAction(sourceFalse);
		menuSource->addAction(sourceFalse);
		actions << sourceFalse;
// 	    }

            QAction *sourceNone = new QAction(i18n("No filter"), sourceGroup);
            sourceNone->setCheckable(true);
            sourceNone->setChecked(true);
            sourceGroup->addAction(sourceNone);
            menuSource->addAction(sourceNone);
            actions << sourceNone;
        }
        if (filters.contains(Client::FilterBasename)) {
            m_filtersQM->addSeparator();
            QAction *basename = new QAction(i18n("Hide subpackages"), m_filtersQM);
            basename->setCheckable(true);
	    basename->setToolTip( i18n("Only show one package, not subpackages") );
	    m_filtersAction[basename] = Client::FilterBasename;
            m_filtersQM->addAction(basename);

            actions << basename;
        }
        if (filters.contains(Client::FilterNewest)) {
            m_filtersQM->addSeparator();
            QAction *newest = new QAction(i18n("Only newest packages"), m_filtersQM);
            newest->setCheckable(true);
	    newest->setToolTip( i18n("Only show the newest available package") );
	    m_filtersAction[newest] = Client::FilterNewest;
            m_filtersQM->addAction(newest);

            actions << newest;
        }
        
        m_filtersQM->addSeparator();
    }
    else {
        //filtersTB->setDisabled(true);
    }
    QAction *groupResults = new QAction(i18n("View in groups"), m_filtersQM);
    groupResults->setCheckable(true);
    m_filtersQM->addAction(groupResults);
    groupResults->setToolTip( i18n("Display packages in groups according to status") );
    connect(groupResults, SIGNAL( toggled(bool) ), m_pkg_model_main, SLOT( setGrouped(bool) ) );
    connect(groupResults, SIGNAL( toggled(bool) ), this, SLOT( packageViewSetRootIsDecorated(bool) ) );
}

void KpkAddRm::packageViewSetRootIsDecorated(bool value)
{
    packageView->setRootIsDecorated(value);
}

Client::Filters KpkAddRm::filters()
{
    Client::Filters buffer;
    for(int i = 0 ; i < actions.size() ; ++i) {
        if( actions.at(i)->isChecked() )
            if( m_filtersAction.contains( actions.at(i) ))
                buffer << m_filtersAction[ actions.at(i) ];
    }
    if(buffer.size() == 0) buffer << Client::NoFilter;
        return m_searchFilters = buffer;
}

#include "KpkAddRm.moc"
