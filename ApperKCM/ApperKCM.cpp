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

#include "ApperKCM.h"

#include <config.h>

#include <KGenericFactory>
#include <KAboutData>

#include <version.h>

#include "KpkFiltersMenu.h"
#include "BrowseView.h"
#include "CategoryModel.h"
#include "TransactionHistory.h"
#include "Settings/Settings.h"
#include "Updater/Updater.h"

#include <KLocale>
#include <KStandardDirs>
#include <KMessageBox>
#include <KFileItemDelegate>

#include <KpkPackageModel.h>
#include <KpkReviewChanges.h>
#include <KpkDelegate.h>
#include <KpkStrings.h>
#include <KpkIcons.h>
#include <AppInstall.h>

#include <KDebug>

KCONFIGGROUP_DECLARE_ENUM_QOBJECT(Enum, Filter)

K_PLUGIN_FACTORY(KPackageKitFactory, registerPlugin<ApperKCM>();)
K_EXPORT_PLUGIN(KPackageKitFactory("kcm_apper"))

ApperKCM::ApperKCM(QWidget *parent, const QVariantList &args) :
    KCModule(KPackageKitFactory::componentData(), parent, args),
    m_currentAction(0),
    m_groupsProxyModel(0),
    m_settingsPage(0),
    m_updaterPage(0),
    m_searchTransaction(0),
    m_findIcon("edit-find"),
    m_cancelIcon("dialog-cancel"),
    m_history(0),
    m_searchRole(Enum::UnknownRole)
{
    KAboutData *aboutData;
    aboutData = new KAboutData("apper",
                               "apper",
                               ki18n("Application Manager"),
                               KPK_VERSION,
                               ki18n("KDE interface for managing software"),
                               KAboutData::License_GPL,
                               ki18n("(C) 2008-2010 Daniel Nicoletti"));
    setAboutData(aboutData);
    setButtons(Apply);
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

    connect(browseView, SIGNAL(categoryActivated(const QModelIndex &)),
            this, SLOT(on_homeView_clicked(const QModelIndex &)));

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
    browseView->setCategoryModel(m_groupsModel);
    connect(m_groupsModel, SIGNAL(finished()),
            this, SLOT(setupHomeModel()));
    homeView->setSpacing(KDialog::spacingHint());
    homeView->viewport()->setAttribute(Qt::WA_Hover);

    KFileItemDelegate *delegate = new KFileItemDelegate(this);
    delegate->setWrapMode(QTextOption::WordWrap);
    homeView->setItemDelegate(delegate);

    // install the backend filters
    filtersTB->setMenu(m_filtersMenu = new KpkFiltersMenu(Client::instance()->filters(), this));
    filtersTB->setIcon(KIcon("view-filter"));
    browseView->proxy()->setFilterFixedString(m_filtersMenu->filterApplications());
    connect(m_filtersMenu, SIGNAL(filterApplications(const QString &)),
            browseView->proxy(), SLOT(setFilterFixedString(const QString &)));


    //initialize the model, delegate, client and  connect it's signals
    m_browseModel = browseView->model();

    // CHANGES TAB
    changesView->viewport()->setAttribute(Qt::WA_Hover);
    m_changesModel = new KpkPackageModel(this);
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

    changesPB->setIcon(KIcon("edit-redo"));
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

void ApperKCM::errorCode(PackageKit::Enum::Error error, const QString &details)
{
    if (error != Enum::ErrorTransactionCancelled) {
        KMessageBox::detailedSorry(this, KpkStrings::errorMessage(error), details, KpkStrings::error(error), KMessageBox::Notify);
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
        m_searchRole    = Enum::RoleSearchName;
        m_searchString  = searchKLE->text();
        m_searchFilters = m_filtersMenu->filters();
        // create the main transaction
        search();
    }
}

void ApperKCM::on_actionFindDescription_triggered()
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

void ApperKCM::on_actionFindFile_triggered()
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
        m_searchRole    = static_cast<Enum::Role>(index.data(CategoryModel::SearchRole).toUInt());
        m_searchFilters = m_filtersMenu->filters();
        if (m_searchRole == Enum::RoleResolve) {
            m_searchString = index.data(CategoryModel::CategoryRole).toString();
        } else if (m_searchRole == Enum::RoleSearchGroup) {
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
                m_searchGroup = static_cast<Enum::Group>(index.data(CategoryModel::GroupRole).toUInt());
            }
        } else if (m_searchRole == Enum::RoleGetOldTransactions) {
            m_history = new TransactionHistory(this);
            searchKLE->clear();
            connect(searchKLE, SIGNAL(textChanged(const QString &)),
                    m_history, SLOT(setFilterRegExp(const QString &)));
            stackedWidget->addWidget(m_history);
            stackedWidget->setCurrentWidget(m_history);
            backTB->setEnabled(true);
            filtersTB->setEnabled(false);
            widget->setEnabled(false);
            return;
        } else if (m_searchRole == Enum::RoleGetUpdates) {
            if (m_updaterPage == 0) {
                m_updaterPage = new Updater(this);
                stackedWidget->addWidget(m_updaterPage);
                checkUpdatesPB->setIcon(KIcon("view-refresh"));
                connect(checkUpdatesPB, SIGNAL(clicked(bool)),
                        m_updaterPage, SLOT(refreshCache()));
            }

            if (!canChangePage(m_browseModel->hasChanges())) {
                return;
            }

            connect(m_updaterPage, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
            emit changed(false);
            stackedWidget->setCurrentWidget(m_updaterPage);
            m_updaterPage->load();
            stackedWidgetBar->setCurrentIndex(1);
            backTB->setEnabled(true);
            return;
        } else if (m_searchRole == Enum::RoleGetRepoList) {
            if (m_settingsPage == 0) {
                m_settingsPage = new Settings(this);
                stackedWidget->addWidget(m_settingsPage);
                m_settingsPage->load();
            }

            if (!canChangePage(m_browseModel->hasChanges())) {
                return;
            }

            connect(m_settingsPage, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
            setButtons(KCModule::Default | KCModule::Apply);
            emit changed(true); // THIS IS DUMB setButtons only take effect after changed goes true
            emit changed(false);
            stackedWidget->setCurrentWidget(m_settingsPage);
            m_settingsPage->load();
            stackedWidgetBar->setCurrentIndex(2);
            backTB->setEnabled(true);
            return;
        }
        // create the main transaction
        search();
    }
}

