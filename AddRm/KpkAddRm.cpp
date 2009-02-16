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
#include <KpkIcons.h>

#include <KDebug>

#define UNIVERSAL_PADDING 6

KCONFIGGROUP_DECLARE_ENUM_QOBJECT(Client, Filter)

KpkAddRm::KpkAddRm(QWidget *parent)
 : QWidget(parent), m_currentAction(0), m_mTransRuning(false),  m_findIcon("edit-find"),
   m_cancelIcon("dialog-cancel"), m_filterIcon("view-filter")
{
    setupUi( this );

    // create our toolbar
    gridLayout_2->addWidget(toolBar = new QToolBar);
    toolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    Client::instance()->setLocale(KGlobal::locale()->language() + "." + KGlobal::locale()->encoding());

    // Create a new daemon
    m_client = Client::instance();

    //initialize the model, delegate, client and  connect it's signals
    packageView->setItemDelegate(pkg_delegate = new KpkDelegate(packageView));
    packageView->setModel(m_pkg_model_main = new KpkPackageModel(this, packageView));
    packageView->viewport()->setAttribute(Qt::WA_Hover);

    // check to see if the backend support these actions
    m_actions = m_client->getActions();
    if (m_actions.contains(Client::ActionInstallPackages) || m_actions.contains(Client::ActionRemovePackages)) {
        connect(m_pkg_model_main, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)),
                this, SLOT(checkChanged()) );
    }

    m_findMenu = new QMenu(this);
    setActionsDefaults();
    // find is just a generic name in case we don't have any search method
    m_genericActionK = new KToolBarPopupAction(m_findIcon, i18n("Find"), this);
    toolBar->addAction(m_genericActionK);

    // Add actions that the backend supports
    if (m_actions.contains(Client::ActionSearchName)) {
        m_findMenu->addAction(actionFindName);
        setCurrentAction(actionFindName);
    }
    if (m_actions.contains(Client::ActionSearchDetails)) {
        m_findMenu->addAction(actionFindDescription);
        if (!m_currentAction) {
            setCurrentAction(actionFindDescription);
        }
    }
    if (m_actions.contains(Client::ActionSearchFile)) {
        m_findMenu->addAction(actionFindFile);
        if (!m_currentAction) {
            setCurrentAction(actionFindFile);
        }
    }

    // If no action was set we can't use this search
    if (m_currentAction == 0) {
        m_genericActionK->setEnabled(false);
    } else {
        // Check to see if we need the KToolBarPopupAction
        if (m_findMenu->actions().size() > 1) {
            m_currentAction->setVisible(false);
            m_genericActionK->setMenu(m_findMenu);
        } else {
            toolBar->removeAction(m_genericActionK);
            toolBar->addAction(m_currentAction);
        }
        connect(m_genericActionK, SIGNAL(triggered()),
                this, SLOT(genericActionKTriggered()));
    }

    if (!m_actions.contains(Client::ActionSearchGroup)) {
        groupsCB->setEnabled(false);
    }

    //initialize the groups
    // TODO add non selectable groups if Client::ActionSearchGroup
    // is not supported
    foreach (Client::Group group, m_client->getGroups()) {
        groupsCB->addItem(KpkIcons::groupsIcon(group), KpkStrings::groups(group), group);
    }

    // install the backend filters
    filterMenu(m_client->getFilters());
    filtersTB->setIcon(m_filterIcon);

    // set focus on the search lineEdit
    searchKLE->setFocus(Qt::OtherFocusReason);
    transactionBar->setBehaviors(KpkTransactionBar::AutoHide);
}

void KpkAddRm::genericActionKTriggered()
{
    m_currentAction->trigger();
}

void KpkAddRm::setCurrentAction(QAction *action)
{
    kDebug();
    // just load the new action if it changes this
    // also ensures that our menu has more than one action
    if (m_currentAction != action) {
        // ensures the current action was created
        if (m_currentAction) {
            m_currentAction->setVisible(true);
        }
        m_currentAction = action;
        // hides the item from the list
        m_currentAction->setVisible(false);
        // copy data from the curront action
        m_genericActionK->setText(m_currentAction->text());
        m_genericActionK->setIcon(m_currentAction->icon());
    }
}

