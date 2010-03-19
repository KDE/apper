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

#include "KpkAddRm.h"

#include <KLocale>
#include <KStandardDirs>
#include <KMessageBox>

#include <QPalette>
#include <QColor>
#include <QDBusConnection>

#include <KpkPackageDetails.h>
#include <KpkReviewChanges.h>
#include <KpkPackageModel.h>
#include <KpkDelegate.h>
#include <KpkStrings.h>
#include <KpkIcons.h>

#include <KDebug>

#define UNIVERSAL_PADDING 6

KCONFIGGROUP_DECLARE_ENUM_QOBJECT(Enum, Filter)

KpkAddRm::KpkAddRm(QWidget *parent)
 : QWidget(parent)
 , m_currentAction(0)
 , m_mTransRuning(false)
 , m_findIcon("edit-find")
 , m_cancelIcon("dialog-cancel")
 , m_filterIcon("view-filter")
{
    setupUi( this );

    // create our toolbar
    gridLayout_2->addWidget(toolBar = new QToolBar);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QString locale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
    Client::instance()->setHints("locale=" + locale);

    // Create a new daemon
    m_client = Client::instance();

    //initialize the model, delegate, client and  connect it's signals
    packageView->setItemDelegate(pkg_delegate = new KpkDelegate(packageView));
    packageView->setModel(m_pkg_model_main = new KpkPackageModel(this, packageView));
    packageView->viewport()->setAttribute(Qt::WA_Hover);

    // check to see if the backend support these actions
    m_roles = m_client->actions();
    // Connect this signal anyway so users that have backend that
    // do not support install or remove can be informed properly
    connect(m_pkg_model_main, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)),
            this, SLOT(checkChanged()));

    m_findMenu = new QMenu(this);
    // find is just a generic name in case we don't have any search method
    m_genericActionK = new KToolBarPopupAction(m_findIcon, i18n("Find"), this);
    toolBar->addAction(m_genericActionK);

    // Add actions that the backend supports
    if (m_roles & Enum::RoleSearchName) {
        m_findMenu->addAction(actionFindName);
        setCurrentAction(actionFindName);
    }
    if (m_roles & Enum::RoleSearchDetails) {
        m_findMenu->addAction(actionFindDescription);
        if (!m_currentAction) {
            setCurrentAction(actionFindDescription);
        }
    }
    if (m_roles & Enum::RoleSearchFile) {
        m_findMenu->addAction(actionFindFile);
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
        if (m_findMenu->actions().size() > 1) {
            m_currentAction->setVisible(false);
            m_genericActionK->setMenu(m_findMenu);
        } else {
            m_currentAction->setVisible(true);
            toolBar->removeAction(m_genericActionK);
            toolBar->addAction(m_currentAction);
        }
        connect(m_genericActionK, SIGNAL(triggered()),
                this, SLOT(genericActionKTriggered()));
    }

    m_groupsModel = new QStandardItemModel(this);
    groupsCB->setModel(m_groupsModel);

    //initialize the groups
    Enum::Groups groups = m_client->groups();
    // Add Text search entry
    QStandardItem *groupItem;
    m_groupsModel->appendRow(groupItem = new QStandardItem(i18n("Text search")));
    groupItem->setData(AllPackages, Qt::UserRole);

    m_groupsModel->appendRow(listOfChangesItem = new QStandardItem(i18n("List of changes")));
    listOfChangesItem->setEnabled(false);
    listOfChangesItem->setData(ListOfChanges, Qt::UserRole);

    if (groups.size()) {
        // An empty small item
        m_groupsModel->appendRow(groupItem = new QStandardItem(QString()));
        groupItem->setEnabled(false);
        groupItem->setSizeHint(QSize(5, 5));
        m_groupsModel->appendRow(groupItem = new QStandardItem(i18ncp("Groups of packages", "Group:", "Groups:", groups.size())));
        groupItem->setEnabled(false);

        foreach (const Enum::Group &group, groups) {
            if (group != Enum::UnknownGroup) {
                m_groupsModel->appendRow(groupItem = new QStandardItem(KpkStrings::groups(group)));
                groupItem->setData(Group, Qt::UserRole);
                groupItem->setData(group, Group);
                groupItem->setIcon(KpkIcons::groupsIcon(group));
                if (!(m_roles & Enum::RoleSearchGroup)) {
                    groupItem->setSelectable(false);
                }
            }
        }
    }

    // install the backend filters
    filterMenu(m_client->filters());
    filtersTB->setIcon(m_filterIcon);

    // set focus on the search lineEdit
    searchKLE->setFocus(Qt::OtherFocusReason);
    transactionBar->setBehaviors(KpkTransactionBar::AutoHide | KpkTransactionBar::HideCancel);
}