bool ApperKCM::canChangePage(bool changed)
{
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
        if (!canChangePage(m_updaterPage->hasChanges())) {
            return;
        }
        stackedWidgetBar->setCurrentIndex(0);
        disconnect(m_updaterPage, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
        checkChanged();
    } else if (stackedWidget->currentWidget() == m_settingsPage) {
        if (!canChangePage(m_settingsPage->hasChanges())) {
            return;
        }
        setButtons(Apply);
        emit changed(true); // THIS IS DUMB setButtons only take effect after changed goes true
        stackedWidgetBar->setCurrentIndex(0);
        disconnect(m_settingsPage, SIGNAL(changed(bool)), this, SIGNAL(changed(bool)));
        checkChanged();
    }

    homeView->selectionModel()->clear();
    stackedWidget->setCurrentWidget(pageHome);
    backTB->setEnabled(canGoBack);
    // reset the search role
    m_searchRole = Enum::UnknownRole;
}

void ApperKCM::on_changesPB_clicked()
{
    m_changesModel->clear();
    m_changesModel->addPackages(m_browseModel->selectedPackages(), true);
    stackedWidget->setCurrentWidget(pageChanges);
    backTB->setEnabled(true);
}

void ApperKCM::search()
{
    browseView->cleanUi();

    if (m_searchTransaction) {
        // Disconnect everything so that the model don't store
        // wrong data
        m_searchTransaction->cancel();
        disconnect(m_searchTransaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                   browseView->busyCursor(), SLOT(stop()));
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
            browseView->busyCursor(), SLOT(stop()));
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
        if (m_searchGroupCategory.isEmpty()) {
            m_searchTransaction->searchGroups(m_searchGroup, m_searchFilters);
        } else {
            browseView->setParentCategory(m_searchParentCategory);
#ifndef HAVE_APPINSTALL
            if (m_searchGroupCategory.startsWith('@') ||
                m_searchGroupCategory.startsWith(QLatin1String("repo:"))) {
                m_searchTransaction->searchGroups(m_searchGroupCategory, m_searchFilters);
            }
#endif //HAVE_APPINSTALL
            // else the transaction is useless
        }
        break;
    case Enum::RoleGetPackages:
        // we want all the installed ones
        browseView->disableExportInstalledPB();
        connect(m_searchTransaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                browseView, SLOT(enableExportInstalledPB()));
        m_searchTransaction->getPackages(Enum::FilterInstalled);
        break;
    case Enum::RoleResolve:
    {
        QStringList packages = AppInstall::instance()->pkgNamesFromWhere(m_searchString);
        if (!packages.isEmpty()) {
            browseView->setParentCategory(m_searchParentCategory);
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

        browseView->showInstalledPanel(m_searchRole == Enum::RoleGetPackages);
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

void ApperKCM::save()
{
    kDebug() << stackedWidget->currentWidget() << m_updaterPage << m_settingsPage;
    if (stackedWidget->currentWidget() == m_updaterPage) {
        m_updaterPage->save();
    } else if (stackedWidget->currentWidget() == m_settingsPage) {
        m_settingsPage->save();
    } else {
        QPointer<KpkReviewChanges> frm = new KpkReviewChanges(m_browseModel->selectedPackages(), this);
        connect(frm, SIGNAL(successfullyInstalled()), m_browseModel, SLOT(uncheckAvailablePackages()));
        connect(frm, SIGNAL(successfullyRemoved()), m_browseModel, SLOT(uncheckInstalledPackages()));

        frm->exec();

        // This avoid crashing as the above function does not always quit it's event loop
        if (!frm.isNull()) {
            frm->deleteLater();

            search();
            QTimer::singleShot(0, this, SLOT(checkChanged()));
        }
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

#include "ApperKCM.moc"