void KpkAddRm::setActionsDefaults()
{
    actionFindName->setText(i18n("Find by &Name"));
    actionFindFile->setText(i18n("Find by f&ile name"));
    actionFindDescription->setText(i18n("Find by &description"));
    // Define actions icon
    actionFindFile->setIcon(m_findIcon);
    actionFindDescription->setIcon(m_findIcon);
    actionFindName->setIcon(m_findIcon);
    if (m_currentAction) {
        m_genericActionK->setText(m_currentAction->text());
        actionFindName->setIcon(m_currentAction->icon());
    }
}

void KpkAddRm::checkChanged()
{
    if (m_pkg_model_main->selectedPackages().size() > 0) {
      emit changed(true);
    } else {
      emit changed(false);
    }
}

void KpkAddRm::on_packageView_pressed(const QModelIndex &index)
{
    if (index.column() == 0) {
        Package *p = m_pkg_model_main->package(index);
        if (p) {
            if (pkg_delegate->isExtended(index)) {
                pkg_delegate->contractItem(index);
            } else {
                pkg_delegate->extendItem(new KpkPackageDetails(p, m_actions), index);
            }
        }
    }
}

void KpkAddRm::errorCode(PackageKit::Client::ErrorType error, const QString &details)
{
    KMessageBox::detailedSorry(this, KpkStrings::errorMessage(error), details, KpkStrings::error(error), KMessageBox::Notify);
}

KpkAddRm::~KpkAddRm()
{
    KConfig config("KPackageKit");
    KConfigGroup filterMenuGroup(&config, "FilterMenu");

    kDebug() << "Saving filters settings";

//     filterMenuGroup.writeEntry("OnlyNewestPackages", m_actionNewestOnly->isChecked());
//     filterMenuGroup.writeEntry("HideSubpackages", m_actionBasename->isChecked());
    // This entry does not depend on the backend it's ok to call this pointer
    filterMenuGroup.writeEntry("ViewInGroups", m_actionViewInGroups->isChecked());
    // We call filters so we can save all of them, although i'm not sure
    // most people will want to save them
    // TODO make the filter bold when it's has not the default filters
    //      this way people can see that their old filter were loaded
//     writeEntry(filterMenuGroup, "SelectedFilters", Client::FilterNotCollections);
//     writeEntry(filterMenuGroup, "SelectedFilters", filters().toList());
}

