/***************************************************************************
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
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

#include "ApperKCM.h"
#include "ui_ApperKCM.h"

#include <config.h>

#include <KGenericFactory>
#include <KAboutData>

#include <KLocalizedString>
#include <KStandardDirs>
#include <KMessageBox>
#include <KFileItemDelegate>
#include <KMenu>
#include <KHelpMenu>
#include <KTabBar>
#include <KCmdLineArgs>
#include <QToolBar>

#include <PackageModel.h>
#include <ApplicationSortFilterModel.h>
#include <ChangesDelegate.h>
#include <PkStrings.h>
#include <PkIcons.h>
#include <PkTransactionWidget.h>

#ifdef HAVE_APPSTREAM
#include <AppStream.h>
#endif

#include <KDebug>
#include <Daemon>

#include "FiltersMenu.h"
#include "BrowseView.h"
#include "CategoryModel.h"
#include "TransactionHistory.h"
#include "Settings/Settings.h"
#include "Updater/Updater.h"

#define BAR_SEARCH   0
#define BAR_UPDATE   1
#define BAR_SETTINGS 2
#define BAR_TITLE    3

KCONFIGGROUP_DECLARE_ENUM_QOBJECT(Transaction, Filter)

K_PLUGIN_FACTORY(ApperFactory, registerPlugin<ApperKCM>();)
K_EXPORT_PLUGIN(ApperFactory("kcm_apper", "apper"))

ApperKCM::ApperKCM(QWidget *parent, const QVariantList &args) :
    KCModule(parent, args),
    ui(new Ui::ApperKCM),
    m_currentAction(0),
    m_groupsProxyModel(0),
    m_settingsPage(0),
    m_updaterPage(0),
    m_searchTransaction(0),
    m_findIcon("edit-find"),
    m_cancelIcon("dialog-cancel"),
    m_forceRefreshCache(false),
    m_cacheAge(600),
    m_history(0),
    m_searchRole(Transaction::RoleUnknown)
{
    KAboutData *aboutData;
    aboutData = new KAboutData("kcm_apper",
                               "apper",
                               APPER_VERSION,
                               i18n("KDE interface for managing software"),
                               KAboutLicense::LicenseKey::GPL);
    aboutData->addAuthor(i18n("(C) 2008-2013 Daniel Nicoletti"), QString(), "dantti12@gmail.com", "http://dantti.wordpress.com");
    aboutData->addAuthor(i18n("Matthias Klumpp"), QString(), QStringLiteral("matthias@tenstral.net"));
    setAboutData(aboutData);
    setButtons(Apply);

    // store the actions supported by the backend
    connect(Daemon::global(), SIGNAL(changed()), this, SLOT(daemonChanged()));

    // Set the current locale
    QString locale(KGlobal::locale()->language() % QLatin1Char('.') % KGlobal::locale()->encoding());
    Daemon::global()->setHints(QLatin1String("locale=") % locale);

    ui->setupUi(this);

    // Browse TAB
    ui->backTB->setIcon(KIcon("go-previous"));

    // create our toolbar
    QToolBar *toolBar = new QToolBar(this);
    ui->gridLayout_2->addWidget(toolBar);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    connect(ui->browseView, SIGNAL(categoryActivated(QModelIndex)),
            this, SLOT(on_homeView_activated(QModelIndex)));

    QMenu *findMenu = new QMenu(this);
    // find is just a generic name in case we don't have any search method
    m_genericActionK = new KToolBarPopupAction(m_findIcon, i18n("Find"), this);
    toolBar->addAction(m_genericActionK);

    // Add actions that the backend supports
    findMenu->addAction(ui->actionFindName);
    setCurrentAction(ui->actionFindName);

    findMenu->addAction(ui->actionFindDescription);
    if (!m_currentAction) {
        setCurrentAction(ui->actionFindDescription);
    }

    findMenu->addAction(ui->actionFindFile);
    if (!m_currentAction) {
        setCurrentAction(ui->actionFindFile);
    }


    // If no action was set we can't use this search
    if (m_currentAction == 0) {
        m_genericActionK->setEnabled(false);
        ui->searchKLE->setEnabled(false);
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
    ui->browseView->setCategoryModel(m_groupsModel);
    connect(m_groupsModel, SIGNAL(finished()),
            this, SLOT(setupHomeModel()));
    ui->homeView->setSpacing(KDialog::spacingHint());
    ui->homeView->viewport()->setAttribute(Qt::WA_Hover);

    KFileItemDelegate *delegate = new KFileItemDelegate(this);
    delegate->setWrapMode(QTextOption::WordWrap);
    ui->homeView->setItemDelegate(delegate);

    // install the backend filters
    ui->filtersTB->setMenu(m_filtersMenu = new FiltersMenu(this));
    connect(m_filtersMenu, SIGNAL(filtersChanged()), this, SLOT(search()));
    ui->filtersTB->setIcon(KIcon("view-filter"));
    ApplicationSortFilterModel *proxy = ui->browseView->proxy();
    proxy->setApplicationFilter(m_filtersMenu->filterApplications());
    connect(m_filtersMenu, SIGNAL(filterApplications(bool)),
            proxy, SLOT(setApplicationFilter(bool)));

    //initialize the model, delegate, client and  connect it's signals
    m_browseModel = ui->browseView->model();

    // CHANGES TAB
    ui->changesView->viewport()->setAttribute(Qt::WA_Hover);
    m_changesModel = new PackageModel(this);
    KCategorizedSortFilterProxyModel *changedProxy = new KCategorizedSortFilterProxyModel(this);
    changedProxy->setSourceModel(m_changesModel);
    changedProxy->setDynamicSortFilter(true);
    changedProxy->setCategorizedModel(true);
    changedProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    changedProxy->setSortRole(PackageModel::SortRole);
    changedProxy->sort(0);
    ui->changesView->setModel(changedProxy);
    ChangesDelegate *changesDelegate = new ChangesDelegate(ui->changesView);
    changesDelegate->setExtendPixmapWidth(0);
    ui->changesView->setItemDelegate(changesDelegate);

    // Connect this signal to keep track of changes
    connect(m_browseModel, SIGNAL(changed(bool)), this, SLOT(checkChanged()));

    // packageUnchecked from changes model
    connect(m_changesModel, SIGNAL(packageUnchecked(QString)),
            m_changesModel, SLOT(removePackage(QString)));
    connect(m_changesModel, SIGNAL(packageUnchecked(QString)),
            m_browseModel, SLOT(uncheckPackage(QString)));

    ui->changesPB->setIcon(KIcon("edit-redo"));

    KMenu *menu = new KMenu(this);
    ui->settingsTB->setMenu(menu);
    ui->settingsTB->setIcon(KIcon("preferences-other"));
    QSignalMapper *signalMapper = new QSignalMapper(this);
    QAction *action;
    action = menu->addAction(KIcon("view-history"), i18n("History"));
    signalMapper->setMapping(action, "history");
    connect(action, SIGNAL(triggered()),
            signalMapper, SLOT(map()));
    connect(signalMapper, SIGNAL(mapped(QString)),
            this, SLOT(setPage(QString)));
    action = menu->addAction(KIcon("preferences-other"), i18n("Settings"));
    signalMapper->setMapping(action, "settings");
    connect(action, SIGNAL(triggered()),
            signalMapper, SLOT(map()));
    connect(signalMapper, SIGNAL(mapped(QString)),
            this, SLOT(setPage(QString)));

    // Only show help menu if not on System Settings
    if (!args.isEmpty()) {
        // adds the help menu
        //! KHelpMenu *helpMenu = new KHelpMenu(this, KGlobal::mainComponent().aboutData());
        //! menu->addMenu(helpMenu->menu());
    }

    // Make sure the search bar is visible
    ui->stackedWidgetBar->setCurrentIndex(BAR_SEARCH);
}

void ApperKCM::setupHomeModel()
{
    KCategorizedSortFilterProxyModel *oldProxy = m_groupsProxyModel;
    m_groupsProxyModel = new KCategorizedSortFilterProxyModel(this);
    m_groupsProxyModel->setSourceModel(m_groupsModel);
    m_groupsProxyModel->setCategorizedModel(true);
    m_groupsProxyModel->sort(0);
    ui->homeView->setModel(m_groupsProxyModel);
    if (oldProxy) {
        oldProxy->deleteLater();
    }
}

void ApperKCM::genericActionKTriggered()
{
    m_currentAction->trigger();
}

void ApperKCM::setCurrentAction(QAction *action)
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

void ApperKCM::setCurrentActionEnabled(bool state)
{
    if (m_currentAction) {
        m_currentAction->setEnabled(state);
    }
    m_genericActionK->setEnabled(state);
}

void ApperKCM::setCurrentActionCancel(bool cancel)
{
    if (cancel) {
        // every action should like cancel
        ui->actionFindName->setText(i18n("&Cancel"));
        ui->actionFindFile->setText(i18n("&Cancel"));
        ui->actionFindDescription->setText(i18n("&Cancel"));
        m_genericActionK->setText(i18n("&Cancel"));
        // set cancel icons
        ui->actionFindFile->setIcon(m_cancelIcon);
        ui->actionFindDescription->setIcon(m_cancelIcon);
        ui->actionFindName->setIcon(m_cancelIcon);
        m_genericActionK->setIcon(m_cancelIcon);
    } else {
        ui->actionFindName->setText(i18n("Find by &name"));
        ui->actionFindFile->setText(i18n("Find by f&ile name"));
        ui->actionFindDescription->setText(i18n("Find by &description"));
        // Define actions icon
        ui->actionFindFile->setIcon(KIcon("document-open"));
        ui->actionFindDescription->setIcon(KIcon("document-edit"));
        ui->actionFindName->setIcon(m_findIcon);
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

void ApperKCM::checkChanged()
{
    bool hasChanges = false;
    if (ui->stackedWidget->currentWidget() == ui->pageHome ||
            ui->stackedWidget->currentWidget() == ui->pageChanges ||
            ui->stackedWidget->currentWidget() == ui->pageBrowse) {
        hasChanges = m_browseModel->hasChanges();
        if (!hasChanges && ui->stackedWidget->currentWidget() == ui->pageChanges) {
            search();
        }
        ui->changesPB->setEnabled(hasChanges);
    } else if (ui->stackedWidget->currentWidget() == m_updaterPage) {
        hasChanges = m_updaterPage->hasChanges();
    } else if (ui->stackedWidget->currentWidget() == m_settingsPage) {
        hasChanges = m_settingsPage->hasChanges();
    }

    emit changed(hasChanges);
}

void ApperKCM::errorCode(PackageKit::Transaction::Error error, const QString &details)
{
    if (error != Transaction::ErrorTransactionCancelled) {
        KMessageBox::detailedSorry(this, PkStrings::errorMessage(error), details, PkStrings::error(error), KMessageBox::Notify);
    }
}

ApperKCM::~ApperKCM()
{
    delete ui;
}

void ApperKCM::daemonChanged()
{
    Transaction::Roles roles = Daemon::roles();
    if (m_roles == roles) {
        return;
    }
    m_roles = roles;

    // Add actions that the backend supports
    ui->actionFindName->setEnabled(roles & Transaction::RoleSearchName);
    ui->actionFindDescription->setEnabled(roles & Transaction::RoleSearchDetails);
    ui->actionFindFile->setEnabled(roles & Transaction::RoleSearchFile);

    ui->browseView->init(roles);

    m_groupsModel->setRoles(roles);

    m_filtersMenu->setFilters(Daemon::filters());
}

void ApperKCM::on_actionFindName_triggered()
{
    setCurrentAction(ui->actionFindName);
    if (!ui->searchKLE->text().isEmpty()) {
        // cache the search
        m_searchRole    = Transaction::RoleSearchName;
        m_searchString  = ui->searchKLE->text();
        // create the main transaction
        search();
    }
}

void ApperKCM::on_actionFindDescription_triggered()
{
    setCurrentAction(ui->actionFindDescription);
    if (!ui->searchKLE->text().isEmpty()) {
        // cache the search
        m_searchRole    = Transaction::RoleSearchDetails;
        m_searchString  = ui->searchKLE->text();
        // create the main transaction
        search();
    }
}

void ApperKCM::on_actionFindFile_triggered()
{
    setCurrentAction(ui->actionFindFile);
    if (!ui->searchKLE->text().isEmpty()) {
        // cache the search
        m_searchRole    = Transaction::RoleSearchFile;
        m_searchString  = ui->searchKLE->text();
        // create the main transaction
        search();
    }
}

void ApperKCM::on_homeView_activated(const QModelIndex &index)
{
    if (index.isValid()) {
        const QSortFilterProxyModel *proxy;
        proxy = qobject_cast<const QSortFilterProxyModel*>(index.model());
        // If the cast failed it's the index came from browseView
        if (proxy) {
            m_searchParentCategory = proxy->mapToSource(index);
        } else {
            m_searchParentCategory = index;
        }

        // cache the search
        m_searchRole = static_cast<Transaction::Role>(index.data(CategoryModel::SearchRole).toUInt());
        kDebug() << m_searchRole << index.data(CategoryModel::CategoryRole).toString();
        if (m_searchRole == Transaction::RoleResolve) {
#ifdef HAVE_APPSTREAM
            CategoryMatcher parser = index.data(CategoryModel::CategoryRole).value<CategoryMatcher>();
            m_searchCategory = AppStream::instance()->findPkgNames(parser);
#endif // HAVE_APPSTREAM
        } else if (m_searchRole == Transaction::RoleSearchGroup) {
            if (index.data(CategoryModel::GroupRole).type() == QVariant::String) {
                QString category = index.data(CategoryModel::GroupRole).toString();
                if (category.startsWith('@') ||
                    (category.startsWith(QLatin1String("repo:")) && category.size() > 5)) {
                    m_searchGroupCategory = category;
                } else {
                    m_groupsModel->setRootIndex(m_searchParentCategory);
                    ui->backTB->setEnabled(true);
                    return;
                }
            } else {
                m_searchGroupCategory.clear();
                int groupRole = index.data(CategoryModel::GroupRole).toInt();
                m_searchGroup = static_cast<PackageKit::Transaction::Group>(groupRole);
                m_searchString = index.data().toString(); // Store the nice name to change the title
            }
        } else if (m_searchRole == Transaction::RoleGetUpdates) {
            setPage("updates");
            return;
        }

        // create the main transaction
        search();
    }
}

bool ApperKCM::canChangePage()
{
    bool changed;
    // Check if we can change the current page
    if (ui->stackedWidget->currentWidget() == m_updaterPage) {
        changed = m_updaterPage->hasChanges();
    } else if (ui->stackedWidget->currentWidget() == m_settingsPage) {
        changed = m_settingsPage->hasChanges();
    } else {
        changed = m_browseModel->hasChanges();
    }

    // if there are no changes don't ask the user
    if (!changed) {
        return true;
    }

    const int queryUser = KMessageBox::warningYesNoCancel(
        this,
        i18n("The settings of the current module have changed.\n"
             "Do you want to apply the changes or discard them?"),
        i18n("Apply Settings"),
        KStandardGuiItem::apply(),
        KStandardGuiItem::discard(),
        KStandardGuiItem::cancel());

    switch (queryUser) {
    case KMessageBox::Yes:
        save();
        return true;
    case KMessageBox::No:
        load();
        return true;
    case KMessageBox::Cancel:
        return false;
    default:
        return false;
    }
}

QString ApperKCM::page() const
{
    return QString();
}

void ApperKCM::setPage(const QString &page)
{
    PkTransaction *transaction = qobject_cast<PkTransaction*>(ui->stackedWidget->currentWidget());
    if (transaction) {
        return;
    }

    if (page == QLatin1String("settings")) {
        if (ui->stackedWidget->currentWidget() != m_settingsPage) {
            if (!canChangePage()) {
                return;
            }

            if (m_settingsPage == 0) {
                m_settingsPage = new Settings(m_roles, this);
                connect(m_settingsPage, SIGNAL(changed(bool)),
                        this, SLOT(checkChanged()));
                connect(m_settingsPage, SIGNAL(refreshCache()),
                        SLOT(refreshCache()));
                ui->stackedWidget->addWidget(m_settingsPage);

                connect(ui->generalSettingsPB, SIGNAL(toggled(bool)),
                        m_settingsPage, SLOT(showGeneralSettings()));
                connect(ui->repoSettingsPB, SIGNAL(toggled(bool)),
                        m_settingsPage, SLOT(showRepoSettings()));
            }
            checkChanged();
            setButtons(KCModule::Default | KCModule::Apply);
            emit changed(true); // THIS IS DUMB setButtons only take effect after changed goes true
            emit changed(false);
            ui->generalSettingsPB->setChecked(true);
            ui->stackedWidgetBar->setCurrentIndex(BAR_SETTINGS);
            ui->stackedWidget->setCurrentWidget(m_settingsPage);
            m_settingsPage->load();
            ui->titleL->clear();
            ui->backTB->setEnabled(true);
            emit caption(i18n("Settings"));
        }
    } else if (page == QLatin1String("updates")) {
        if (ui->stackedWidget->currentWidget() != m_updaterPage) {
            if (!canChangePage()) {
                return;
            }

            if (m_updaterPage == 0) {
                m_updaterPage = new Updater(m_roles, this);
                connect(m_updaterPage, SIGNAL(refreshCache()),
                        this, SLOT(refreshCache()));
                connect(m_updaterPage, SIGNAL(downloadSize(QString)),
                        ui->downloadL, SLOT(setText(QString)));
                connect(m_updaterPage, SIGNAL(changed(bool)),
                        this, SLOT(checkChanged()));
                ui->stackedWidget->addWidget(m_updaterPage);
                ui->checkUpdatesPB->setIcon(KIcon("view-refresh"));
                connect(ui->checkUpdatesPB, SIGNAL(clicked(bool)),
                        this, SLOT(refreshCache()));
            }

            checkChanged();
            ui->stackedWidget->setCurrentWidget(m_updaterPage);
            m_updaterPage->load();
            ui->stackedWidgetBar->setCurrentIndex(BAR_UPDATE);
            ui->backTB->setEnabled(true);
            emit caption(i18n("Updates"));
        }
    } else if (page == QLatin1String("home")) {
        if (ui->stackedWidget->currentWidget() == m_updaterPage ||
            ui->stackedWidget->currentWidget() == m_settingsPage) {
            on_backTB_clicked();
        }
    } else if (page == QLatin1String("history")) {
        m_history = new TransactionHistory(this);
        ui->searchKLE->clear();
        connect(ui->searchKLE, SIGNAL(textChanged(QString)),
                m_history, SLOT(setFilterRegExp(QString)));
        ui->stackedWidget->addWidget(m_history);
        ui->stackedWidget->setCurrentWidget(m_history);
        ui->backTB->setEnabled(true);
        ui->filtersTB->setEnabled(false);
        ui->widget->setEnabled(false);
        emit caption(i18n("History"));
    }
}

void ApperKCM::on_backTB_clicked()
{
    bool canGoBack = false;
    if (ui->stackedWidget->currentWidget() == ui->pageBrowse) {
        if (!ui->browseView->goBack()) {
            return;
        } else if (m_groupsModel->hasParent()) {
            canGoBack = true;
        }
    } else if (ui->stackedWidget->currentWidget() == m_history) {
        ui->filtersTB->setEnabled(true);
        ui->widget->setEnabled(true);
        m_history->deleteLater();
        m_history = 0;
    } else if (ui->stackedWidget->currentWidget() == ui->pageHome) {
        if (m_groupsModel->setParentIndex()) {
            // if we are able to set a new parent item
            // do not disable back button
            return;
        }
    } else if (ui->stackedWidget->currentWidget() == m_updaterPage) {
        if (!canChangePage()) {
            return;
        }
        ui->stackedWidgetBar->setCurrentIndex(BAR_SEARCH);
        checkChanged();
    } else if (ui->stackedWidget->currentWidget() == m_settingsPage) {
        if (!canChangePage()) {
            return;
        }
        setButtons(Apply);
        emit changed(true); // THIS IS DUMB setButtons only take effect after changed goes true
        ui->stackedWidgetBar->setCurrentIndex(BAR_SEARCH);
        checkChanged();
    }

    ui->homeView->selectionModel()->clear();
    ui->stackedWidget->setCurrentWidget(ui->pageHome);
    ui->backTB->setEnabled(canGoBack);
    // reset the search role
    m_searchRole = Transaction::RoleUnknown;
    emit caption();
}

void ApperKCM::on_changesPB_clicked()
{
    m_changesModel->clear();
    m_changesModel->addSelectedPackagesFromModel(m_browseModel);
    ui->stackedWidget->setCurrentWidget(ui->pageChanges);
    ui->backTB->setEnabled(true);
    emit caption(i18n("Pending Changes"));
}

void ApperKCM::disconnectTransaction()
{
    if (m_searchTransaction) {
        // Disconnect everything so that the model don't store
        // wrong data
        m_searchTransaction->cancel();
        disconnect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   ui->browseView->busyCursor(), SLOT(stop()));
        disconnect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   this, SLOT(finished()));
        disconnect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   m_browseModel, SLOT(finished()));
        disconnect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   m_browseModel, SLOT(fetchSizes()));
        disconnect(m_searchTransaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
                   m_browseModel, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
        disconnect(m_searchTransaction, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
                   this, SLOT(errorCode(PackageKit::Transaction::Error,QString)));
    }
}

void ApperKCM::search()
{
    ui->browseView->cleanUi();
    if (ui->stackedWidgetBar->currentIndex() != BAR_SEARCH) {
        ui->stackedWidgetBar->setCurrentIndex(BAR_SEARCH);
    }

    disconnectTransaction();

    // search
    switch (m_searchRole) {
    case Transaction::RoleSearchName:
        m_searchTransaction = Daemon::searchNames(m_searchString, m_filtersMenu->filters());
        emit caption(m_searchString);
        break;
    case Transaction::RoleSearchDetails:
        m_searchTransaction = Daemon::searchDetails(m_searchString, m_filtersMenu->filters());
        emit caption(m_searchString);
        break;
    case Transaction::RoleSearchFile:
        m_searchTransaction = Daemon::searchFiles(m_searchString, m_filtersMenu->filters());
        emit caption(m_searchString);
        break;
    case Transaction::RoleSearchGroup:
        if (m_searchGroupCategory.isEmpty()) {
            m_searchTransaction = Daemon::searchGroup(m_searchGroup, m_filtersMenu->filters());
            // m_searchString has the group nice name
            emit caption(m_searchString);
        } else {
            ui->browseView->setParentCategory(m_searchParentCategory);
            emit caption(m_searchParentCategory.data().toString());
#ifndef HAVE_APPSTREAM
            if (m_searchGroupCategory.startsWith('@') ||
                m_searchGroupCategory.startsWith(QLatin1String("repo:"))) {
                m_searchTransaction = Daemon::searchGroup(m_searchGroupCategory, m_filtersMenu->filters());
            }
#endif
            // else the transaction is useless
        }
        break;
    case Transaction::RoleGetPackages:
        // we want all the installed ones
        ui->browseView->disableExportInstalledPB();
        m_searchTransaction = Daemon::getPackages(Transaction::FilterInstalled | m_filtersMenu->filters());
        connect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                ui->browseView, SLOT(enableExportInstalledPB()));
        emit caption(i18n("Installed Software"));
        break;
    case Transaction::RoleResolve:
#ifdef HAVE_APPSTREAM
        if (!m_searchCategory.isEmpty()) {
            ui->browseView->setParentCategory(m_searchParentCategory);
            // WARNING the resolve might fail if the backend
            // has a low limit MaximumItemsToResolve
            m_searchTransaction = Daemon::resolve(m_searchCategory, m_filtersMenu->filters());
            emit caption(m_searchParentCategory.data().toString());
        } else {
            ui->browseView->setParentCategory(m_searchParentCategory);
            KMessageBox::sorry(this, i18n("Could not find an application that matched this category"));
            emit caption();
            disconnectTransaction();
            m_searchTransaction = 0;
            return;
        }
        break;
#endif
    default:
        kWarning() << "Search type not defined yet";
        emit caption();
        disconnectTransaction();
        m_searchTransaction = 0;
        return;
    }
    connect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            ui->browseView->busyCursor(), SLOT(stop()));
    connect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(finished()));
    connect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            m_browseModel, SLOT(finished()));
    if (ui->browseView->isShowingSizes()) {
        connect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                m_browseModel, SLOT(fetchSizes()));
    }
    connect(m_searchTransaction, SIGNAL(package(PackageKit::Transaction::Info,QString,QString)),
            m_browseModel, SLOT(addPackage(PackageKit::Transaction::Info,QString,QString)));
    connect(m_searchTransaction, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            this, SLOT(errorCode(PackageKit::Transaction::Error,QString)));

    // cleans the models
    m_browseModel->clear();

    ui->browseView->showInstalledPanel(m_searchRole == Transaction::RoleGetPackages);
    ui->browseView->busyCursor()->start();

    ui->backTB->setEnabled(true);
    setCurrentActionCancel(true);
    setCurrentActionEnabled(m_searchTransaction->allowCancel());

    ui->stackedWidget->setCurrentWidget(ui->pageBrowse);
}

void ApperKCM::changed()
{
    Transaction *trans = qobject_cast<Transaction*>(sender());
    setCurrentActionEnabled(trans->allowCancel());
}

void ApperKCM::refreshCache()
{
    emit changed(false);

    QWidget *currentWidget = ui->stackedWidget->currentWidget();

    PkTransactionWidget *transactionW = new PkTransactionWidget(this);
    connect(transactionW, SIGNAL(titleChangedProgress(QString)), this, SIGNAL(caption(QString)));
    QPointer<PkTransaction> transaction = new PkTransaction(transactionW);
    Daemon::setHints (QLatin1String("cache-age=")+QString::number(m_cacheAge));
    transaction->refreshCache(m_forceRefreshCache);
    transactionW->setTransaction(transaction, Transaction::RoleRefreshCache);

    ui->stackedWidget->addWidget(transactionW);
    ui->stackedWidget->setCurrentWidget(transactionW);
    ui->stackedWidgetBar->setCurrentIndex(BAR_TITLE);
    ui->backTB->setEnabled(false);
    connect(transactionW, SIGNAL(titleChanged(QString)),
            ui->titleL, SLOT(setText(QString)));

    QEventLoop loop;
    connect(transaction, SIGNAL(finished(PkTransaction::ExitStatus)), &loop, SLOT(quit()));

    // wait for the end of transaction
    if (!transaction->isFinished()) {
        loop.exec();
        if (!transaction) {
            // Avoid crashing
            return;
        }

        // If the refresh failed force next refresh Cache call
        m_forceRefreshCache = transaction->exitStatus() == PkTransaction::Failed;
    }

    if (m_updaterPage) {
        m_updaterPage->getUpdates();
    }

    if (currentWidget == m_settingsPage) {
        setPage("settings");
    } else {
        setPage("updates");
    }

    QTimer::singleShot(0, this, SLOT(checkChanged()));
}

void ApperKCM::save()
{
    QWidget *currentWidget = ui->stackedWidget->currentWidget();
    if (currentWidget == m_settingsPage) {
        m_settingsPage->save();
    } else {
        PkTransactionWidget *transactionW = new PkTransactionWidget(this);
        connect(transactionW, SIGNAL(titleChangedProgress(QString)), this, SIGNAL(caption(QString)));
        QPointer<PkTransaction> transaction = new PkTransaction(transactionW);

        ui->stackedWidget->addWidget(transactionW);
        ui->stackedWidget->setCurrentWidget(transactionW);
        ui->stackedWidgetBar->setCurrentIndex(BAR_TITLE);
        ui->backTB->setEnabled(false);
        connect(transactionW, SIGNAL(titleChanged(QString)),
                ui->titleL, SLOT(setText(QString)));
        emit changed(false);

        QEventLoop loop;
        connect(transaction, SIGNAL(finished(PkTransaction::ExitStatus)), &loop, SLOT(quit()));
        if (currentWidget == m_updaterPage) {
            transaction->updatePackages(m_updaterPage->packagesToUpdate());
            transactionW->setTransaction(transaction, Transaction::RoleUpdatePackages);

            // wait for the end of transaction
            if (!transaction->isFinished()) {
                loop.exec();
                if (!transaction) {
                    // Avoid crashing
                    return;
                }
            }
        } else {
            // install then remove packages
            QStringList installPackages = m_browseModel->selectedPackagesToInstall();
            if (!installPackages.isEmpty()) {
                transaction->installPackages(installPackages);
                transactionW->setTransaction(transaction, Transaction::RoleInstallPackages);

                // wait for the end of transaction
                if (!transaction->isFinished()) {
                    loop.exec();
                    if (!transaction) {
                        // Avoid crashing
                        return;
                    }
                }

                if (transaction->exitStatus() == PkTransaction::Success) {
                    m_browseModel->uncheckAvailablePackages();
                }
            }

            QStringList removePackages = m_browseModel->selectedPackagesToRemove();
            if (!removePackages.isEmpty()) {
                transaction->removePackages(removePackages);
                transactionW->setTransaction(transaction, Transaction::RoleRemovePackages);

                // wait for the end of transaction
                if (!transaction->isFinished()) {
                    loop.exec();
                    if (!transaction) {
                        // Avoid crashing
                        return;
                    }
                }

                if (transaction->exitStatus() == PkTransaction::Success) {
                    m_browseModel->uncheckInstalledPackages();
                }
            }
        }

        transaction->deleteLater();
        if (currentWidget == m_updaterPage) {
            m_updaterPage->getUpdates();
            setPage("updates");
        } else {
            // install then remove packages
            search();
        }
        QTimer::singleShot(0, this, SLOT(checkChanged()));
    }
}

void ApperKCM::load()
{
    if (ui->stackedWidget->currentWidget() == m_updaterPage) {
        m_updaterPage->load();
    } else if (ui->stackedWidget->currentWidget() == m_settingsPage) {
        m_settingsPage->load();
    } else {
        // set focus on the search lineEdit
        ui->searchKLE->setFocus(Qt::OtherFocusReason);
        m_browseModel->setAllChecked(false);
    }
}

void ApperKCM::defaults()
{
    if (ui->stackedWidget->currentWidget() == m_settingsPage) {
        m_settingsPage->defaults();
    }
}

void ApperKCM::finished()
{
    // if m_currentAction is false means that our
    // find button should be disable as there aren't any
    // search methods
    setCurrentActionEnabled(m_currentAction);
    setCurrentActionCancel(false);
    m_searchTransaction = 0;
}

void ApperKCM::keyPressEvent(QKeyEvent *event)
{
    if (ui->searchKLE->hasFocus() &&
        ui->stackedWidget->currentWidget() != m_history &&
        (event->key() == Qt::Key_Return ||
         event->key() == Qt::Key_Enter)) {
        // special tab handling here
        m_currentAction->trigger();
        return;
    }
    KCModule::keyPressEvent(event);
}

void ApperKCM::closeEvent(QCloseEvent *event)
{
//     PkTransaction *transaction = qobject_cast<PkTransaction*>(stackedWidget->currentWidget());
//     if (transaction) {
        event->ignore();
//     } else {
//         event->accept();
//     }
}

#include "ApperKCM.moc"
