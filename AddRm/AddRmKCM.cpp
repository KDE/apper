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

#include "AddRmKCM.h"


#include <KGenericFactory>
#include <KAboutData>

#include <version.h>

#include "KpkFiltersMenu.h"
#include "KpkPackageDetails.h"
#include "BrowseView.h"
#include "CategoryModel.h"
#include "TransactionHistory.h"

#include <KLocale>
#include <KStandardDirs>
#include <KMessageBox>
#include <KFileItemDelegate>
#include <KCategorizedSortFilterProxyModel>

#include <QPalette>
#include <QColor>

#include <KpkPackageModel.h>
#include <KpkReviewChanges.h>
#include <KpkPackageModel.h>
#include <KpkDelegate.h>
#include <KpkStrings.h>
#include <KpkIcons.h>
#include <AppInstall.h>

#include <KDebug>

#define APP_PKG_NAME 0
#define APP_NAME     1
#define APP_SUMMARY  2
#define APP_ICON     3
#define APP_ID       4

KCONFIGGROUP_DECLARE_ENUM_QOBJECT(Enum, Filter)

K_PLUGIN_FACTORY(KPackageKitFactory, registerPlugin<AddRmKCM>();)
K_EXPORT_PLUGIN(KPackageKitFactory("kcm_kpk_addrm"))