void KpkAddRm::genericActionKTriggered()
{
    m_currentAction->trigger();
}

void KpkAddRm::setCurrentAction(QAction *action)
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

void KpkAddRm::setCurrentActionEnabled(bool state)
{
    if (m_currentAction) {
        m_currentAction->setEnabled(state);
    }
    m_genericActionK->setEnabled(state);
}

void KpkAddRm::setCurrentActionCancel(bool cancel)
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

void KpkAddRm::checkChanged()
{
    if (m_pkg_model_main->selectedPackages().size() > 0) {
      emit changed(true);
      listOfChangesItem->setEnabled(true);
    } else {
      emit changed(false);
      listOfChangesItem->setEnabled(false);
    }
}

void KpkAddRm::on_packageView_pressed(const QModelIndex &index)
{
    if (index.column() == 0) {
        QSharedPointer<PackageKit::Package>p = m_pkg_model_main->package(index);
        if (p) {
            if (pkg_delegate->isExtended(index)) {
                pkg_delegate->contractItem(index);
            } else {
                pkg_delegate->extendItem(new KpkPackageDetails(p, m_roles), index);
            }
        }
    }
}

void KpkAddRm::errorCode(PackageKit::Enum::Error error, const QString &details)
{
    if (error != Enum::ErrorTransactionCancelled) {
        KMessageBox::detailedSorry(this, KpkStrings::errorMessage(error), details, KpkStrings::error(error), KMessageBox::Notify);
    }
}

KpkAddRm::~KpkAddRm()
{
    KConfig config("KPackageKit");
    KConfigGroup filterMenuGroup(&config, "FilterMenu");

    // For usability we will only save ViewInGroups settings and Newest filter,
    // - The user might get angry when he does not find any packages because he didn't
    //   see that a filter is set by config

    // This entry does not depend on the backend it's ok to call this pointer
    filterMenuGroup.writeEntry("ViewInGroups", m_actionViewInGroups->isChecked());

    // This entry does not depend on the backend it's ok to call this pointer
    if (m_client->filters() & Enum::FilterNewest) {
        filterMenuGroup.writeEntry("FilterNewest",
                                   static_cast<bool>(filters() & Enum::FilterNewest));
    }
}

void KpkAddRm::on_actionFindName_triggered()
{
    setCurrentAction(actionFindName);
    if (m_mTransRuning) {
        m_pkClient_main->cancel();
    } else if (!searchKLE->text().isEmpty()) {
        // cache the search
        m_searchRole    = Enum::RoleSearchName;
        m_searchString  = searchKLE->text();
        m_searchFilters = filters();
        // create the main transaction
        search();
    }
}

void KpkAddRm::on_actionFindDescription_triggered()
{
    setCurrentAction(actionFindDescription);
    if (m_mTransRuning) {
        m_pkClient_main->cancel();
    } else if (!searchKLE->text().isEmpty()) {
        // cache the search
        m_searchRole    = Enum::RoleSearchDetails;
        m_searchString  = searchKLE->text();
        m_searchFilters = filters();
        // create the main transaction
        search();
    }
}

