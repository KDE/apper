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

#include "KpkPackageModel.h"

#include <KGenericFactory>
#include <KAboutData>

#include <version.h>

#include "KpkFiltersMenu.h"
#include "KpkPackageDetails.h"
#include "BrowseView.h"
#include "CategoryModel.h"

#include <KLocale>
#include <KStandardDirs>
#include <KMessageBox>
#include <KFileItemDelegate>
#include <KCategorizedSortFilterProxyModel>
#include <khtmlview.h>
#include <khtml_part.h>
// #include <KUrl>

#include <QPalette>
#include <QColor>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <KpkReviewChanges.h>
#include <KpkPackageModel.h>
#include <KpkDelegate.h>
#include <KpkStrings.h>
#include <KpkIcons.h>

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
                               ki18n("Add and Remove Software"),
                               KPK_VERSION,
                               ki18n("KDE interface for managing software"),
                               KAboutData::License_GPL,
                               ki18n("(C) 2008-2010 Daniel Nicoletti"));
    setAboutData(aboutData);
    setButtons(KCModule::Help | Apply);
    KGlobal::locale()->insertCatalog("kpackagekit");

    setupUi(this);

#ifdef HAVE_APPINSTALL
    // load all the data in memory since quering SQLITE is really slow
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "app-install");
    db.setDatabaseName("/home/daniel/code/os/app-install/share/desktop.db");
    db.open();
    QSqlQuery query(db);
    query.prepare(
        "SELECT "
            "a.package_name, "
            "COALESCE(t.application_name, a.application_name), "
            "COALESCE(t.application_summary, a.application_summary), "
            "a.icon_name, "
            "a.application_id "
        "FROM "
            "applications a "
        "LEFT JOIN "
            "translations t "
        "ON "
            "a.application_id = t.application_id "
          "AND "
            "t.locale = :locale");
    query.bindValue(":name", KGlobal::locale()->language());
    QHash<QString, QStringList> *appInstall;
    appInstall = new QHash<QString, QStringList>();
    if (query.exec()) {
        while (query.next()) {
            appInstall->insertMulti(query.value(APP_PKG_NAME).toString(),
                                    QStringList()
                                        << query.value(APP_NAME).toString()
                                        << query.value(APP_SUMMARY).toString()
                                        << query.value(APP_ICON).toString()
                                        << query.value(APP_ID).toString());
        }
    }
#endif //HAVE_APPINSTALL

    // Create a new daemon
    m_client = Client::instance();
    QString locale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
    m_client->setHints("locale=" + locale);
    // store the actions supported by the backend
    m_roles = m_client->actions();

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
//     m_pkgDetails = new PackageDetails(this);
    m_viewLayout->addWidget(homeView);
    m_viewLayout->addWidget(m_browseView);
    m_viewLayout->addWidget(changesView);
//     m_viewLayout->addWidget(m_pkgDetails);

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
    filtersTB->setMenu(m_filtersMenu = new KpkFiltersMenu(m_client->filters(), this));
    filtersTB->setIcon(KIcon("view-filter"));
    m_browseView->proxy()->setFilterFixedString(m_filtersMenu->filterApplications());
    connect(m_filtersMenu, SIGNAL(filterApplications(const QString &)),
            m_browseView->proxy(), SLOT(setFilterFixedString(const QString &)));


    //initialize the model, delegate, client and  connect it's signals
//     setupView(&m_browseModel, packageView);
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

    // Connect this signal anyway so users that have backend that
    // do not support install or remove can be informed properly
    connect(m_changesModel, SIGNAL(rowsInserted(const QModelIndex, int, int)),
            this, SLOT(checkChanged()));
    connect(m_changesModel, SIGNAL(rowsRemoved(const QModelIndex, int, int)),
            this, SLOT(checkChanged()));

    // Make the models talk to each other
    // packageCheced from browse model
    connect(m_browseModel, SIGNAL(packageChecked(const QSharedPointer<PackageKit::Package> &)),
            m_changesModel, SLOT(addSelectedPackage(const QSharedPointer<PackageKit::Package> &)));

    // packageUnchecked from browse model
    connect(m_browseModel, SIGNAL(packageUnchecked(const QSharedPointer<PackageKit::Package> &)),
            m_changesModel, SLOT(uncheckPackage(const QSharedPointer<PackageKit::Package> &)));
    connect(m_browseModel, SIGNAL(packageUnchecked(const QSharedPointer<PackageKit::Package> &)),
            m_changesModel, SLOT(rmSelectedPackage(const QSharedPointer<PackageKit::Package> &)));

    // packageUnchecked from changes model
    connect(m_changesModel, SIGNAL(packageUnchecked(const QSharedPointer<PackageKit::Package> &)),
            m_changesModel, SLOT(rmSelectedPackage(const QSharedPointer<PackageKit::Package> &)));
    connect(m_changesModel, SIGNAL(packageUnchecked(const QSharedPointer<PackageKit::Package> &)),
            m_browseModel, SLOT(uncheckPackage(const QSharedPointer<PackageKit::Package> &)));

    // colapse package description when removing rows
