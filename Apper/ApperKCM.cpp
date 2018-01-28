/***************************************************************************
 *   Copyright (C) 2008-2018 by Daniel Nicoletti                           *
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

#include <KAboutData>

#include <KLocalizedString>
#include <QStandardPaths>
#include <KMessageBox>
#include <KFileItemDelegate>
#include <KHelpMenu>
#include <QTabBar>
#include <QToolBar>
#include <QSignalMapper>
#include <QTimer>
#include <QPointer>

#include <PackageModel.h>
#include <ApplicationSortFilterModel.h>
#include <ChangesDelegate.h>
#include <PkStrings.h>
#include <PkIcons.h>
#include <PkTransactionWidget.h>

#ifdef HAVE_APPSTREAM
#include <AppStream.h>
#endif

#include <QLoggingCategory>
#include <Daemon>

#include "FiltersMenu.h"
#include "BrowseView.h"
#include "CategoryModel.h"
#include "TransactionHistory.h"
#include "Settings/Settings.h"
#include "Updater/Updater.h"

Q_LOGGING_CATEGORY(APPER, "apper")

#define BAR_SEARCH   0
#define BAR_UPDATE   1
#define BAR_SETTINGS 2
#define BAR_TITLE    3

ApperKCM::ApperKCM(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ApperKCM),
    m_findIcon(QIcon::fromTheme(QLatin1String("edit-find"))),
    m_cancelIcon(QIcon::fromTheme(QLatin1String("dialog-cancel")))
{
    ui->setupUi(this);

    // store the actions supported by the backend
    connect(Daemon::global(), &Daemon::changed, this, &ApperKCM::daemonChanged);

    // Set the current locale
    Daemon::global()->setHints(QLatin1String("locale=") + QLocale::system().name() + QLatin1String(".UTF-8"));

    // Browse TAB
    ui->backTB->setIcon(QIcon::fromTheme(QLatin1String("go-previous")));

    // create our toolbar
    auto toolBar = new QToolBar(this);
    ui->gridLayout_2->addWidget(toolBar);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    connect(ui->browseView, &BrowseView::categoryActivated, this, &ApperKCM::on_homeView_activated);

    auto findMenu = new QMenu(this);
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
        connect(m_genericActionK, &KToolBarPopupAction::triggered, this, &ApperKCM::genericActionKTriggered);
    }

    // Create the groups model
    m_groupsModel = new CategoryModel(this);
    ui->browseView->setCategoryModel(m_groupsModel);
    connect(m_groupsModel, &CategoryModel::finished, this, &ApperKCM::setupHomeModel);
    ui->homeView->setSpacing(10);
    ui->homeView->viewport()->setAttribute(Qt::WA_Hover);

    KFileItemDelegate *delegate = new KFileItemDelegate(this);
    delegate->setWrapMode(QTextOption::WordWrap);
    ui->homeView->setItemDelegate(delegate);

    // install the backend filters
    ui->filtersTB->setMenu(m_filtersMenu = new FiltersMenu(this));
    connect(m_filtersMenu, &FiltersMenu::filtersChanged, this, &ApperKCM::search);
    ui->filtersTB->setIcon(QIcon::fromTheme(QLatin1String("view-filter")));
    ApplicationSortFilterModel *proxy = ui->browseView->proxy();
    proxy->setApplicationFilter(m_filtersMenu->filterApplications());
    connect(m_filtersMenu, QOverload<bool>::of(&FiltersMenu::filterApplications), proxy, &ApplicationSortFilterModel::setApplicationFilter);

    //initialize the model, delegate, client and  connect it's signals
    m_browseModel = ui->browseView->model();

    // CHANGES TAB
    ui->changesView->viewport()->setAttribute(Qt::WA_Hover);
    m_changesModel = new PackageModel(this);
    auto changedProxy = new KCategorizedSortFilterProxyModel(this);
    changedProxy->setSourceModel(m_changesModel);
    changedProxy->setDynamicSortFilter(true);
    changedProxy->setCategorizedModel(true);
    changedProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    changedProxy->setSortRole(PackageModel::SortRole);
    changedProxy->sort(0);
    ui->changesView->setModel(changedProxy);
    auto changesDelegate = new ChangesDelegate(ui->changesView);
    changesDelegate->setExtendPixmapWidth(0);
    ui->changesView->setItemDelegate(changesDelegate);

    // Connect this signal to keep track of changes
    connect(m_browseModel, &PackageModel::changed, this, &ApperKCM::checkChanged);

    // packageUnchecked from changes model
    connect(m_changesModel, &PackageModel::packageUnchecked, m_changesModel, &PackageModel::removePackage);
    connect(m_changesModel, &PackageModel::packageUnchecked, m_browseModel, &PackageModel::uncheckPackageDefault);

    ui->reviewMessage->setIcon(QIcon::fromTheme(QLatin1String("edit-redo")));
    ui->reviewMessage->setText(i18n("Some software changes were made"));
    auto reviewAction = new QAction(i18n("Review"), this);
    connect(reviewAction, &QAction::triggered, this, &ApperKCM::showReviewPages);
    ui->reviewMessage->addAction(reviewAction);
    auto discardAction = new QAction(i18n("Discard"), this);
    connect(discardAction, &QAction::triggered, m_browseModel, &PackageModel::uncheckAll);
    ui->reviewMessage->addAction(discardAction);
    auto applyAction = new QAction(i18n("Apply"), this);
    connect(applyAction, &QAction::triggered, this, &ApperKCM::save);
    ui->reviewMessage->addAction(applyAction);
    ui->reviewMessage->setCloseButtonVisible(false);
    ui->reviewMessage->hide();
    connect(ui->reviewMessage, &KMessageWidget::showAnimationFinished, this, [this] () {
        if (!ui->reviewMessage->property("HasChanges").toBool()) {
            ui->reviewMessage->animatedHide();
        }
    });
    connect(ui->reviewMessage, &KMessageWidget::hideAnimationFinished, this, [this] () {
        if (ui->reviewMessage->property("HasChanges").toBool()) {
            ui->reviewMessage->animatedShow();
        }
    });

    auto menu = new QMenu(this);
    ui->settingsTB->setMenu(menu);
    ui->settingsTB->setIcon(QIcon::fromTheme(QLatin1String("preferences-other")));

    auto signalMapper = new QSignalMapper(this);
    connect(signalMapper, QOverload<const QString &>::of(&QSignalMapper::mapped), this, &ApperKCM::setPage);

    QAction *action;
    action = menu->addAction(QIcon::fromTheme(QLatin1String("view-history")), i18n("History"));
    signalMapper->setMapping(action, QLatin1String("history"));
    connect(action, &QAction::triggered, signalMapper, QOverload<>::of(&QSignalMapper::map));

    action = menu->addAction(QIcon::fromTheme(QLatin1String("preferences-other")), i18n("Settings"));
    signalMapper->setMapping(action, QLatin1String("settings"));
    connect(action, &QAction::triggered, signalMapper, QOverload<>::of(&QSignalMapper::map));

    auto helpMenu = new KHelpMenu(this, KAboutData::applicationData());
    menu->addMenu(helpMenu->menu());

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
        ui->actionFindFile->setIcon(QIcon::fromTheme(QLatin1String("document-open")));
        ui->actionFindDescription->setIcon(QIcon::fromTheme(QLatin1String("document-edit")));
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
        if (hasChanges) {
            if (!ui->reviewMessage->isHideAnimationRunning() && !ui->reviewMessage->isShowAnimationRunning()) {
                ui->reviewMessage->animatedShow();
            }
        } else {
            if (!ui->reviewMessage->isHideAnimationRunning() && !ui->reviewMessage->isShowAnimationRunning()) {
                ui->reviewMessage->animatedHide();
            }
        }
        ui->reviewMessage->setProperty("HasChanges", hasChanges);
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
        m_searchRole   = Transaction::RoleSearchName;
        m_searchString = ui->searchKLE->text();
        // create the main transaction
        search();
    }
}

void ApperKCM::on_actionFindDescription_triggered()
{
    setCurrentAction(ui->actionFindDescription);
    if (!ui->searchKLE->text().isEmpty()) {
        // cache the search
        m_searchRole   = Transaction::RoleSearchDetails;
        m_searchString = ui->searchKLE->text();
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
        const auto proxy = qobject_cast<const QSortFilterProxyModel*>(index.model());
        // If the cast failed it's the index came from browseView
        if (proxy) {
            m_searchParentCategory = proxy->mapToSource(index);
        } else {
            m_searchParentCategory = index;
        }

        // cache the search
        m_searchRole = static_cast<Transaction::Role>(index.data(CategoryModel::SearchRole).toUInt());
        qCDebug(APPER) << m_searchRole << index.data(CategoryModel::CategoryRole).toString();
        if (m_searchRole == Transaction::RoleResolve) {
#ifdef HAVE_APPSTREAM
            CategoryMatcher parser = index.data(CategoryModel::CategoryRole).value<CategoryMatcher>();
//            m_searchCategory = AppStream::instance()->findPkgNames(parser);
#endif // HAVE_APPSTREAM
        } else if (m_searchRole == Transaction::RoleSearchGroup) {
            if (index.data(CategoryModel::GroupRole).type() == QVariant::String) {
                QString category = index.data(CategoryModel::GroupRole).toString();
                if (category.startsWith(QLatin1Char('@')) ||
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
            setPage(QLatin1String("updates"));
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
    auto transaction = qobject_cast<PkTransaction*>(ui->stackedWidget->currentWidget());
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
                connect(m_settingsPage, &Settings::changed, this, &ApperKCM::checkChanged);
                connect(m_settingsPage, &Settings::refreshCache, this, &ApperKCM::refreshCache);
                ui->stackedWidget->addWidget(m_settingsPage);

                connect(ui->generalSettingsPB, &QPushButton::toggled, m_settingsPage, &Settings::showGeneralSettings);
                connect(ui->repoSettingsPB, &QPushButton::toggled, m_settingsPage, &Settings::showRepoSettings);
            }
            checkChanged();
//            ui->buttonBox->clear();
//            ui->buttonBox->setStandardButtons(QDialogButtonBox::Apply | QDialogButtonBox::Reset);
//            setButtons(KCModule::Default | KCModule::Apply);
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

            if (m_updaterPage == nullptr) {
                m_updaterPage = new Updater(m_roles, this);
                connect(m_updaterPage, &Updater::refreshCache, this, &ApperKCM::refreshCache);
                connect(m_updaterPage, &Updater::downloadSize, ui->downloadL, &QLabel::setText);
//                connect(m_updaterPage, &Updater::changed, this, &ApperKCM::checkChanged);
                ui->stackedWidget->addWidget(m_updaterPage);
                ui->checkUpdatesPB->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
                connect(ui->checkUpdatesPB, &QPushButton::clicked, this, &ApperKCM::refreshCache);

                ui->updatePB->setIcon(QIcon::fromTheme(QLatin1String("system-software-update")));
                connect(ui->updatePB, &QPushButton::clicked, this, &ApperKCM::save);
                connect(m_updaterPage, &Updater::changed, ui->updatePB, &QPushButton::setEnabled);
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
        connect(ui->searchKLE, &QLineEdit::textChanged, m_history, &TransactionHistory::setFilterRegExp);
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

void ApperKCM::showReviewPages()
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
        disconnect(m_searchTransaction, &Transaction::finished, ui->browseView->busyCursor(), &KPixmapSequenceOverlayPainter::stop);
        disconnect(m_searchTransaction, &Transaction::finished, this, &ApperKCM::finished);
        disconnect(m_searchTransaction, &Transaction::finished, m_browseModel, &PackageModel::finished);
        disconnect(m_searchTransaction, &Transaction::finished, m_browseModel, &PackageModel::fetchSizes);
        disconnect(m_searchTransaction, &Transaction::package, m_browseModel, &PackageModel::addNotSelectedPackage);
        disconnect(m_searchTransaction, &Transaction::errorCode, this, &ApperKCM::errorCode);
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
            if (m_searchGroupCategory.startsWith(QLatin1Char('@')) ||
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
        connect(m_searchTransaction, &Transaction::finished, ui->browseView, &BrowseView::enableExportInstalledPB);
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
        qCWarning(APPER) << "Search type not defined yet";
        emit caption();
        disconnectTransaction();
        m_searchTransaction = 0;
        return;
    }
    connect(m_searchTransaction, &Transaction::finished, ui->browseView->busyCursor(), &KPixmapSequenceOverlayPainter::stop);
    connect(m_searchTransaction, &Transaction::finished, this, &ApperKCM::finished);
    connect(m_searchTransaction, &Transaction::finished, m_browseModel, &PackageModel::finished);

    if (ui->browseView->isShowingSizes()) {
        connect(m_searchTransaction, &Transaction::finished, m_browseModel, &PackageModel::fetchSizes);
    }
    connect(m_searchTransaction, &Transaction::package, m_browseModel, &PackageModel::addNotSelectedPackage);
    connect(m_searchTransaction, &Transaction::errorCode, this, &ApperKCM::errorCode);

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

    auto transactionW = new PkTransactionWidget(this);
    connect(transactionW, &PkTransactionWidget::titleChangedProgress, this, &ApperKCM::caption);
    QPointer<PkTransaction> transaction = new PkTransaction(transactionW);
    Daemon::setHints(QLatin1String("cache-age=")+QString::number(m_cacheAge));
    transaction->refreshCache(m_forceRefreshCache);
    transactionW->setTransaction(transaction, Transaction::RoleRefreshCache);

    ui->stackedWidget->addWidget(transactionW);
    ui->stackedWidget->setCurrentWidget(transactionW);
    ui->stackedWidgetBar->setCurrentIndex(BAR_TITLE);
    ui->backTB->setEnabled(false);
    connect(transactionW, &PkTransactionWidget::titleChanged, ui->titleL, &QLabel::setText);

    QEventLoop loop;
    connect(transaction, &PkTransaction::finished, &loop, &QEventLoop::quit);

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
        setPage(QLatin1String("settings"));
    } else {
        setPage(QLatin1String("updates"));
    }

    QTimer::singleShot(0, this, &ApperKCM::checkChanged);
}

void ApperKCM::save()
{
    QWidget *currentWidget = ui->stackedWidget->currentWidget();
    if (currentWidget == m_settingsPage) {
        m_settingsPage->save();
    } else {
        ui->reviewMessage->hide();

        auto transactionW = new PkTransactionWidget(this);
        connect(transactionW, &PkTransactionWidget::titleChangedProgress, this, &ApperKCM::caption);
        QPointer<PkTransaction> transaction = new PkTransaction(transactionW);

        ui->stackedWidget->addWidget(transactionW);
        ui->stackedWidget->setCurrentWidget(transactionW);
        ui->stackedWidgetBar->setCurrentIndex(BAR_TITLE);
        ui->backTB->setEnabled(false);
        connect(transactionW, &PkTransactionWidget::titleChanged, ui->titleL, &QLabel::setText);
        emit changed(false);

        QEventLoop loop;
        connect(transaction, &PkTransaction::finished, &loop, &QEventLoop::quit);
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
            setPage(QLatin1String("updates"));
        } else {
            // install then remove packages
            search();
        }
        QTimer::singleShot(0, this, &ApperKCM::checkChanged);
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
//    KCModule::keyPressEvent(event);
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

#include "moc_ApperKCM.cpp"
