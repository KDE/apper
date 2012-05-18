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

#include <config.h>

#include <KGenericFactory>
#include <KAboutData>

#include <version.h>

#include "FiltersMenu.h"
#include "BrowseView.h"
#include "CategoryModel.h"
#include "TransactionHistory.h"
#include "Settings/Settings.h"
#include "Updater/Updater.h"

#include <KLocale>
#include <KStandardDirs>
#include <KMessageBox>
#include <KFileItemDelegate>
#include <KMenu>
#include <KTabBar>

#include <PackageModel.h>
#include <ChangesDelegate.h>
#include <PkStrings.h>
#include <PkIcons.h>
#include <AppInstall.h>

#include <KDebug>

#include <Daemon>

#define BAR_SEARCH   0
#define BAR_UPDATE   1
#define BAR_TITLE    2
#define BAR_SETTINGS 3

KCONFIGGROUP_DECLARE_ENUM_QOBJECT(Transaction, Filter)

K_PLUGIN_FACTORY(ApperFactory, registerPlugin<ApperKCM>();)
K_EXPORT_PLUGIN(ApperFactory("kcm_apper"))

ApperKCM::ApperKCM(QWidget *parent, const QVariantList &args) :
    KCModule(ApperFactory::componentData(), parent, args),
    m_currentAction(0),
    m_groupsProxyModel(0),
    m_settingsPage(0),
    m_updaterPage(0),
    m_searchTransaction(0),
    m_findIcon("edit-find"),
    m_cancelIcon("dialog-cancel"),
    m_forceRefreshCache(false),
    m_history(0),
    m_searchRole(Transaction::UnknownRole)
{
    KAboutData *aboutData;
    aboutData = new KAboutData("apper",
                               "apper",
                               ki18n("Application Manager"),
                               APP_VERSION,
                               ki18n("KDE interface for managing software"),
                               KAboutData::License_GPL,
                               ki18n("(C) 2008-2011 Daniel Nicoletti"));
    setAboutData(aboutData);
    setButtons(Apply);
    KGlobal::insertCatalog(QLatin1String("apper"));

    // store the actions supported by the backend
    m_roles = Daemon::actions();

    // Set the current locale
    QString locale = QString(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
    Daemon::setHints("locale=" + locale);

    setupUi(this);
    browseView->init(m_roles);

    // Browse TAB
    backTB->setIcon(KIcon("go-previous"));

    // create our toolbar
    QToolBar *toolBar = new QToolBar(this);
    gridLayout_2->addWidget(toolBar);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    connect(browseView, SIGNAL(categoryActivated(QModelIndex)),
            this, SLOT(on_homeView_clicked(QModelIndex)));

    QMenu *findMenu = new QMenu(this);
    // find is just a generic name in case we don't have any search method
    m_genericActionK = new KToolBarPopupAction(m_findIcon, i18n("Find"), this);
    toolBar->addAction(m_genericActionK);

    // Add actions that the backend supports
    if (m_roles & Transaction::RoleSearchName) {
        findMenu->addAction(actionFindName);
        setCurrentAction(actionFindName);
    }
    if (m_roles & Transaction::RoleSearchDetails) {
        findMenu->addAction(actionFindDescription);
        if (!m_currentAction) {
            setCurrentAction(actionFindDescription);
        }
    }
    if (m_roles & Transaction::RoleSearchFile) {
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
    m_groupsModel = new CategoryModel(m_roles, this);
    browseView->setCategoryModel(m_groupsModel);
    connect(m_groupsModel, SIGNAL(finished()),
            this, SLOT(setupHomeModel()));
    homeView->setSpacing(KDialog::spacingHint());
    homeView->viewport()->setAttribute(Qt::WA_Hover);

    KFileItemDelegate *delegate = new KFileItemDelegate(this);
    delegate->setWrapMode(QTextOption::WordWrap);
    homeView->setItemDelegate(delegate);

    // install the backend filters
    filtersTB->setMenu(m_filtersMenu = new FiltersMenu(Daemon::filters(), this));
    connect(m_filtersMenu, SIGNAL(filtersChanged()), this, SLOT(search()));
    filtersTB->setIcon(KIcon("view-filter"));
    browseView->proxy()->setFilterFixedString(m_filtersMenu->filterApplications());
    connect(m_filtersMenu, SIGNAL(filterApplications(QString)),
            browseView->proxy(), SLOT(setFilterFixedString(QString)));

    //initialize the model, delegate, client and  connect it's signals
    m_browseModel = browseView->model();

    // CHANGES TAB
    changesView->viewport()->setAttribute(Qt::WA_Hover);
    m_changesModel = new PackageModel(this);
    KCategorizedSortFilterProxyModel *changedProxy = new KCategorizedSortFilterProxyModel(this);
    changedProxy->setSourceModel(m_changesModel);
    changedProxy->setCategorizedModel(true);
    changedProxy->sort(0);
    changedProxy->setDynamicSortFilter(true);
    changedProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    changedProxy->setSortRole(PackageModel::SortRole);
    changesView->setModel(changedProxy);
    ChangesDelegate *changesDelegate = new ChangesDelegate(changesView);
    changesDelegate->setExtendPixmapWidth(0);
    changesView->setItemDelegate(changesDelegate);

    // Connect this signal to keep track of changes
    connect(m_browseModel, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
    connect(m_browseModel, SIGNAL(changed(bool)), changesPB, SLOT(setEnabled(bool)));

    // packageUnchecked from changes model
    connect(m_changesModel, SIGNAL(packageUnchecked(PackageModel::InternalPackage)),
            m_changesModel, SLOT(rmSelectedPackage(PackageModel::InternalPackage)));
    connect(m_changesModel, SIGNAL(packageUnchecked(PackageModel::InternalPackage)),
            m_browseModel, SLOT(uncheckPackage(PackageModel::InternalPackage)));

    changesPB->setIcon(KIcon("edit-redo"));

    KMenu *menu = new KMenu(this);
    settingsTB->setMenu(menu);
    settingsTB->setIcon(KIcon("preferences-other"));
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
}

void ApperKCM::setupHomeModel()
{
    KCategorizedSortFilterProxyModel *oldProxy = m_groupsProxyModel;
    m_groupsProxyModel = new KCategorizedSortFilterProxyModel(this);
    m_groupsProxyModel->setSourceModel(m_groupsModel);
    m_groupsProxyModel->setCategorizedModel(true);
    m_groupsProxyModel->sort(0);
    homeView->setModel(m_groupsProxyModel);
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

void ApperKCM::checkChanged()
{
    bool value = m_browseModel->hasChanges();
    changesPB->setEnabled(value);
    emit changed(value);
}

void ApperKCM::errorCode(PackageKit::Transaction::Error error, const QString &details)
{
    if (error != Transaction::ErrorTransactionCancelled) {
        KMessageBox::detailedSorry(this, PkStrings::errorMessage(error), details, PkStrings::error(error), KMessageBox::Notify);
    }
}

ApperKCM::~ApperKCM()
{
}

void ApperKCM::on_actionFindName_triggered()
{
    setCurrentAction(actionFindName);
    if (!searchKLE->text().isEmpty()) {
        // cache the search
        m_searchRole    = Transaction::RoleSearchName;
        m_searchString  = searchKLE->text();
        // create the main transaction
        search();
    }
}

void ApperKCM::on_actionFindDescription_triggered()
{
    setCurrentAction(actionFindDescription);
    if (!searchKLE->text().isEmpty()) {
        // cache the search
        m_searchRole    = Transaction::RoleSearchDetails;
        m_searchString  = searchKLE->text();
        // create the main transaction
        search();
    }
}

void ApperKCM::on_actionFindFile_triggered()
{
    setCurrentAction(actionFindFile);
    if (!searchKLE->text().isEmpty()) {
        // cache the search
        m_searchRole    = Transaction::RoleSearchFile;
        m_searchString  = searchKLE->text();
        // create the main transaction
        search();
    }
}

void ApperKCM::on_homeView_clicked(const QModelIndex &index)
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
        m_searchRole    = static_cast<Transaction::Role>(index.data(CategoryModel::SearchRole).toUInt());
        if (m_searchRole == Transaction::RoleResolve) {
            m_searchString = index.data(CategoryModel::CategoryRole).toString();
        } else if (m_searchRole == Transaction::RoleSearchGroup) {
            if (index.data(CategoryModel::GroupRole).type() == QVariant::String) {
                QString category = index.data(CategoryModel::GroupRole).toString();
                if (category.startsWith('@') ||
                    (category.startsWith(QLatin1String("repo:")) && category.size() > 5)) {
                    m_searchGroupCategory = category;
                } else {
                    m_groupsModel->setRootIndex(m_searchParentCategory);
                    backTB->setEnabled(true);
                    return;
                }
            } else {
                m_searchGroupCategory.clear();
                m_searchGroup = static_cast<Package::Group>(index.data(CategoryModel::GroupRole).toUInt());
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
    if (stackedWidget->currentWidget() == m_updaterPage) {
        changed = m_updaterPage->hasChanges();
    } else if (stackedWidget->currentWidget() == m_settingsPage) {
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
    PkTransaction *transaction = qobject_cast<PkTransaction*>(stackedWidget->currentWidget());
    if (transaction) {
        return;
    }

    if (page == "settings") {
        if (stackedWidget->currentWidget() != m_settingsPage) {
            if (!canChangePage()) {
                return;
            }

            if (m_settingsPage == 0) {
                m_settingsPage = new Settings(m_roles, this);
                stackedWidget->addWidget(m_settingsPage);
                m_settingsPage->load();
                KTabBar *tabBar = new KTabBar(this);
                tabBar->addTab(i18n("General Settings"));
                tabBar->addTab(i18n("Software Origins"));
                connect(tabBar, SIGNAL(currentChanged(int)),
                        m_settingsPage, SLOT(changeCurrentPage(int)));
                stackedWidgetBar->addWidget(tabBar);
            }
            connect(m_settingsPage, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
            setButtons(KCModule::Default | KCModule::Apply);
            emit changed(true); // THIS IS DUMB setButtons only take effect after changed goes true
            emit changed(false);
            stackedWidget->setCurrentWidget(m_settingsPage);
            m_settingsPage->load();
            stackedWidgetBar->setCurrentIndex(BAR_SETTINGS);
            titleL->clear();
            backTB->setEnabled(true);
        }
    } else if (page == "updates" || page == "updatesSelected") {
        if (stackedWidget->currentWidget() != m_updaterPage) {
            if (!canChangePage()) {
                return;
            }

            if (m_updaterPage == 0) {
                m_updaterPage = new Updater(m_roles, this);
                connect(m_updaterPage, SIGNAL(refreshCache()),
                        this, SLOT(refreshCache()));
                connect(m_updaterPage, SIGNAL(downloadSize(QString)),
                        downloadL, SLOT(setText(QString)));
                stackedWidget->addWidget(m_updaterPage);
                checkUpdatesPB->setIcon(KIcon("view-refresh"));
                connect(checkUpdatesPB, SIGNAL(clicked(bool)),
                        this, SLOT(refreshCache()));
            }

            connect(m_updaterPage, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
            emit changed(false);
            stackedWidget->setCurrentWidget(m_updaterPage);
            m_updaterPage->setSelected(page == "updatesSelected");
            m_updaterPage->load();
            stackedWidgetBar->setCurrentIndex(BAR_UPDATE);
            backTB->setEnabled(true);
        }
    } else if (page == "home") {
        if (stackedWidget->currentWidget() == m_updaterPage ||
            stackedWidget->currentWidget() == m_settingsPage) {
            on_backTB_clicked();
        }
    } else if (page == "history") {
        m_history = new TransactionHistory(this);
        searchKLE->clear();
        connect(searchKLE, SIGNAL(textChanged(QString)),
                m_history, SLOT(setFilterRegExp(QString)));
        stackedWidget->addWidget(m_history);
        stackedWidget->setCurrentWidget(m_history);
        backTB->setEnabled(true);
        filtersTB->setEnabled(false);
        widget->setEnabled(false);
    }
}

void ApperKCM::on_backTB_clicked()
{
    bool canGoBack = false;
    if (stackedWidget->currentWidget() == pageBrowse) {
        if (!browseView->goBack()) {
            return;
        } else if (m_groupsModel->hasParent()) {
            canGoBack = true;
        }
    } else if (stackedWidget->currentWidget() == m_history) {
        filtersTB->setEnabled(true);
        widget->setEnabled(true);
        m_history->deleteLater();
        m_history = 0;
    } else if (stackedWidget->currentWidget() == pageHome) {
        if (m_groupsModel->setParentIndex()) {
            // if we are able to set a new parent item
            // do not disable back button
            return;
        }
    } else if (stackedWidget->currentWidget() == m_updaterPage) {
        if (!canChangePage()) {
            return;
        }
        stackedWidgetBar->setCurrentIndex(BAR_SEARCH);
        disconnect(m_updaterPage, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
        checkChanged();
    } else if (stackedWidget->currentWidget() == m_settingsPage) {
        if (!canChangePage()) {
            return;
        }
        setButtons(Apply);
        emit changed(true); // THIS IS DUMB setButtons only take effect after changed goes true
        stackedWidgetBar->setCurrentIndex(BAR_SEARCH);
        disconnect(m_settingsPage, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
        checkChanged();
    }

    homeView->selectionModel()->clear();
    stackedWidget->setCurrentWidget(pageHome);
    backTB->setEnabled(canGoBack);
    // reset the search role
    m_searchRole = Transaction::UnknownRole;
}

void ApperKCM::on_changesPB_clicked()
{
    m_changesModel->clear();
    m_changesModel->addPackages(m_browseModel->selectedPackages(), true);
    stackedWidget->setCurrentWidget(pageChanges);
    backTB->setEnabled(true);
}

void ApperKCM::disconnectTransaction()
{
    if (m_searchTransaction) {
        // Disconnect everything so that the model don't store
        // wrong data
        m_searchTransaction->cancel();
        disconnect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   browseView->busyCursor(), SLOT(stop()));
        disconnect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   this, SLOT(finished()));
        disconnect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   m_browseModel, SLOT(finished()));
        disconnect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                   m_browseModel, SLOT(fetchSizes()));
        disconnect(m_searchTransaction, SIGNAL(package(PackageKit::Package)),
                   m_browseModel, SLOT(addPackage(PackageKit::Package)));
        disconnect(m_searchTransaction, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
                   this, SLOT(errorCode(PackageKit::Transaction::Error,QString)));
    }
}

void ApperKCM::search()
{
    browseView->cleanUi();

    disconnectTransaction();

    // search
    m_searchTransaction = new Transaction(this);
    connect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            browseView->busyCursor(), SLOT(stop()));
    connect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            this, SLOT(finished()));
    connect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
            m_browseModel, SLOT(finished()));
    if (browseView->isShowingSizes()) {
        connect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                m_browseModel, SLOT(fetchSizes()));
    }
    connect(m_searchTransaction, SIGNAL(package(PackageKit::Package)),
            m_browseModel, SLOT(addPackage(PackageKit::Package)));
    connect(m_searchTransaction, SIGNAL(errorCode(PackageKit::Transaction::Error,QString)),
            this, SLOT(errorCode(PackageKit::Transaction::Error,QString)));
    switch (m_searchRole) {
    case Transaction::RoleSearchName:
        m_searchTransaction->searchNames(m_searchString, m_filtersMenu->filters());
        break;
    case Transaction::RoleSearchDetails:
        m_searchTransaction->searchDetails(m_searchString, m_filtersMenu->filters());
        break;
    case Transaction::RoleSearchFile:
        m_searchTransaction->searchFiles(m_searchString, m_filtersMenu->filters());
        break;
    case Transaction::RoleSearchGroup:
        if (m_searchGroupCategory.isEmpty()) {
            m_searchTransaction->searchGroup(m_searchGroup, m_filtersMenu->filters());
        } else {
            browseView->setParentCategory(m_searchParentCategory);
#ifndef HAVE_APPINSTALL
            if (m_searchGroupCategory.startsWith('@') ||
                m_searchGroupCategory.startsWith(QLatin1String("repo:"))) {
                m_searchTransaction->searchGroup(m_searchGroupCategory, m_filtersMenu->filters());
            }
#endif //HAVE_APPINSTALL
            // else the transaction is useless
        }
        break;
    case Transaction::RoleGetPackages:
        // we want all the installed ones
        browseView->disableExportInstalledPB();
        connect(m_searchTransaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                browseView, SLOT(enableExportInstalledPB()));
        m_searchTransaction->getPackages(Transaction::FilterInstalled | m_filtersMenu->filters());
        break;
    case Transaction::RoleResolve:
    {
        QStringList packages = AppInstall::instance()->pkgNamesFromWhere(m_searchString);
        if (!packages.isEmpty()) {
            browseView->setParentCategory(m_searchParentCategory);
            // WARNING the resolve might fail if the backend
            // has a low limit MaximumItemsToResolve
            m_searchTransaction->resolve(packages, m_filtersMenu->filters());
        } else {
            return;
        }
        break;
    }
    default:
        kDebug() << "Search type not defined yet";
        return;
    }

    Transaction::InternalError error = m_searchTransaction->error();
    if (error) {
        setCurrentActionEnabled(true);
        disconnectTransaction();
        m_searchTransaction = 0;
        KMessageBox::sorry(this, PkStrings::daemonError(error));
    } else {
        // cleans the models
        m_browseModel->clear();

        browseView->showInstalledPanel(m_searchRole == Transaction::RoleGetPackages);
        browseView->busyCursor()->start();

        backTB->setEnabled(true);
        setCurrentActionCancel(true);
        setCurrentActionEnabled(m_searchTransaction->allowCancel());

        stackedWidget->setCurrentWidget(pageBrowse);
    }
}

void ApperKCM::changed()
{
    Transaction *trans = qobject_cast<Transaction*>(sender());
    setCurrentActionEnabled(trans->allowCancel());
}

void ApperKCM::refreshCache()
{
    QWidget *currentWidget = stackedWidget->currentWidget();
    emit changed(false);

    PkTransaction *transaction = new PkTransaction(this);
    QWeakPointer<PkTransaction> pointer = transaction;

    stackedWidget->addWidget(transaction);
    stackedWidget->setCurrentWidget(transaction);
    int oldBar = stackedWidgetBar->currentIndex();
    stackedWidgetBar->setCurrentIndex(BAR_TITLE);
    backTB->setEnabled(false);
    connect(transaction, SIGNAL(titleChanged(QString)),
            titleL, SLOT(setText(QString)));

    QEventLoop loop;
    connect(transaction, SIGNAL(finished(PkTransaction::ExitStatus)), &loop, SLOT(quit()));
    transaction->refreshCache(m_forceRefreshCache);

    // wait for the end of transaction
    if (!transaction->isFinished()) {
        loop.exec();
        if (pointer.isNull()) {
            // Avoid crashing
            return;
        }

        // If the refresh failed force next refresh Cache call
        m_forceRefreshCache = transaction->exitStatus() == PkTransaction::Failed;
    }

    // Finished setup old stuff
    backTB->setEnabled(true);
    stackedWidget->setCurrentWidget(currentWidget);
    stackedWidgetBar->setCurrentIndex(oldBar);
    transaction->deleteLater();
    if (currentWidget == m_updaterPage) {
        m_updaterPage->getUpdates();
    } else {
        // install then remove packages
        search();
    }
    QTimer::singleShot(0, this, SLOT(checkChanged()));
}

void ApperKCM::save()
{
    QWidget *currentWidget = stackedWidget->currentWidget();
    if (currentWidget == m_settingsPage) {
        m_settingsPage->save();
    } else {
        PkTransaction *transaction = new PkTransaction(this);
        QWeakPointer<PkTransaction> pointer = transaction;

        stackedWidget->addWidget(transaction);
        stackedWidget->setCurrentWidget(transaction);
        int oldBar = stackedWidgetBar->currentIndex();
        stackedWidgetBar->setCurrentIndex(BAR_TITLE);
        backTB->setEnabled(false);
        connect(transaction, SIGNAL(titleChanged(QString)),
                titleL, SLOT(setText(QString)));
        emit changed(false);

        QEventLoop loop;
        connect(transaction, SIGNAL(finished(PkTransaction::ExitStatus)), &loop, SLOT(quit()));
        if (currentWidget == m_updaterPage) {
            transaction->updatePackages(m_updaterPage->packagesToUpdate());

            // wait for the end of transaction
            if (!transaction->isFinished()) {
                loop.exec();
                if (pointer.isNull()) {
                    // Avoid crashing
                    return;
                }
            }
        } else {
            // install then remove packages
            QList<Package> removePackages;
            QList<Package> installPackages;
            foreach (const Package &p, m_browseModel->selectedPackages()) {
                if (p.info() == Package::InfoInstalled ||
                    p.info() == Package::InfoCollectionInstalled) {
                    // check what packages are installed and marked to be removed
                    removePackages << p;
                } else if (p.info() == Package::InfoAvailable ||
                           p.info() == Package::InfoCollectionAvailable) {
                    // check what packages are available and marked to be installed
                    installPackages << p;
                }
            }

            if (!installPackages.isEmpty()) {
                transaction->installPackages(installPackages);

                // wait for the end of transaction
                if (!transaction->isFinished()) {
                    loop.exec();
                    if (pointer.isNull()) {
                        // Avoid crashing
                        return;
                    }
                }

                if (transaction->exitStatus() == PkTransaction::Success) {
                    m_browseModel->uncheckAvailablePackages();
                }
            }

            if (!removePackages.isEmpty()) {
                transaction->removePackages(removePackages);

                // wait for the end of transaction
                if (!transaction->isFinished()) {
                    loop.exec();
                    if (pointer.isNull()) {
                        // Avoid crashing
                        return;
                    }
                }

                if (transaction->exitStatus() == PkTransaction::Success) {
                    m_browseModel->uncheckInstalledPackages();
                }
            }
        }

        // Finished setup old stuff
        backTB->setEnabled(true);
        stackedWidget->setCurrentWidget(currentWidget);
        stackedWidgetBar->setCurrentIndex(oldBar);
        transaction->deleteLater();
        if (currentWidget == m_updaterPage) {
            m_updaterPage->getUpdates();
        } else {
            // install then remove packages
            search();
        }
        QTimer::singleShot(0, this, SLOT(checkChanged()));
    }
}

void ApperKCM::load()
{
    if (stackedWidget->currentWidget() == m_updaterPage) {
        m_updaterPage->load();
    } else if (stackedWidget->currentWidget() == m_settingsPage) {
        m_settingsPage->load();
    } else {
        // set focus on the search lineEdit
        searchKLE->setFocus(Qt::OtherFocusReason);
        m_browseModel->setAllChecked(false);
    }
}

void ApperKCM::defaults()
{
    if (stackedWidget->currentWidget() == m_settingsPage) {
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
    if (searchKLE->hasFocus() &&
        stackedWidget->currentWidget() != m_history &&
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