void KpkAddRm::on_actionFindFile_triggered()
{
    setCurrentAction(actionFindFile);
    if (m_mTransRuning) {
        m_pkClient_main->cancel();
    } else if (!searchKLE->text().isEmpty()) {
        // cache the search
        m_searchRole    = Enum::RoleSearchFile;
        m_searchString  = searchKLE->text();
        m_searchFilters = filters();
        // create the main transaction
        search();
    }
}

void KpkAddRm::on_groupsCB_currentIndexChanged(int index)
{
    if (groupsCB->itemData(index, Qt::UserRole).isValid()) {
        switch (static_cast<ItemType>(groupsCB->itemData(index, Qt::UserRole).toUInt())) {
        case AllPackages :
            // contract and delete and details widgets
            pkg_delegate->contractAll();
            // cleans the models
            m_pkg_model_main->clear();
            break;
        case ListOfChanges :
            // contract and delete and details widgets
            pkg_delegate->contractAll();
            // cleans the models
            m_pkg_model_main->clear();
            foreach (QSharedPointer<PackageKit::Package>pkg, m_pkg_model_main->selectedPackages()) {
                m_pkg_model_main->addPackage(pkg);
            }
            break;
        case Group :
            // cache the search
            m_searchRole    = Enum::RoleSearchGroup;
            m_searchGroup   = static_cast<Enum::Group>(groupsCB->itemData(index, Group).toUInt());
            m_searchFilters = filters();
            // create the main transaction
            search();
        }
    }
}