AddRmKCM::AddRmKCM(QWidget *parent, const QVariantList &args)
 : KCModule(KPackageKitFactory::componentData(), parent, args),
   m_currentAction(0),
   m_searchTransaction(0),
   m_findIcon("edit-find"),
   m_cancelIcon("dialog-cancel"),
   m_searchRole(Enum::UnknownRole)
{
    KAboutData *aboutData;
    aboutData = new KAboutData("kpackagekit",
                               "kpackagekit",
                               ki18n("Get and Remove Software"),
                               KPK_VERSION,
                               ki18n("KDE interface for managing software"),
                               KAboutData::License_GPL,
                               ki18n("(C) 2008-2010 Daniel Nicoletti"));
    setAboutData(aboutData);
    setButtons(KCModule::Help | Apply);
    KGlobal::locale()->insertCatalog("kpackagekit");

    setupUi(this);

    // Set the current locale
    QString locale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
    Client::instance()->setHints("locale=" + locale);
    // store the actions supported by the backend
    m_roles = Client::instance()->actions();

    // Browse TAB
    backTB->setIcon(KIcon("go-previous"));

    // create our toolbar
    QToolBar *toolBar = new QToolBar(this);
    gridLayout_2->addWidget(toolBar);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    m_browseView = new BrowseView(this);
    connect(m_browseView, SIGNAL(categoryActivated(const QModelIndex &)),
            this, SLOT(on_homeView_activated(const QModelIndex &)));

    // Create a stacked layout so only homeView or packageView are displayed
    m_viewLayout = new QStackedLayout(stackedWidget);
    m_viewLayout->addWidget(homeView);
    m_viewLayout->addWidget(m_browseView);
    m_viewLayout->addWidget(changesView);

    QMenu *findMenu = new QMenu(this);
    // find is just a generic name in case we don't have any search method
    m_genericActionK = new KToolBarPopupAction(m_findIcon, i18n("Find"), this);
    toolBar->addAction(m_genericActionK);

    // Add actions that the backend supports
    if (m_roles & Enum::RoleSearchName) {
        findMenu->addAction(actionFindName);
        setCurrentAction(actionFindName);
    }
    if (m_roles & Enum::RoleSearchDetails) {
        findMenu->addAction(actionFindDescription);
        if (!m_currentAction) {
            setCurrentAction(actionFindDescription);
        }
    }
    if (m_roles & Enum::RoleSearchFile) {
        findMenu->addAction(actionFindFile);
        if (!m_currentAction) {
            setCurrentAction(actionFindFile);
        }
    }

    // If no action was set we can't use this search
    if (m_currentAction == 0) {
        m_genericActionK->setEnabled(false);
        searchKLE->setEnabled(false);
    } else {
        // Check to see if we need the KToolBarPopupAction
        setCurrentActionCancel(false);
        if (findMenu->actions().size() > 1) {
            m_currentAction->setVisible(false);
            m_genericActionK->setMenu(findMenu);
        } else {
            m_currentAction->setVisible(true);
            toolBar->removeAction(m_genericActionK);
            toolBar->addAction(m_currentAction);
        }
        connect(m_genericActionK, SIGNAL(triggered()),
                this, SLOT(genericActionKTriggered()));
    }

    // Create the groups model
    m_groupsModel = new CategoryModel(this);

    homeView->setSpacing(KDialog::spacingHint());
    homeView->viewport()->setAttribute(Qt::WA_Hover);

    m_browseView->setCategoryModel(m_groupsModel);

    KFileItemDelegate *delegate = new KFileItemDelegate(this);
    delegate->setWrapMode(QTextOption::WordWrap);
    homeView->setItemDelegate(delegate);

    KCategorizedSortFilterProxyModel *proxy = new KCategorizedSortFilterProxyModel(this);
    proxy->setSourceModel(m_groupsModel);
    proxy->setCategorizedModel(true);
    proxy->sort(0);
    homeView->setModel(proxy);

    // install the backend filters
    filtersTB->setMenu(m_filtersMenu = new KpkFiltersMenu(Client::instance()->filters(), this));
    filtersTB->setIcon(KIcon("view-filter"));
    m_browseView->proxy()->setFilterFixedString(m_filtersMenu->filterApplications());
    connect(m_filtersMenu, SIGNAL(filterApplications(const QString &)),
            m_browseView->proxy(), SLOT(setFilterFixedString(const QString &)));


    //initialize the model, delegate, client and  connect it's signals
    m_browseModel = m_browseView->model();

    // CHANGES TAB
    changesView->viewport()->setAttribute(Qt::WA_Hover);
    m_changesModel = new KpkPackageModel(this, changesView);
    KCategorizedSortFilterProxyModel *changedProxy = new KCategorizedSortFilterProxyModel(this);
    changedProxy->setSourceModel(m_changesModel);
    changedProxy->setCategorizedModel(true);
    changedProxy->sort(0);
    changedProxy->setDynamicSortFilter(true);
    changedProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    changedProxy->setSortRole(KpkPackageModel::SortRole);
    changesView->setModel(changedProxy);
    KpkDelegate *changesDelegate = new KpkDelegate(changesView);
    changesDelegate->setExtendPixmapWidth(0);
    changesView->setItemDelegate(changesDelegate);

    // Connect this signal to keep track of changes
    connect(m_browseModel, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
    connect(m_browseModel, SIGNAL(changed(bool)), changesPB, SLOT(setEnabled(bool)));

    // packageUnchecked from changes model
    connect(m_changesModel, SIGNAL(packageUnchecked(const KpkPackageModel::InternalPackage &)),
            m_changesModel, SLOT(rmSelectedPackage(const KpkPackageModel::InternalPackage &)));
    connect(m_changesModel, SIGNAL(packageUnchecked(const KpkPackageModel::InternalPackage &)),
            m_browseModel, SLOT(uncheckPackage(const KpkPackageModel::InternalPackage &)));

#ifdef HAVE_APPINSTALL
//     m_browseModel->setAppInstallData(appInstall, true);
//     m_changesModel->setAppInstallData(appInstall, false);
#endif //HAVE_APPINSTALL

    changesPB->setIcon(KIcon("edit-redo"));
}

void AddRmKCM::genericActionKTriggered()
{
    m_currentAction->trigger();
}

void AddRmKCM::setCurrentAction(QAction *action)
{
    // just load the new action if it changes this
    // also ensures that our menu has more than one action
    if (m_currentAction != action) {
        // hides the item from the list
        action->setVisible(false);
        // ensures the current action was created
        if (m_currentAction) {
            // show the item back in the list
            m_currentAction->setVisible(true);
        }
        m_currentAction = action;
        // copy data from the curront action
        m_genericActionK->setText(m_currentAction->text());
        m_genericActionK->setIcon(m_currentAction->icon());
    }
}

void AddRmKCM::setCurrentActionEnabled(bool state)
{
    if (m_currentAction) {
        m_currentAction->setEnabled(state);
    }
    m_genericActionK->setEnabled(state);
}

void AddRmKCM::setCurrentActionCancel(bool cancel)
{
    if (cancel) {
        // every action should like cancel
        actionFindName->setText(i18n("&Cancel"));
        actionFindFile->setText(i18n("&Cancel"));
        actionFindDescription->setText(i18n("&Cancel"));
        m_genericActionK->setText(i18n("&Cancel"));
        // set cancel icons
        actionFindFile->setIcon(m_cancelIcon);
        actionFindDescription->setIcon(m_cancelIcon);
        actionFindName->setIcon(m_cancelIcon);
        m_genericActionK->setIcon(m_cancelIcon);
    } else {
        actionFindName->setText(i18n("Find by &name"));
        actionFindFile->setText(i18n("Find by f&ile name"));
        actionFindDescription->setText(i18n("Find by &description"));
        // Define actions icon
        actionFindFile->setIcon(KIcon("document-open"));
        actionFindDescription->setIcon(KIcon("document-edit"));
        actionFindName->setIcon(m_findIcon);
        m_genericActionK->setIcon(m_findIcon);
        if (m_currentAction) {
            m_genericActionK->setText(m_currentAction->text());
        } else {
            // This might happen when the backend can
            // only search groups
            m_genericActionK->setText(i18n("Find"));
        }
    }
}

void AddRmKCM::checkChanged()
{
    bool value = m_browseModel->hasChanges();
    changesPB->setEnabled(value);
    emit changed(value);
}

void AddRmKCM::errorCode(PackageKit::Enum::Error error, const QString &details)
{
    if (error != Enum::ErrorTransactionCancelled) {
        KMessageBox::detailedSorry(this, KpkStrings::errorMessage(error), details, KpkStrings::error(error), KMessageBox::Notify);
    }
}

AddRmKCM::~AddRmKCM()
{
}

void AddRmKCM::on_actionFindName_triggered()
{
    setCurrentAction(actionFindName);
    if (!searchKLE->text().isEmpty()) {
        // cache the search
        m_searchRole    = Enum::RoleSearchName;
        m_searchString  = searchKLE->text();
        m_searchFilters = m_filtersMenu->filters();
        // create the main transaction
        search();
    }
}

void AddRmKCM::on_actionFindDescription_triggered()
{
    setCurrentAction(actionFindDescription);
    if (!searchKLE->text().isEmpty()) {
        // cache the search
        m_searchRole    = Enum::RoleSearchDetails;
        m_searchString  = searchKLE->text();
        m_searchFilters = m_filtersMenu->filters();
        // create the main transaction
        search();
    }
}

void AddRmKCM::on_actionFindFile_triggered()
{
    setCurrentAction(actionFindFile);
    if (!searchKLE->text().isEmpty()) {
        // cache the search
        m_searchRole    = Enum::RoleSearchFile;
        m_searchString  = searchKLE->text();
        m_searchFilters = m_filtersMenu->filters();
        // create the main transaction
        search();
    }
}

void AddRmKCM::on_homeView_activated(const QModelIndex &index)
{
    if (index.isValid()) {
        // cache the search
        m_searchRole    = static_cast<Enum::Role>(index.data(CategoryModel::SearchRole).toUInt());
        m_searchGroup   = static_cast<Enum::Group>(index.data(CategoryModel::GroupRole).toUInt());
        m_searchFilters = m_filtersMenu->filters();
        if (m_searchRole == Enum::RoleResolve) {
            m_searchString = index.data(CategoryModel::CategoryRole).toString();
            const QSortFilterProxyModel *proxy;
            proxy = qobject_cast<const QSortFilterProxyModel*>(index.model());
            // If the cast failed it's the index came from browseView
            if (proxy) {
                m_searchParentCategory = proxy->mapToSource(index);
            } else {
                m_searchParentCategory = index;
            }
        } else if (m_searchRole == Enum::RoleGetOldTransactions) {
            m_history = new TransactionHistory(this);
            searchKLE->clear();
            connect(searchKLE, SIGNAL(textChanged(const QString &)),
                    m_history, SLOT(setFilterRegExp(const QString &)));
            m_viewLayout->addWidget(m_history);
            m_viewLayout->setCurrentWidget(m_history);
            backTB->setEnabled(true);
            filtersTB->setEnabled(false);
            widget->setEnabled(false);
            return;
        }
        // create the main transaction
        search();
    }
}

void AddRmKCM::on_backTB_clicked()
{
    if (m_viewLayout->currentWidget() == m_browseView) {
        if (!m_browseView->goBack()) {
            return;
        }
    } else if (m_viewLayout->currentWidget() == m_history) {
        filtersTB->setEnabled(true);
        widget->setEnabled(true);
        delete m_history;
    }
    m_viewLayout->setCurrentIndex(0);
    backTB->setEnabled(false);
    // reset the search role
    m_searchRole = Enum::UnknownRole;
}

void AddRmKCM::on_changesPB_clicked()
{
    m_changesModel->clear();
    m_changesModel->addPackages(m_browseModel->selectedPackages(), true);
    m_viewLayout->setCurrentWidget(changesView);
    backTB->setEnabled(true);
}

void AddRmKCM::search()
{
    m_browseView->cleanUi();

    if (m_searchTransaction) {
        // Disconnect everything so that the model don't store
        // wrong data
        m_searchTransaction->cancel();
        disconnect(m_searchTransaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                   m_browseView->busyCursor(), SLOT(stop()));
        disconnect(m_searchTransaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                   this, SLOT(finished()));
        disconnect(m_searchTransaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                   m_browseModel, SLOT(finished()));
        disconnect(m_searchTransaction, SIGNAL(package(const QSharedPointer<PackageKit::Package> &)),
                   m_browseModel, SLOT(addPackage(const QSharedPointer<PackageKit::Package> &)));
        disconnect(m_searchTransaction, SIGNAL(errorCode(PackageKit::Enum::Error, const QString &)),
                   this, SLOT(errorCode(PackageKit::Enum::Error, const QString &)));
    }

    // search
    m_searchTransaction = new Transaction(QString());
    connect(m_searchTransaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            m_browseView->busyCursor(), SLOT(stop()));
    connect(m_searchTransaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            this, SLOT(finished()));
    connect(m_searchTransaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            m_browseModel, SLOT(finished()));
    connect(m_searchTransaction, SIGNAL(package(const QSharedPointer<PackageKit::Package> &)),
            m_browseModel, SLOT(addPackage(const QSharedPointer<PackageKit::Package> &)));
    connect(m_searchTransaction, SIGNAL(errorCode(PackageKit::Enum::Error, const QString &)),
            this, SLOT(errorCode(PackageKit::Enum::Error, const QString &)));
    switch (m_searchRole) {
    case Enum::RoleSearchName:
        m_searchTransaction->searchNames(m_searchString, m_searchFilters);
        break;
    case Enum::RoleSearchDetails:
        m_searchTransaction->searchDetails(m_searchString, m_searchFilters);
        break;
    case Enum::RoleSearchFile:
        m_searchTransaction->searchFiles(m_searchString, m_searchFilters);
        break;
    case Enum::RoleSearchGroup:
        m_searchTransaction->searchGroups(m_searchGroup, m_searchFilters);
        break;
    case Enum::RoleGetPackages:
        // we want all the installed ones
        m_browseView->disableExportInstalledPB();
        connect(m_searchTransaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                m_browseView, SLOT(enableExportInstalledPB()));
        m_searchTransaction->getPackages(Enum::FilterInstalled);
        break;
    case Enum::RoleResolve:
    {
        QStringList packages = AppInstall::instance()->pkgNamesFromWhere(m_searchString);
        if (!packages.isEmpty()) {
            m_browseView->setParentCategory(m_searchParentCategory);
            // WARNING the resolve might fail if the backend
            // has a low limit MaximumItemsToResolve
            m_searchTransaction->resolve(packages, m_searchFilters);
        } else {
            return;
        }
        break;
    }
    default:
        kDebug() << "Search type not defined yet";
        return;
    }

    if (m_searchTransaction->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(m_searchTransaction->error()));
        setCurrentActionEnabled(true);
        m_searchTransaction = 0;
    } else {
        // cleans the models
        m_browseModel->clear();

        m_browseView->showInstalledPanel(m_searchRole == Enum::RoleGetPackages);
        m_browseView->busyCursor()->start();

        backTB->setEnabled(true);
        setCurrentActionCancel(true);
        setCurrentActionEnabled(m_searchTransaction->allowCancel());

        m_viewLayout->setCurrentIndex(1);
    }
}