//     connect(m_changesModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
//             this, SLOT(rowsAboutToBeRemoved(const QModelIndex &, int, int)));

#ifdef HAVE_APPINSTALL
    m_browseModel->setAppInstallData(appInstall, true);
    m_changesModel->setAppInstallData(appInstall, false);
#endif //HAVE_APPINSTALL
}

void AddRmKCM::setupView(KpkPackageModel **model, QTreeView *view)
{
    *model = new KpkPackageModel(this, view);
    KCategorizedSortFilterProxyModel *proxyModel = new KCategorizedSortFilterProxyModel(this);
    proxyModel->setSourceModel(*model);
    proxyModel->setDynamicSortFilter(true);
    proxyModel->setCategorizedModel(true);
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setSortRole(KpkPackageModel::SortRole);
    view->setModel(proxyModel);
    view->sortByColumn(0, Qt::AscendingOrder);
    view->header()->setDefaultAlignment(Qt::AlignCenter);
    KpkDelegate *delegate = new KpkDelegate(view);
    view->setItemDelegate(delegate);
    connect(delegate, SIGNAL(showExtendItem(const QModelIndex &)),
            this, SLOT(showExtendItem(const QModelIndex &)));
    connect(proxyModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)),
            this, SLOT(rowsAboutToBeRemoved(const QModelIndex &, int, int)));
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
    int size = m_changesModel->rowCount();
    if (size > 0) {
//         tabWidget->setTabText(2, i18np("1 Change Pending", "%1 Changes Pending", size));
        emit changed(true);
    } else {
//         tabWidget->setTabText(2, i18n("No Change Pending"));
        emit changed(false);
    }
}

void AddRmKCM::showExtendItem(const QModelIndex &index)
{
    if (index.column() == 0) {
        QSharedPointer<PackageKit::Package> package = m_browseModel->package(index);
        if (package) {
//             m_viewLayout->setCurrentWidget(m_pkgDetails);
//             m_pkgDetails->setPackage(package,
//                                      index.data(KpkPackageModel::NameRole).toString(),
//                                      index.data(KpkPackageModel::SummaryRole).toString(),
//                                      index.data(KpkPackageModel::IconPathRole).toString());
        }
    }
}