void KpkAddRm::search()
{
    // Check to see if "list of changes" is selected
    // if so refresh it and do nothing
    int index = groupsCB->currentIndex();
    if (groupsCB->itemData(index, Qt::UserRole).isValid() &&
        ListOfChanges == static_cast<ItemType>(groupsCB->itemData(index, Qt::UserRole).toUInt()))
    {
        on_groupsCB_currentIndexChanged(index);
        return;
    } else {
        // search
        if (m_searchRole == Enum::RoleSearchName) {
            m_pkClient_main = m_client->searchNames(m_searchString, m_searchFilters );
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
        transactionBar->addTransaction(m_pkClient_main);
        // contract and delete and details widgets
        pkg_delegate->contractAll();
        // cleans the models
        m_pkg_model_main->clear();
        m_mTransRuning = true;
    }
}

void KpkAddRm::connectTransaction(Transaction *transaction)
{
    connect(transaction, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
            m_pkg_model_main, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
    connect(transaction, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
            this, SLOT(finished(PackageKit::Enum::Exit, uint)));
    connect(transaction, SIGNAL(errorCode(PackageKit::Enum::Error, const QString &)),
            this, SLOT(errorCode(PackageKit::Enum::Error, const QString &)));
    setCurrentActionEnabled(transaction->allowCancel());
}

void KpkAddRm::changed()
{
    Transaction *trans = qobject_cast<Transaction*>(sender());
    setCurrentActionEnabled(trans->allowCancel());
}

void KpkAddRm::save()
{
    QPointer<KpkReviewChanges> frm = new KpkReviewChanges(m_pkg_model_main->selectedPackages(), this);
    frm->setTitle(i18n("Review Changes"));
    if (frm->exec() == QDialog::Accepted) {
        m_pkg_model_main->uncheckAll();
    } else {
        QTimer::singleShot(0, this, SLOT(checkChanged()));
    }
    delete frm;
    search();
}

void KpkAddRm::load()
{
    m_pkg_model_main->uncheckAll();
}

void KpkAddRm::finished(PackageKit::Enum::Exit status, uint runtime)
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

void KpkAddRm::filterMenu(Enum::Filters filters)
{
    m_filtersQM = new QMenu(this);
    filtersTB->setMenu(m_filtersQM);

    // Loads the filter menu settings
    KConfig config("KPackageKit");
    KConfigGroup filterMenuGroup(&config, "FilterMenu");

    if (filters & Enum::FilterCollections || filters & Enum::FilterNotCollections) {
        QMenu *menuCollections = new QMenu(i18n("Collections"), m_filtersQM);
        m_filtersQM->addMenu(menuCollections);
        QActionGroup *collectionGroup = new QActionGroup(menuCollections);
        collectionGroup->setExclusive(true);

        QAction *collectionTrue = new QAction(i18n("Only collections"), collectionGroup);
        collectionTrue->setCheckable(true);
        m_filtersAction[collectionTrue] = Enum::FilterCollections;
        collectionGroup->addAction(collectionTrue);
        menuCollections->addAction(collectionTrue);
        actions << collectionTrue;

        QAction *collectionFalse = new QAction(i18n("Exclude collections"), collectionGroup);
        collectionFalse->setCheckable(true);
        m_filtersAction[collectionFalse] = Enum::FilterNotCollections;
        collectionGroup->addAction(collectionFalse);
        menuCollections->addAction(collectionFalse);
        actions << collectionFalse;
    }
    if (filters & Enum::FilterInstalled || filters & Enum::FilterNotInstalled) {
        // Installed
        QMenu *menuInstalled = new QMenu(i18n("Installed"), m_filtersQM);
        m_filtersQM->addMenu(menuInstalled);
        QActionGroup *installedGroup = new QActionGroup(menuInstalled);
        installedGroup->setExclusive(true);

        QAction *installedTrue = new QAction(i18n("Only installed"), installedGroup);
        installedTrue->setCheckable(true);
        m_filtersAction[installedTrue] = Enum::FilterInstalled;
        installedGroup->addAction(installedTrue);
        menuInstalled->addAction(installedTrue);
        actions << installedTrue;

        QAction *installedFalse = new QAction(i18n("Only available"), installedGroup);
        installedFalse->setCheckable(true);
        m_filtersAction[installedFalse] = Enum::FilterNotInstalled;
        installedGroup->addAction(installedFalse);
        menuInstalled->addAction(installedFalse);
        actions << installedFalse;

        QAction *installedNone = new QAction(i18n("No filter"), installedGroup);
        installedNone->setCheckable(true);
        installedNone->setChecked(true);
        installedGroup->addAction(installedNone);
        menuInstalled->addAction(installedNone);
        actions << installedNone;
    }
    if (filters & Enum::FilterDevelopment || filters & Enum::FilterNotDevelopment) {
        // Development
        QMenu *menuDevelopment = new QMenu(i18n("Development"), m_filtersQM);
        m_filtersQM->addMenu(menuDevelopment);
        QActionGroup *developmentGroup = new QActionGroup(menuDevelopment);
        developmentGroup->setExclusive(true);

        QAction *developmentTrue = new QAction(i18n("Only development"), developmentGroup);
        developmentTrue->setCheckable(true);
        m_filtersAction[developmentTrue] = Enum::FilterDevelopment;
        developmentGroup->addAction(developmentTrue);
        menuDevelopment->addAction(developmentTrue);
        actions << developmentTrue;

        QAction *developmentFalse = new QAction(i18n("Only end user files"), developmentGroup);
        developmentFalse->setCheckable(true);
        m_filtersAction[developmentFalse] = Enum::FilterNotDevelopment;
        developmentGroup->addAction(developmentFalse);
        menuDevelopment->addAction(developmentFalse);
        actions << developmentFalse;

        QAction *developmentNone = new QAction(i18n("No filter"), developmentGroup);
        developmentNone->setCheckable(true);
        developmentNone->setChecked(true);
        developmentGroup->addAction(developmentNone);
        menuDevelopment->addAction(developmentNone);
        actions << developmentNone;
    }
    if (filters & Enum::FilterGui || filters & Enum::FilterNotGui) {
        // Graphical
        QMenu *menuGui = new QMenu(i18n("Graphical"), m_filtersQM);
        m_filtersQM->addMenu(menuGui);
        QActionGroup *guiGroup = new QActionGroup(menuGui);
        guiGroup->setExclusive(true);

        QAction *guiTrue = new QAction(i18n("Only graphical"), guiGroup);
        guiTrue->setCheckable(true);
        m_filtersAction[guiTrue] = Enum::FilterGui;
        guiGroup->addAction(guiTrue);
        menuGui->addAction(guiTrue);
        actions << guiTrue;

        QAction *guiFalse = new QAction(i18n("Only text"), guiGroup);
        guiFalse->setCheckable(true);
        m_filtersAction[guiFalse] = Enum::FilterNotGui;
        guiGroup->addAction(guiFalse);
        menuGui->addAction(guiFalse);
        actions << guiFalse;

        QAction *guiNone = new QAction(i18n("No filter"), guiGroup);
        guiNone->setCheckable(true);
        guiNone->setChecked(true);
        guiGroup->addAction(guiNone);
        menuGui->addAction(guiNone);
        actions << guiNone;
    }
    if (filters & Enum::FilterFree || filters & Enum::FilterNotFree) {
        // Free
        QMenu *menuFree = new QMenu(i18nc("Filter for free packages", "Free"), m_filtersQM);
        m_filtersQM->addMenu(menuFree);
        QActionGroup *freeGroup = new QActionGroup(menuFree);
        freeGroup->setExclusive(true);

        QAction *freeTrue = new QAction(i18n("Only free software"), freeGroup);
        freeTrue->setCheckable(true);
        m_filtersAction[freeTrue] = Enum::FilterFree;
        freeGroup->addAction(freeTrue);
        menuFree->addAction(freeTrue);
        actions << freeTrue;

        QAction *freeFalse = new QAction(i18n("Only non-free software"), freeGroup);
        freeFalse->setCheckable(true);
        m_filtersAction[freeFalse] = Enum::FilterNotFree;
        freeGroup->addAction(freeFalse);
        menuFree->addAction(freeFalse);
        actions << freeFalse;

        QAction *freeNone = new QAction(i18n("No filter"), freeGroup);
        freeNone->setCheckable(true);
        freeNone->setChecked(true);
        freeGroup->addAction(freeNone);
        menuFree->addAction(freeNone);
        actions << freeNone;
    }
    if (filters & Enum::FilterArch || filters & Enum::FilterNotArch) {
        // Arch
        QMenu *menuArch = new QMenu(i18n("Architectures"), m_filtersQM);
        m_filtersQM->addMenu(menuArch);
        QActionGroup *archGroup = new QActionGroup(menuArch);
        archGroup->setExclusive(true);

        QAction *archTrue = new QAction(i18n("Only native architectures"), archGroup);
        archTrue->setCheckable(true);
        m_filtersAction[archTrue] = Enum::FilterArch;
        archGroup->addAction(archTrue);
        menuArch->addAction(archTrue);
        actions << archTrue;

        QAction *archFalse = new QAction(i18n("Only non-native architectures"), archGroup);
        archFalse->setCheckable(true);
        m_filtersAction[archFalse] = Enum::FilterNotArch;
        archGroup->addAction(archFalse);
        menuArch->addAction(archFalse);
        actions << archFalse;

        QAction *archNone = new QAction(i18n("No filter"), archGroup);
        archNone->setCheckable(true);
        archNone->setChecked(true);
        archGroup->addAction(archNone);
        menuArch->addAction(archNone);
        actions << archNone;
    }
    if (filters & Enum::FilterSource || filters & Enum::FilterNotSource) {
        // Source
        QMenu *menuSource = new QMenu(i18nc("Filter for source packages", "Source"), m_filtersQM);
        m_filtersQM->addMenu(menuSource);
        QActionGroup *sourceGroup = new QActionGroup(menuSource);
        sourceGroup->setExclusive(true);

        QAction *sourceTrue = new QAction(i18n("Only sourcecode"), sourceGroup);
        sourceTrue->setCheckable(true);
        m_filtersAction[sourceTrue] = Enum::FilterSource;
        sourceGroup->addAction(sourceTrue);
        menuSource->addAction(sourceTrue);
        actions << sourceTrue;

        QAction *sourceFalse = new QAction(i18n("Only non-sourcecode"), sourceGroup);
        sourceFalse->setCheckable(true);
        m_filtersAction[sourceFalse] = Enum::FilterNotSource;
        sourceGroup->addAction(sourceFalse);
        menuSource->addAction(sourceFalse);
        actions << sourceFalse;

        QAction *sourceNone = new QAction(i18n("No filter"), sourceGroup);
        sourceNone->setCheckable(true);
        sourceNone->setChecked(true);
        sourceGroup->addAction(sourceNone);
        menuSource->addAction(sourceNone);
        actions << sourceNone;
    }
    if (filters & Enum::FilterBasename) {
        m_filtersQM->addSeparator();
        QAction *basename = new QAction(i18n("Hide subpackages"), m_filtersQM);
        basename->setCheckable(true);
        basename->setToolTip(i18n("Only show one package, not subpackages"));
        m_filtersAction[basename] = Enum::FilterBasename;
        m_filtersQM->addAction(basename);

        actions << basename;
    }
    if (filters & Enum::FilterNewest) {
        m_filtersQM->addSeparator();
        QAction *newest = new QAction(i18n("Only newest packages"), m_filtersQM);
        newest->setCheckable(true);
        newest->setChecked(filterMenuGroup.readEntry("FilterNewest", true));
        newest->setToolTip(i18n("Only show the newest available package"));
        m_filtersAction[newest] = Enum::FilterNewest;
        m_filtersQM->addAction(newest);

        actions << newest;
    }

    m_filtersQM->addSeparator();

    m_actionViewInGroups = new QAction(i18n("View in groups"), m_filtersQM);
    m_actionViewInGroups->setCheckable(true);
    m_filtersQM->addAction(m_actionViewInGroups);
    m_actionViewInGroups->setToolTip(i18n("Display packages in groups according to status"));
    if (filterMenuGroup.readEntry("ViewInGroups", false)) {
        m_pkg_model_main->setGrouped(true);
        packageViewSetRootIsDecorated(true);
        m_actionViewInGroups->setChecked(true);
    }

    connect(m_actionViewInGroups, SIGNAL(toggled(bool)),
            m_pkg_model_main, SLOT(setGrouped(bool)));
    connect(m_actionViewInGroups, SIGNAL(toggled(bool)),
            this, SLOT(packageViewSetRootIsDecorated(bool)));
}

void KpkAddRm::packageViewSetRootIsDecorated(bool value)
{
    // contract and delete and details widgets
    pkg_delegate->contractAll();
    packageView->setRootIsDecorated(value);
}

Enum::Filters KpkAddRm::filters()
{
    Enum::Filters filters;
    bool filterSet = false;
    for (int i = 0 ; i < actions.size(); ++i) {
        if (actions.at(i)->isChecked()) {
            if (m_filtersAction.contains(actions.at(i))) {
                filters |= m_filtersAction[actions.at(i)];
                filterSet = true;
            }
        }
    }
    if (!filterSet) {
        filters = Enum::NoFilter;
    }
    return m_searchFilters = filters;
}

void KpkAddRm::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateColumnsWidth();
}

bool KpkAddRm::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::KeyPress:
            // use bracktes to don't cross initialization og keyEvent
            {
                QKeyEvent *ke = static_cast<QKeyEvent *>(event);
                if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
                    // special tab handling here
                    m_currentAction->trigger();
                    return true;
                }
            }
            break;
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

#include "KpkAddRm.moc"