void AddRmKCM::changed()
{
    Transaction *trans = qobject_cast<Transaction*>(sender());
    setCurrentActionEnabled(trans->allowCancel());
}

void AddRmKCM::save()
{
    QPointer<KpkReviewChanges> frm = new KpkReviewChanges(m_browseModel->selectedPackages(), this);
    connect(frm, SIGNAL(successfullyInstalled()), m_browseModel, SLOT(uncheckAvailablePackages()));
    connect(frm, SIGNAL(successfullyRemoved()), m_browseModel, SLOT(uncheckInstalledPackages()));

    frm->exec();

    // This avoid crashing as the above function does not always quit it's event loop
    if (!frm.isNull()) {
        delete frm;

        search();
        QTimer::singleShot(0, this, SLOT(checkChanged()));
    }
}

void AddRmKCM::load()
{
    // set focus on the search lineEdit
    searchKLE->setFocus(Qt::OtherFocusReason);
    m_browseModel->setAllChecked(false);
}

void AddRmKCM::finished()
{
    // if m_currentAction is false means that our
    // find button should be disable as there aren't any
    // search methods
    setCurrentActionEnabled(m_currentAction);
    setCurrentActionCancel(false);
    m_searchTransaction = 0;
}

void AddRmKCM::keyPressEvent(QKeyEvent *event)
{
    if (searchKLE->hasFocus() &&
        m_viewLayout->currentWidget() != m_history &&
        (event->key() == Qt::Key_Return ||
         event->key() == Qt::Key_Enter)) {
        // special tab handling here
        m_currentAction->trigger();
        return;
    }
    KCModule::keyPressEvent(event);
}

#include "AddRmKCM.moc"