void KpkAddRm::on_actionFindName_triggered()
{
    kDebug();
    setCurrentAction(actionFindName);
    if (m_mTransRuning) {
        m_pkClient_main->cancel();
    } else if (!searchKLE->text().isEmpty()) {
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

void KpkAddRm::on_actionFindDescription_triggered()
{
    kDebug();
    setCurrentAction(actionFindDescription);
    if (m_mTransRuning) {
        m_pkClient_main->cancel();
    } else if (!searchKLE->text().isEmpty()) {
        // cache the search
        m_searchAction = Client::ActionSearchDetails;
        m_searchString = searchKLE->text();
        m_searchFilters = filters();
        // select "All Packages"
        groupsCB->setCurrentIndex(0);
        // create the main transaction
        search();
    }
}

void KpkAddRm::on_actionFindFile_triggered()
{
    kDebug();
    setCurrentAction(actionFindFile);
    if (m_mTransRuning) {
        m_pkClient_main->cancel();
    } else if (!searchKLE->text().isEmpty()) {
        // cache the search
        m_searchAction = Client::ActionSearchFile;
        m_searchString = searchKLE->text();
        m_searchFilters = filters();
        // select "All Packages"
        groupsCB->setCurrentIndex(0);
        // create the main transaction
        search();
    }
}

void KpkAddRm::on_groupsCB_currentIndexChanged(int index)
{
    if (groupsCB->itemData(index, Qt::UserRole).isValid()) {
        // cache the search
        m_searchAction = Client::ActionSearchGroup;
        m_searchGroup = (Client::Group) groupsCB->itemData(index, Qt::UserRole).toUInt();
        m_searchFilters = filters();
        // create the main transaction
        search();
    }
}

void KpkAddRm::search()
{
    // search
    if (m_searchAction == Client::ActionSearchName) {
        m_pkClient_main = m_client->searchName(m_searchString, m_searchFilters );
    } else if (m_searchAction == Client::ActionSearchDetails) {
        m_pkClient_main = m_client->searchDetails(m_searchString, m_searchFilters);
    } else if (m_searchAction == Client::ActionSearchFile) {
        m_pkClient_main = m_client->searchFile(m_searchString, m_searchFilters);
    } else if (m_searchAction == Client::ActionSearchGroup) {
        m_pkClient_main = m_client->searchGroup(m_searchGroup, m_searchFilters);
    } else {
        kWarning() << "Search type not implemented yet";
        return;
    }

    connectTransaction(m_pkClient_main);
    transactionBar->addTransaction(m_pkClient_main);
    // contract and delete and details widgets
    pkg_delegate->contractAll();
    // cleans the models
    m_pkg_model_main->clear();
    m_mTransRuning = true;
}

void KpkAddRm::connectTransaction(Transaction *transaction)
{
    connect(transaction, SIGNAL(package(PackageKit::Package *)),
            m_pkg_model_main, SLOT(addPackage(PackageKit::Package *)));
    connect(transaction, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
            this, SLOT(finished(PackageKit::Transaction::ExitStatus, uint)));
    connect(transaction, SIGNAL(errorCode(PackageKit::Client::ErrorType, const QString &)),
            this, SLOT(errorCode(PackageKit::Client::ErrorType, const QString &)));
}

void KpkAddRm::message(PackageKit::Client::MessageType message, const QString &details)
{
    qDebug() << "Error code: " << message << " two: " << details;
}

void KpkAddRm::save()
{
    KpkReviewChanges *frm = new KpkReviewChanges( m_pkg_model_main->selectedPackages(), this);
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

void KpkAddRm::finished(PackageKit::Transaction::ExitStatus status, uint runtime)
{
    Q_UNUSED(runtime)
    Q_UNUSED(status)
    m_mTransRuning = false;
}

void KpkAddRm::filterMenu(Client::Filters filters)
{
    m_filtersQM = new QMenu(this);
    filtersTB->setMenu(m_filtersQM);

    // Loads the filter menu settings
    KConfig config("KPackageKit");
    KConfigGroup filterMenuGroup(&config, "FilterMenu");

    if (!filters.isEmpty()) {
        if (filters.contains(Client::FilterCollections) || filters.contains(Client::FilterNotCollections)) {
            QMenu *menuCollections = new QMenu(i18n("Collections"), m_filtersQM);
            m_filtersQM->addMenu(menuCollections);
            QActionGroup *collectionGroup = new QActionGroup(menuCollections);
            collectionGroup->setExclusive(true);

            QAction *collectionTrue = new QAction(i18n("Only collections"), collectionGroup);
            collectionTrue->setCheckable(true);
            m_filtersAction[collectionTrue] = Client::FilterCollections;
            collectionGroup->addAction(collectionTrue);
            menuCollections->addAction(collectionTrue);
            actions << collectionTrue;

            QAction *collectionFalse = new QAction(i18n("Exclude collections"), collectionGroup);
            collectionFalse->setCheckable(true);
            m_filtersAction[collectionFalse] = Client::FilterNotCollections;
            collectionGroup->addAction(collectionFalse);
            menuCollections->addAction(collectionFalse);
            actions << collectionFalse;
        }
        if (filters.contains(Client::FilterInstalled)  || filters.contains(Client::FilterNotInstalled)) {
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
//             }

//             if ( filters.contains(Client::FilterNotInstalled) ) {
                QAction *installedFalse = new QAction(i18n("Only available"), installedGroup);
                installedFalse->setCheckable(true);
                m_filtersAction[installedFalse] = Client::FilterNotInstalled;
                installedGroup->addAction(installedFalse);
                menuInstalled->addAction(installedFalse);
                actions << installedFalse;
//             }

            QAction *installedNone = new QAction(i18n("No filter"), installedGroup);
            installedNone->setCheckable(true);
            installedNone->setChecked(true);
            installedGroup->addAction(installedNone);
            menuInstalled->addAction(installedNone);
            actions << installedNone;
        }
        if (filters.contains(Client::FilterDevelopment) || filters.contains(Client::FilterNotDevelopment)) {
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
//             }

//             if ( filters.contains(Client::FilterNotDevelopment) ) {
                QAction *developmentFalse = new QAction(i18n("Only end user files"), developmentGroup);
                developmentFalse->setCheckable(true);
                m_filtersAction[developmentFalse] = Client::FilterNotDevelopment;
                developmentGroup->addAction(developmentFalse);
                menuDevelopment->addAction(developmentFalse);
                actions << developmentFalse;
//             }

            QAction *developmentNone = new QAction(i18n("No filter"), developmentGroup);
            developmentNone->setCheckable(true);
            developmentNone->setChecked(true);
            developmentGroup->addAction(developmentNone);
            menuDevelopment->addAction(developmentNone);
            actions << developmentNone;
        }
        if (filters.contains(Client::FilterGui) || filters.contains(Client::FilterNotGui)) {
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
//             }

//             if ( filters.contains(Client::FilterNotGui) ) {
                QAction *guiFalse = new QAction(i18n("Only text"), guiGroup);
                guiFalse->setCheckable(true);
                m_filtersAction[guiFalse] = Client::FilterNotGui;
                guiGroup->addAction(guiFalse);
                menuGui->addAction(guiFalse);
                actions << guiFalse;
//             }

            QAction *guiNone = new QAction(i18n("No filter"), guiGroup);
            guiNone->setCheckable(true);
            guiNone->setChecked(true);
            guiGroup->addAction(guiNone);
            menuGui->addAction(guiNone);
            actions << guiNone;
        }
        if (filters.contains(Client::FilterFree) || filters.contains(Client::FilterNotFree)) {
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
//             }

//             if ( filters.contains(Client::FilterNotFree) ) {
                QAction *freeFalse = new QAction(i18n("Only non-free software"), freeGroup);
                freeFalse->setCheckable(true);
                m_filtersAction[freeFalse] = Client::FilterNotFree;
                freeGroup->addAction(freeFalse);
                menuFree->addAction(freeFalse);
                actions << freeFalse;
//             }

            QAction *freeNone = new QAction(i18n("No filter"), freeGroup);
            freeNone->setCheckable(true);
            freeNone->setChecked(true);
            freeGroup->addAction(freeNone);
            menuFree->addAction(freeNone);
            actions << freeNone;
        }
        if (filters.contains(Client::FilterArch) || filters.contains(Client::FilterNotArch)) {
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
//             }

//             if ( filters.contains(Client::FilterNotArch) ) {
                QAction *archFalse = new QAction(i18n("Only non-native architectures"), archGroup);
                archFalse->setCheckable(true);
                m_filtersAction[archFalse] = Client::FilterNotArch;
                archGroup->addAction(archFalse);
                menuArch->addAction(archFalse);
                actions << archFalse;
//             }

            QAction *archNone = new QAction(i18n("No filter"), archGroup);
            archNone->setCheckable(true);
            archNone->setChecked(true);
            archGroup->addAction(archNone);
            menuArch->addAction(archNone);
            actions << archNone;
        }
        if (filters.contains(Client::FilterSource) || filters.contains(Client::FilterNotSource)) {
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
//             }

//             if ( filters.contains(Client::FilterNotSource) ) {
                QAction *sourceFalse = new QAction(i18n("Only non-sourcecode"), sourceGroup);
                sourceFalse->setCheckable(true);
                m_filtersAction[sourceFalse] = Client::FilterNotSource;
                sourceGroup->addAction(sourceFalse);
                menuSource->addAction(sourceFalse);
                actions << sourceFalse;
//             }

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

Client::Filters KpkAddRm::filters()
{
    Client::Filters buffer;
    for (int i = 0 ; i < actions.size(); ++i) {
        if (actions.at(i)->isChecked()) {
            if (m_filtersAction.contains(actions.at(i))) {
                buffer << m_filtersAction[actions.at(i)];
            }
        }
    }
    if (buffer.size() == 0) {
        buffer << Client::NoFilter;
    }
    return m_searchFilters = buffer;
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
