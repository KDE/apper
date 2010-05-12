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
#include "KpkFiltersMenu.h"
#include "KpkPackageDetails.h"

#include <KLocale>
#include <KStandardDirs>
#include <KMessageBox>

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
    filtersTB->setMenu(m_filtersMenu = new KpkFiltersMenu(m_client->filters(), this));
    connect(m_filtersMenu, SIGNAL(grouped(bool)),
            m_pkg_model_main, SLOT(setGrouped(bool)));
    connect(m_filtersMenu, SIGNAL(grouped(bool)),
            this, SLOT(packageViewSetRootIsDecorated(bool)));
    m_pkg_model_main->setGrouped(m_filtersMenu->actionGrouped());
    packageViewSetRootIsDecorated(m_filtersMenu->actionGrouped());
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
    filterMenuGroup.writeEntry("ViewInGroups", m_filtersMenu->actionGrouped());

    // This entry does not depend on the backend it's ok to call this pointer
    if (m_client->filters() & Enum::FilterNewest) {
        filterMenuGroup.writeEntry("FilterNewest",
                                   static_cast<bool>(m_filtersMenu->filters() & Enum::FilterNewest));
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
        m_searchFilters = m_filtersMenu->filters();
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
        m_searchFilters = m_filtersMenu->filters();
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
        m_searchFilters = m_filtersMenu->filters();
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
            m_searchFilters = m_filtersMenu->filters();
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

void KpkAddRm::packageViewSetRootIsDecorated(bool value)
{
    // contract and delete and details widgets
    pkg_delegate->contractAll();
    packageView->setRootIsDecorated(value);
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
