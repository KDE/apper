/***************************************************************************
 *   Copyright (C) 2009-2010 by Daniel Nicoletti                           *
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

#include <config.h>

#include "KpkFiltersMenu.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocale>

KpkFiltersMenu::KpkFiltersMenu(Enum::Filters filters, QWidget *parent)
 : QMenu(parent)
{
    // Loads the filter menu settings
    KConfig config("KPackageKit");
    KConfigGroup filterMenuGroup(&config, "FilterMenu");

    if (filters & Enum::FilterCollections || filters & Enum::FilterNotCollections) {
        QMenu *menuCollections = new QMenu(i18n("Collections"), this);
        addMenu(menuCollections);
        QActionGroup *collectionGroup = new QActionGroup(menuCollections);
        collectionGroup->setExclusive(true);

        QAction *collectionTrue = new QAction(i18n("Only collections"), collectionGroup);
        collectionTrue->setCheckable(true);
        m_filtersAction[collectionTrue] = Enum::FilterCollections;
        collectionGroup->addAction(collectionTrue);
        menuCollections->addAction(collectionTrue);
        m_actions << collectionTrue;

        QAction *collectionFalse = new QAction(i18n("Exclude collections"), collectionGroup);
        collectionFalse->setCheckable(true);
        m_filtersAction[collectionFalse] = Enum::FilterNotCollections;
        collectionGroup->addAction(collectionFalse);
        menuCollections->addAction(collectionFalse);
        m_actions << collectionFalse;
    }
    if (filters & Enum::FilterInstalled || filters & Enum::FilterNotInstalled) {
        // Installed
        QMenu *menuInstalled = new QMenu(i18n("Installed"), this);
        addMenu(menuInstalled);
        QActionGroup *installedGroup = new QActionGroup(menuInstalled);
        installedGroup->setExclusive(true);

        QAction *installedTrue = new QAction(i18n("Only installed"), installedGroup);
        installedTrue->setCheckable(true);
        m_filtersAction[installedTrue] = Enum::FilterInstalled;
        installedGroup->addAction(installedTrue);
        menuInstalled->addAction(installedTrue);
        m_actions << installedTrue;

        QAction *installedFalse = new QAction(i18n("Only available"), installedGroup);
        installedFalse->setCheckable(true);
        m_filtersAction[installedFalse] = Enum::FilterNotInstalled;
        installedGroup->addAction(installedFalse);
        menuInstalled->addAction(installedFalse);
        m_actions << installedFalse;

        QAction *installedNone = new QAction(i18n("No filter"), installedGroup);
        installedNone->setCheckable(true);
        installedNone->setChecked(true);
        installedGroup->addAction(installedNone);
        menuInstalled->addAction(installedNone);
        m_actions << installedNone;
    }
    if (filters & Enum::FilterDevelopment || filters & Enum::FilterNotDevelopment) {
        // Development
        QMenu *menuDevelopment = new QMenu(i18n("Development"), this);
        addMenu(menuDevelopment);
        QActionGroup *developmentGroup = new QActionGroup(menuDevelopment);
        developmentGroup->setExclusive(true);

        QAction *developmentTrue = new QAction(i18n("Only development"), developmentGroup);
        developmentTrue->setCheckable(true);
        m_filtersAction[developmentTrue] = Enum::FilterDevelopment;
        developmentGroup->addAction(developmentTrue);
        menuDevelopment->addAction(developmentTrue);
        m_actions << developmentTrue;

        QAction *developmentFalse = new QAction(i18n("Only end user files"), developmentGroup);
        developmentFalse->setCheckable(true);
        m_filtersAction[developmentFalse] = Enum::FilterNotDevelopment;
        developmentGroup->addAction(developmentFalse);
        menuDevelopment->addAction(developmentFalse);
        m_actions << developmentFalse;

        QAction *developmentNone = new QAction(i18n("No filter"), developmentGroup);
        developmentNone->setCheckable(true);
        developmentNone->setChecked(true);
        developmentGroup->addAction(developmentNone);
        menuDevelopment->addAction(developmentNone);
        m_actions << developmentNone;
    }
    if (filters & Enum::FilterGui || filters & Enum::FilterNotGui) {
        // Graphical
        QMenu *menuGui = new QMenu(i18n("Graphical"), this);
        addMenu(menuGui);
        QActionGroup *guiGroup = new QActionGroup(menuGui);
        guiGroup->setExclusive(true);

        QAction *guiTrue = new QAction(i18n("Only graphical"), guiGroup);
        guiTrue->setCheckable(true);
        m_filtersAction[guiTrue] = Enum::FilterGui;
        guiGroup->addAction(guiTrue);
        menuGui->addAction(guiTrue);
        m_actions << guiTrue;

        QAction *guiFalse = new QAction(i18n("Only text"), guiGroup);
        guiFalse->setCheckable(true);
        m_filtersAction[guiFalse] = Enum::FilterNotGui;
        guiGroup->addAction(guiFalse);
        menuGui->addAction(guiFalse);
        m_actions << guiFalse;

        QAction *guiNone = new QAction(i18n("No filter"), guiGroup);
        guiNone->setCheckable(true);
        guiNone->setChecked(true);
        guiGroup->addAction(guiNone);
        menuGui->addAction(guiNone);
        m_actions << guiNone;
    }
    if (filters & Enum::FilterFree || filters & Enum::FilterNotFree) {
        // Free
        QMenu *menuFree = new QMenu(i18nc("Filter for free packages", "Free"), this);
        addMenu(menuFree);
        QActionGroup *freeGroup = new QActionGroup(menuFree);
        freeGroup->setExclusive(true);

        QAction *freeTrue = new QAction(i18n("Only free software"), freeGroup);
        freeTrue->setCheckable(true);
        m_filtersAction[freeTrue] = Enum::FilterFree;
        freeGroup->addAction(freeTrue);
        menuFree->addAction(freeTrue);
        m_actions << freeTrue;

        QAction *freeFalse = new QAction(i18n("Only non-free software"), freeGroup);
        freeFalse->setCheckable(true);
        m_filtersAction[freeFalse] = Enum::FilterNotFree;
        freeGroup->addAction(freeFalse);
        menuFree->addAction(freeFalse);
        m_actions << freeFalse;

        QAction *freeNone = new QAction(i18n("No filter"), freeGroup);
        freeNone->setCheckable(true);
        freeNone->setChecked(true);
        freeGroup->addAction(freeNone);
        menuFree->addAction(freeNone);
        m_actions << freeNone;
    }
    if (filters & Enum::FilterArch || filters & Enum::FilterNotArch) {
        // Arch
        QMenu *menuArch = new QMenu(i18n("Architectures"), this);
        addMenu(menuArch);
        QActionGroup *archGroup = new QActionGroup(menuArch);
        archGroup->setExclusive(true);

        QAction *archTrue = new QAction(i18n("Only native architectures"), archGroup);
        archTrue->setCheckable(true);
        m_filtersAction[archTrue] = Enum::FilterArch;
        archGroup->addAction(archTrue);
        menuArch->addAction(archTrue);
        m_actions << archTrue;

        QAction *archFalse = new QAction(i18n("Only non-native architectures"), archGroup);
        archFalse->setCheckable(true);
        m_filtersAction[archFalse] = Enum::FilterNotArch;
        archGroup->addAction(archFalse);
        menuArch->addAction(archFalse);
        m_actions << archFalse;

        QAction *archNone = new QAction(i18n("No filter"), archGroup);
        archNone->setCheckable(true);
        archNone->setChecked(true);
        archGroup->addAction(archNone);
        menuArch->addAction(archNone);
        m_actions << archNone;
    }
    if (filters & Enum::FilterSource || filters & Enum::FilterNotSource) {
        // Source
        QMenu *menuSource = new QMenu(i18nc("Filter for source packages", "Source"), this);
        addMenu(menuSource);
        QActionGroup *sourceGroup = new QActionGroup(menuSource);
        sourceGroup->setExclusive(true);

        QAction *sourceTrue = new QAction(i18n("Only sourcecode"), sourceGroup);
        sourceTrue->setCheckable(true);
        m_filtersAction[sourceTrue] = Enum::FilterSource;
        sourceGroup->addAction(sourceTrue);
        menuSource->addAction(sourceTrue);
        m_actions << sourceTrue;

        QAction *sourceFalse = new QAction(i18n("Only non-sourcecode"), sourceGroup);
        sourceFalse->setCheckable(true);
        m_filtersAction[sourceFalse] = Enum::FilterNotSource;
        sourceGroup->addAction(sourceFalse);
        menuSource->addAction(sourceFalse);
        m_actions << sourceFalse;

        QAction *sourceNone = new QAction(i18n("No filter"), sourceGroup);
        sourceNone->setCheckable(true);
        sourceNone->setChecked(true);
        sourceGroup->addAction(sourceNone);
        menuSource->addAction(sourceNone);
        m_actions << sourceNone;
    }
    if (filters & Enum::FilterBasename) {
        addSeparator();
        QAction *basename = new QAction(i18n("Hide subpackages"), this);
        basename->setCheckable(true);
        basename->setToolTip(i18n("Only show one package, not subpackages"));
        m_filtersAction[basename] = Enum::FilterBasename;
        addAction(basename);

        m_actions << basename;
    }
    if (filters & Enum::FilterNewest) {
        addSeparator();
        QAction *newest = new QAction(i18n("Only newest packages"), this);
        newest->setCheckable(true);
        newest->setChecked(filterMenuGroup.readEntry("FilterNewest", true));
        newest->setToolTip(i18n("Only show the newest available package"));
        m_filtersAction[newest] = Enum::FilterNewest;
        addAction(newest);

        m_actions << newest;
    }

#ifdef HAVE_APPINSTALL
    addSeparator();
    m_applications = new QAction(i18n("Hide packages"), this);
    m_applications->setCheckable(true);
    m_applications->setChecked(filterMenuGroup.readEntry("HidePackages", false));
    m_applications->setToolTip(i18n("Only show applications"));
    addAction(m_applications);
    connect(m_applications, SIGNAL(triggered(bool)),
            this, SLOT(filterAppTriggered(bool)));
#endif //HAVE_APPINSTALL
}

KpkFiltersMenu::~KpkFiltersMenu()
{
    KConfig config("KPackageKit");
    KConfigGroup filterMenuGroup(&config, "FilterMenu");

    // For usability we will only save ViewInGroups settings and Newest filter,
    // - The user might get angry when he does not find any packages because he didn't
    //   see that a filter is set by config

    // This entry does not depend on the backend it's ok to call this pointer
//     filterMenuGroup.writeEntry("ViewInGroups", m_filtersMenu->actionGrouped());

    // This entry does not depend on the backend it's ok to call this pointer
    filterMenuGroup.writeEntry("FilterNewest",
                               static_cast<bool>(filters() & Enum::FilterNewest));
#ifdef HAVE_APPINSTALL
    filterMenuGroup.writeEntry("HidePackages", m_applications->isChecked());
#endif //HAVE_APPINSTALL
}

QString KpkFiltersMenu::filterApplications() const
{
#ifdef HAVE_APPINSTALL
    return m_applications->isChecked() ? "a" : QString();
#else
    return QString();
#endif //HAVE_APPINSTALL
}

void KpkFiltersMenu::filterAppTriggered(bool checked)
{
    emit filterApplications(checked ? "a" : QString());
}

Enum::Filters KpkFiltersMenu::filters() const
{
    Enum::Filters filters;
    bool filterSet = false;
    foreach (QAction * const action, m_actions) {
        if (action->isChecked()) {
            if (m_filtersAction.contains(action)) {
                filters |= m_filtersAction[action];
                filterSet = true;
            }
        }
    }
    if (!filterSet) {
        filters = Enum::NoFilter;
    }
    return filters;
}

#include "KpkFiltersMenu.moc"