void AddRmKCM::rowsAboutToBeRemoved(const QModelIndex &index, int start, int end)
{
    Q_UNUSED(end)
    Q_UNUSED(index)
    // If the item is going to be removed and it's extend item is expanded
    // we need to contract it in order to don't show it parent less
    QAbstractItemModel *model;
    model = qobject_cast<QAbstractItemModel*>(sender());
    QModelIndex removingIndex = model->index(start, 0);
    KpkDelegate *delegate = qobject_cast<KpkDelegate*>(changesView->itemDelegate());
    delegate->contractItem(removingIndex);
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
    if (m_searchTransaction) {
        m_searchTransaction->cancel();
    } else if (!searchKLE->text().isEmpty()) {
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
    if (m_searchTransaction) {
        m_searchTransaction->cancel();
    } else if (!searchKLE->text().isEmpty()) {
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
    if (m_searchTransaction) {
        m_searchTransaction->cancel();
    } else if (!searchKLE->text().isEmpty()) {
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
            const QSortFilterProxyModel *proxy = qobject_cast<const QSortFilterProxyModel*>(index.model());
            // If the cast failed it's the index came from browseView
            if (proxy) {
                m_searchParentCategory = proxy->mapToSource(index);
            } else {
                m_searchParentCategory = index;
            }
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
    }
    m_viewLayout->setCurrentIndex(0);
    backTB->setEnabled(false);
    // reset the search role
    m_searchRole = Enum::UnknownRole;
}

void AddRmKCM::search()
{
    m_browseView->hideCategory();

    // search
    switch (m_searchRole) {
    case Enum::RoleSearchName:
        m_searchTransaction = m_client->searchNames(m_searchString, m_searchFilters);
        break;
    case Enum::RoleSearchDetails:
        m_searchTransaction = m_client->searchDetails(m_searchString, m_searchFilters);
        break;
    case Enum::RoleSearchFile:
        m_searchTransaction = m_client->searchFiles(m_searchString, m_searchFilters);
        break;
    case Enum::RoleSearchGroup:
        m_searchTransaction = m_client->searchGroups(m_searchGroup, m_searchFilters);
        break;
    case Enum::RoleGetPackages:
        // we want all the installed ones
        m_browseView->disableExportInstalledPB();
        m_searchTransaction = m_client->getPackages(Enum::FilterInstalled);
        connect(m_searchTransaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                m_browseView, SLOT(enableExportInstalledPB()));
        break;
    case Enum::RoleResolve:
    {
        QSqlDatabase db = QSqlDatabase::database("app-install");
        QSqlQuery query(db);
        query.prepare("SELECT package_name FROM applications WHERE " + m_searchString);
        QStringList packages;
        if (query.exec()) {
            while (query.next()) {
                packages << query.value(0).toString();
            }
            m_browseView->setParentCategory(m_searchParentCategory);
            m_searchTransaction = m_client->resolve(packages, m_searchFilters);
        } else {
            return;
        }
        break;
    }
    default:
        kDebug() << "Search type not defined yet";
        return;
    }
    m_browseView->showInstalledPanel(m_searchRole == Enum::RoleGetPackages);

    m_viewLayout->setCurrentIndex(1);
    backTB->setEnabled(true);

    if (m_searchTransaction->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(m_searchTransaction->error()));
        setCurrentActionEnabled(true);
        m_searchTransaction = 0;
    } else {
        setCurrentActionCancel(true);
        connectTransaction(m_searchTransaction, m_browseModel);
        connect(m_searchTransaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                this, SLOT(finished(PackageKit::Enum::Exit, uint)));
        setCurrentActionEnabled(m_searchTransaction->allowCancel());
        // contract and delete and details widgets
//         KpkDelegate *delegate = qobject_cast<KpkDelegate*>(packageView->itemDelegate());
//         delegate->contractAll();
        // cleans the models
        m_browseModel->clear();
    }
}

void AddRmKCM::connectTransaction(Transaction *transaction, KpkPackageModel *model)
{
    connect(transaction, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
            model, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
    connect(transaction, SIGNAL(errorCode(PackageKit::Enum::Error, const QString &)),
            this, SLOT(errorCode(PackageKit::Enum::Error, const QString &)));
}

void AddRmKCM::changed()
{
    Transaction *trans = qobject_cast<Transaction*>(sender());
    setCurrentActionEnabled(trans->allowCancel());
}

void AddRmKCM::save()
{
    QPointer<KpkReviewChanges> frm = new KpkReviewChanges(m_changesModel->selectedPackages(), this);
    frm->exec();

    // This avoid crashing as the above function does not always quit it's event loop
    if (!frm.isNull()) {
        delete frm;
        // The database might have changed
        m_changesModel->resolveSelected();
        search();
        QTimer::singleShot(0, this, SLOT(checkChanged()));
    }
}

void AddRmKCM::load()
{
    // set focus on the search lineEdit
    searchKLE->setFocus(Qt::OtherFocusReason);
    m_changesModel->setAllChecked(false);
}

void AddRmKCM::finished(PackageKit::Enum::Exit status, uint runtime)
{
    Q_UNUSED(runtime)
    Q_UNUSED(status)
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
        (event->key() == Qt::Key_Return ||
         event->key() == Qt::Key_Enter)) {
        // special tab handling here
        m_currentAction->trigger();
        return;
    }
    KCModule::keyPressEvent(event);
}

#include "AddRmKCM.moc"
