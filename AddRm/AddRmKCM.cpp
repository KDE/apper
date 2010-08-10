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

#include <KLocale>
#include <KStandardDirs>
#include <KMessageBox>
#include <KFileItemDelegate>
#include "CategoryDrawer.h"
#include <KCategorizedSortFilterProxyModel>

#include <QPalette>
#include <QColor>
#include <QDBusConnection>

#include <KpkReviewChanges.h>
#include <KpkPackageModel.h>
#include <KpkDelegate.h>
#include <KpkStrings.h>
#include <KpkIcons.h>

#include <KDebug>

#define UNIVERSAL_PADDING 6

KCONFIGGROUP_DECLARE_ENUM_QOBJECT(Enum, Filter)

K_PLUGIN_FACTORY(KPackageKitFactory, registerPlugin<AddRmKCM>();)
K_EXPORT_PLUGIN(KPackageKitFactory("kcm_kpk_addrm"))

AddRmKCM::AddRmKCM(QWidget *parent, const QVariantList &args)
 : KCModule(KPackageKitFactory::componentData(), parent, args),
   m_currentAction(0),
   m_mTransRuning(false),
   m_installedModel(0),
   m_databaseChanged(true),
   m_findIcon("edit-find"),
   m_cancelIcon("dialog-cancel")
{
    KAboutData *aboutData;
    aboutData = new KAboutData("kpackagekit",
                               "kpackagekit",
                               ki18n("Add and Remove Software"),
                               KPK_VERSION,
                               ki18n("KDE interface for managing software"),
                               KAboutData::License_GPL,
                               ki18n("(C) 2008-2010 Daniel Nicoletti"));
    KGlobal::locale()->insertCatalog("kpackagekit");
    setAboutData(aboutData);

    setupUi(this);

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

    m_viewLayout = new QStackedLayout(stackedWidget);
    m_viewLayout->addWidget(homeView);
    m_viewLayout->addWidget(packageView);


    //initialize the model, delegate, client and  connect it's signals
    KpkDelegate *pkgDelegate = new KpkDelegate(packageView);
    connect(pkgDelegate, SIGNAL(showExtendItem(const QModelIndex &)),
            this, SLOT(showExtendItem(const QModelIndex &)));
    packageView->setItemDelegate(pkgDelegate);
    packageView->setModel(m_packageModel = new KpkPackageModel(this, packageView));
    packageView->viewport()->setAttribute(Qt::WA_Hover);

    // Connect this signal anyway so users that have backend that
    // do not support install or remove can be informed properly
    connect(m_packageModel, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)),
            this, SLOT(checkChanged()));

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
    m_groupsModel = new QStandardItemModel(this);

    CategoryDrawer *drawer = new CategoryDrawer;
    homeView->setSpacing(KDialog::spacingHint());
    homeView->setCategoryDrawer(drawer);
    homeView->viewport()->setAttribute(Qt::WA_Hover);

    //initialize the groups
    Enum::Groups groups = m_client->groups();
    QStandardItem *groupItem;
    foreach (const Enum::Group &group, groups) {
        if (group != Enum::UnknownGroup) {
            groupItem = new QStandardItem(KpkStrings::groups(group));
            groupItem->setData(Group, Qt::UserRole);
            groupItem->setData(group, Group);
            groupItem->setData(i18n("Groups"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
            groupItem->setData(0, KCategorizedSortFilterProxyModel::CategorySortRole);
            groupItem->setIcon(KpkIcons::groupsIcon(group));
            if (!(m_roles & Enum::RoleSearchGroup)) {
                groupItem->setSelectable(false);
            }
            m_groupsModel->appendRow(groupItem);
        }
    }

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

    // set focus on the search lineEdit
    searchKLE->setFocus(Qt::OtherFocusReason);
    transactionBar->setBehaviors(KpkTransactionBar::AutoHide | KpkTransactionBar::HideCancel);

    // INSTALLED TAB
    pkgDelegate = new KpkDelegate(installedView);
    installedView->setItemDelegate(pkgDelegate);
    connect(pkgDelegate, SIGNAL(showExtendItem(const QModelIndex &)),
            this, SLOT(showExtendItem(const QModelIndex &)));
    m_installedModel = new KpkPackageModel(this, installedView);
    installedView->setModel(m_installedModel);
    installedView->viewport()->setAttribute(Qt::WA_Hover);
    tabWidget->setTabIcon(1, KIcon("drive-harddisk"));
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
        actionFindFile->setIcon(m_findIcon);
        actionFindDescription->setIcon(m_findIcon);
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
    if (m_packageModel->selectedPackages().size() > 0) {
        emit changed(true);
    } else {
        emit changed(false);
    }
}

void AddRmKCM::showExtendItem(const QModelIndex &index)
{
    if (index.column() == 0) {
        KpkDelegate *delegate = qobject_cast<KpkDelegate*>(sender());
        const KpkPackageModel *model = qobject_cast<const KpkPackageModel*>(index.model());
        QSharedPointer<PackageKit::Package> package = model->package(index);
        if (package) {
            if (delegate->isExtended(index)) {
                delegate->contractItem(index);
            } else {
                delegate->extendItem(new KpkPackageDetails(package, m_roles), index);
            }
        }
    }
}

void AddRmKCM::errorCode(PackageKit::Enum::Error error, const QString &details)
{
    if (error != Enum::ErrorTransactionCancelled) {
        KMessageBox::detailedSorry(this, KpkStrings::errorMessage(error), details, KpkStrings::error(error), KMessageBox::Notify);
    }
}

AddRmKCM::~AddRmKCM()
{
    KConfig config("KPackageKit");
    KConfigGroup filterMenuGroup(&config, "FilterMenu");

    // For usability we will only save ViewInGroups settings and Newest filter,
    // - The user might get angry when he does not find any packages because he didn't
    //   see that a filter is set by config

    // This entry does not depend on the backend it's ok to call this pointer
//     filterMenuGroup.writeEntry("ViewInGroups", m_filtersMenu->actionGrouped());

    // This entry does not depend on the backend it's ok to call this pointer
    if (m_client->filters() & Enum::FilterNewest) {
        filterMenuGroup.writeEntry("FilterNewest",
                                   static_cast<bool>(m_filtersMenu->filters() & Enum::FilterNewest));
    }
}

void AddRmKCM::on_actionFindName_triggered()
{
    setCurrentAction(actionFindName);
    if (m_mTransRuning) {
        m_pkClient_main->cancel();
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
    if (m_mTransRuning) {
        m_pkClient_main->cancel();
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
    if (m_mTransRuning) {
        m_pkClient_main->cancel();
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
    KpkDelegate *delegate = qobject_cast<KpkDelegate*>(packageView->itemDelegate());
    if (index.data(Qt::UserRole).isValid()) {
        switch (static_cast<ItemType>(index.data(Qt::UserRole).toUInt())) {
        case AllPackages :
            // contract and delete and details widgets
            delegate->contractAll();
            // cleans the models
            m_packageModel->clear();
            break;
        case ListOfChanges :
            // contract and delete and details widgets
            delegate->contractAll();
            // cleans the models
            m_packageModel->clear();
            foreach (QSharedPointer<PackageKit::Package>pkg, m_packageModel->selectedPackages()) {
                m_packageModel->addPackage(pkg);
            }
            break;
        case Group :
            // cache the search
            m_searchRole    = Enum::RoleSearchGroup;
            m_searchGroup   = static_cast<Enum::Group>(index.data(Group).toUInt());
            m_searchFilters = m_filtersMenu->filters();
            // create the main transaction
            search();
        }
    }
}

void AddRmKCM::on_backTB_clicked()
{
    m_viewLayout->setCurrentIndex(0);
    backTB->setEnabled(false);
}

void AddRmKCM::on_tabWidget_currentChanged(int index)
{
    if (index == 1 && m_databaseChanged == true) {
        m_databaseChanged = false;
        Transaction *trans = m_client->getPackages(Enum::FilterInstalled);
        connect(trans, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                m_installedModel, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
    }
}

void AddRmKCM::search()
{
    m_viewLayout->setCurrentIndex(1);
    backTB->setEnabled(true);
    // Check to see if "list of changes" is selected
    // if so refresh it and do nothing
    QModelIndex index;
    if (!homeView->selectionModel()->selectedIndexes().isEmpty()) {
        index = homeView->selectionModel()->selectedIndexes().first();
    }

    if (index.data(Qt::UserRole).isValid() &&
        ListOfChanges == static_cast<ItemType>(index.data(Qt::UserRole).toUInt()))
    {
        on_homeView_activated(index);
        return;
    } else {
        // search
        if (m_searchRole == Enum::RoleSearchName) {
            m_pkClient_main = m_client->searchNames(m_searchString, m_searchFilters);
        } else if (m_searchRole == Enum::RoleSearchDetails) {
            m_pkClient_main = m_client->searchDetails(m_searchString, m_searchFilters);
        } else if (m_searchRole == Enum::RoleSearchFile) {
            m_pkClient_main = m_client->searchFiles(m_searchString, m_searchFilters);
        } else if (m_searchRole == Enum::RoleSearchGroup) {
            m_pkClient_main = m_client->searchGroups(m_searchGroup, m_searchFilters);
        } else {
            kWarning() << "Search type not implemented yet";
            return;
        }
    }

    if (m_pkClient_main->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(m_pkClient_main->error()));
        setCurrentActionEnabled(true);
    } else {
        setCurrentActionCancel(true);
        connectTransaction(m_pkClient_main);
        // contract and delete and details widgets
        KpkDelegate *delegate = qobject_cast<KpkDelegate*>(packageView->itemDelegate());
        delegate->contractAll();
        // cleans the models
        m_packageModel->clear();
        m_mTransRuning = true;
    }
}

void AddRmKCM::connectTransaction(Transaction *transaction)
{
    connect(transaction, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
            m_packageModel, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
    connect(transaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            this, SLOT(finished(PackageKit::Enum::Exit, uint)));
    connect(transaction, SIGNAL(errorCode(PackageKit::Enum::Error, const QString &)),
            this, SLOT(errorCode(PackageKit::Enum::Error, const QString &)));
    setCurrentActionEnabled(transaction->allowCancel());
    transactionBar->addTransaction(transaction);
}

void AddRmKCM::changed()
{
    Transaction *trans = qobject_cast<Transaction*>(sender());
    setCurrentActionEnabled(trans->allowCancel());
}

void AddRmKCM::save()
{
    QPointer<KpkReviewChanges> frm = new KpkReviewChanges(m_packageModel->selectedPackages(), this);
    frm->setTitle(i18n("Review Changes"));
    if (frm->exec() == QDialog::Accepted) {
//         m_packageModel->uncheckAll();//TODO
    } else {
        QTimer::singleShot(0, this, SLOT(checkChanged()));
    }
    delete frm;
    search();
}

void AddRmKCM::load()
{
//     m_packageModel->uncheckAll();
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
    m_mTransRuning = false;
}

#include "AddRmKCM.moc"

