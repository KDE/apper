/***************************************************************************
 *   Copyright (C) 2009-2011 by Daniel Nicoletti                           *
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

#include <config.h>

#include "FiltersMenu.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

FiltersMenu::FiltersMenu(QWidget *parent)
 : QMenu(parent)
{
    // Loads the filter menu settings
    KConfig config("apper");
    KConfigGroup filterMenuGroup(&config, "FilterMenu");

    QMenu *menuCollections = new QMenu(i18n("Collections"), this);
    connect(menuCollections, SIGNAL(triggered(QAction*)),
            this, SIGNAL(filtersChanged()));
    connect(this, SIGNAL(enableCollections(bool)),
            addMenu(menuCollections), SLOT(setVisible(bool)));
    QActionGroup *collectionGroup = new QActionGroup(menuCollections);
    collectionGroup->setExclusive(true);

    QAction *collectionTrue = new QAction(i18n("Only collections"), collectionGroup);
    collectionTrue->setCheckable(true);
    m_filtersAction[collectionTrue] = Transaction::FilterCollections;
    collectionGroup->addAction(collectionTrue);
    menuCollections->addAction(collectionTrue);
    m_actions << collectionTrue;

    QAction *collectionFalse = new QAction(i18n("Exclude collections"), collectionGroup);
    collectionFalse->setCheckable(true);
    m_filtersAction[collectionFalse] = Transaction::FilterNotCollections;
    collectionGroup->addAction(collectionFalse);
    menuCollections->addAction(collectionFalse);
    m_actions << collectionFalse;

    // Installed
    QMenu *menuInstalled = new QMenu(i18n("Installed"), this);
    connect(menuInstalled, SIGNAL(triggered(QAction*)),
            this, SIGNAL(filtersChanged()));
    connect(this, SIGNAL(enableInstalled(bool)),
            addMenu(menuInstalled), SLOT(setVisible(bool)));
    addMenu(menuInstalled);
    QActionGroup *installedGroup = new QActionGroup(menuInstalled);
    installedGroup->setExclusive(true);

    QAction *installedTrue = new QAction(i18n("Only installed"), installedGroup);
    installedTrue->setCheckable(true);
    m_filtersAction[installedTrue] = Transaction::FilterInstalled;
    installedGroup->addAction(installedTrue);
    menuInstalled->addAction(installedTrue);
    m_actions << installedTrue;

    QAction *installedFalse = new QAction(i18n("Only available"), installedGroup);
    installedFalse->setCheckable(true);
    m_filtersAction[installedFalse] = Transaction::FilterNotInstalled;
    installedGroup->addAction(installedFalse);
    menuInstalled->addAction(installedFalse);
    m_actions << installedFalse;

    QAction *installedNone = new QAction(i18n("No filter"), installedGroup);
    installedNone->setCheckable(true);
    installedNone->setChecked(true);
    installedGroup->addAction(installedNone);
    menuInstalled->addAction(installedNone);
    m_actions << installedNone;

    // Development
    QMenu *menuDevelopment = new QMenu(i18n("Development"), this);
    connect(menuDevelopment, SIGNAL(triggered(QAction*)),
            this, SIGNAL(filtersChanged()));
    connect(this, SIGNAL(enableDevelopment(bool)),
            addMenu(menuDevelopment), SLOT(setVisible(bool)));
    addMenu(menuDevelopment);
    QActionGroup *developmentGroup = new QActionGroup(menuDevelopment);
    developmentGroup->setExclusive(true);

    QAction *developmentTrue = new QAction(i18n("Only development"), developmentGroup);
    developmentTrue->setCheckable(true);
    m_filtersAction[developmentTrue] = Transaction::FilterDevel;
    developmentGroup->addAction(developmentTrue);
    menuDevelopment->addAction(developmentTrue);
    m_actions << developmentTrue;

    QAction *developmentFalse = new QAction(i18n("Only end user files"), developmentGroup);
    developmentFalse->setCheckable(true);
    m_filtersAction[developmentFalse] = Transaction::FilterNotDevel;
    developmentGroup->addAction(developmentFalse);
    menuDevelopment->addAction(developmentFalse);
    m_actions << developmentFalse;

    QAction *developmentNone = new QAction(i18n("No filter"), developmentGroup);
    developmentNone->setCheckable(true);
    developmentNone->setChecked(true);
    developmentGroup->addAction(developmentNone);
    menuDevelopment->addAction(developmentNone);
    m_actions << developmentNone;

    // Graphical
    QMenu *menuGui = new QMenu(i18n("Graphical"), this);
    connect(menuGui, SIGNAL(triggered(QAction*)),
            this, SIGNAL(filtersChanged()));
    connect(this, SIGNAL(enableGraphical(bool)),
            addMenu(menuGui), SLOT(setVisible(bool)));
    addMenu(menuGui);
    QActionGroup *guiGroup = new QActionGroup(menuGui);
    guiGroup->setExclusive(true);

    QAction *guiTrue = new QAction(i18n("Only graphical"), guiGroup);
    guiTrue->setCheckable(true);
    m_filtersAction[guiTrue] = Transaction::FilterGui;
    guiGroup->addAction(guiTrue);
    menuGui->addAction(guiTrue);
    m_actions << guiTrue;

    QAction *guiFalse = new QAction(i18n("Only text"), guiGroup);
    guiFalse->setCheckable(true);
    m_filtersAction[guiFalse] = Transaction::FilterNotGui;
    guiGroup->addAction(guiFalse);
    menuGui->addAction(guiFalse);
    m_actions << guiFalse;

    QAction *guiNone = new QAction(i18n("No filter"), guiGroup);
    guiNone->setCheckable(true);
    guiNone->setChecked(true);
    guiGroup->addAction(guiNone);
    menuGui->addAction(guiNone);
    m_actions << guiNone;

    // Free
    QMenu *menuFree = new QMenu(i18nc("Filter for free packages", "Free"), this);
    connect(menuFree, SIGNAL(triggered(QAction*)),
            this, SIGNAL(filtersChanged()));
    connect(this, SIGNAL(enableFree(bool)),
            addMenu(menuFree), SLOT(setVisible(bool)));
    addMenu(menuFree);
    QActionGroup *freeGroup = new QActionGroup(menuFree);
    freeGroup->setExclusive(true);

    QAction *freeTrue = new QAction(i18n("Only free software"), freeGroup);
    freeTrue->setCheckable(true);
    m_filtersAction[freeTrue] = Transaction::FilterFree;
    freeGroup->addAction(freeTrue);
    menuFree->addAction(freeTrue);
    m_actions << freeTrue;

    QAction *freeFalse = new QAction(i18n("Only non-free software"), freeGroup);
    freeFalse->setCheckable(true);
    m_filtersAction[freeFalse] = Transaction::FilterNotFree;
    freeGroup->addAction(freeFalse);
    menuFree->addAction(freeFalse);
    m_actions << freeFalse;

    QAction *freeNone = new QAction(i18n("No filter"), freeGroup);
    freeNone->setCheckable(true);
    freeNone->setChecked(true);
    freeGroup->addAction(freeNone);
    menuFree->addAction(freeNone);
    m_actions << freeNone;

    // Supported
    QMenu *menuSupported = new QMenu(i18nc("Filter for supported packages", "Supported"), this);
    connect(menuSupported, SIGNAL(triggered(QAction*)),
            this, SIGNAL(filtersChanged()));
    connect(this, SIGNAL(enableSupported(bool)),
            addMenu(menuSupported), SLOT(setVisible(bool)));
    addMenu(menuSupported);
    QActionGroup *supportedGroup = new QActionGroup(menuSupported);
    supportedGroup->setExclusive(true);

    QAction *supportedTrue = new QAction(i18n("Only supported software"), supportedGroup);
    supportedTrue->setCheckable(true);
    m_filtersAction[supportedTrue] = Transaction::FilterSupported;
    supportedGroup->addAction(supportedTrue);
    menuSupported->addAction(supportedTrue);
    m_actions << supportedTrue;

    QAction *supportedFalse = new QAction(i18n("Only non-supported software"), supportedGroup);
    supportedFalse->setCheckable(true);
    m_filtersAction[supportedFalse] = Transaction::FilterNotSupported;
    supportedGroup->addAction(supportedFalse);
    menuSupported->addAction(supportedFalse);
    m_actions << supportedFalse;

    QAction *supportedNone = new QAction(i18n("No filter"), supportedGroup);
    supportedNone->setCheckable(true);
    supportedNone->setChecked(true);
    supportedGroup->addAction(supportedNone);
    menuSupported->addAction(supportedNone);
    m_actions << supportedNone;

    // Source
    QMenu *menuSource = new QMenu(i18nc("Filter for source packages", "Source"), this);
    connect(menuSource, SIGNAL(triggered(QAction*)),
            this, SIGNAL(filtersChanged()));
    connect(this, SIGNAL(enableSource(bool)),
            addMenu(menuSource), SLOT(setVisible(bool)));
    addMenu(menuSource);
    QActionGroup *sourceGroup = new QActionGroup(menuSource);
    sourceGroup->setExclusive(true);

    QAction *sourceTrue = new QAction(i18n("Only sourcecode"), sourceGroup);
    sourceTrue->setCheckable(true);
    m_filtersAction[sourceTrue] = Transaction::FilterSource;
    sourceGroup->addAction(sourceTrue);
    menuSource->addAction(sourceTrue);
    m_actions << sourceTrue;

    QAction *sourceFalse = new QAction(i18n("Only non-sourcecode"), sourceGroup);
    sourceFalse->setCheckable(true);
    m_filtersAction[sourceFalse] = Transaction::FilterNotSource;
    sourceGroup->addAction(sourceFalse);
    menuSource->addAction(sourceFalse);
    m_actions << sourceFalse;

    QAction *sourceNone = new QAction(i18n("No filter"), sourceGroup);
    sourceNone->setCheckable(true);
    sourceNone->setChecked(true);
    sourceGroup->addAction(sourceNone);
    menuSource->addAction(sourceNone);
    m_actions << sourceNone;

    // Basename, Newest, Arch separator
    connect(this, SIGNAL(enableBasenameNewestArchSeparator(bool)),
            addSeparator(), SLOT(setVisible(bool)));

    QAction *basename = new QAction(i18n("Hide Subpackages"), this);
    connect(basename, SIGNAL(triggered()),
            this, SIGNAL(filtersChanged()));
    connect(this, SIGNAL(enableBasename(bool)),
            basename, SLOT(setVisible(bool)));
    basename->setCheckable(true);
    basename->setToolTip(i18n("Only show one package, not subpackages"));
    m_filtersAction[basename] = Transaction::FilterBasename;
    m_actions << basename;

    QAction *newest = new QAction(i18n("Only Newest Packages"), this);
    connect(newest, SIGNAL(triggered()),
            this, SIGNAL(filtersChanged()));
    connect(this, SIGNAL(enableNewest(bool)),
            newest, SLOT(setVisible(bool)));
    newest->setCheckable(true);
    newest->setChecked(filterMenuGroup.readEntry("FilterNewest", true));
    newest->setToolTip(i18n("Only show the newest available package"));
    m_filtersAction[newest] = Transaction::FilterNewest;
    m_actions << newest;

    QAction *native = new QAction(i18n("Only Native Packages"), this);
    connect(native, SIGNAL(triggered()),
            this, SIGNAL(filtersChanged()));
    connect(this, SIGNAL(enableArch(bool)),
            native, SLOT(setVisible(bool)));
    native->setCheckable(true);
    native->setChecked(filterMenuGroup.readEntry("FilterNative", true));
    native->setToolTip(i18n("Only show packages matching the machine architecture"));
    m_filtersAction[native] = Transaction::FilterArch;
    m_actions << native;

#ifdef HAVE_APPSTREAM
    addSeparator();
    m_applications = new QAction(i18n("Only Show Applications"), this);
    m_applications->setCheckable(true);
    m_applications->setChecked(filterMenuGroup.readEntry("HidePackages", false));
    m_applications->setToolTip(i18n("Hide packages that are not applications"));
    addAction(m_applications);
    connect(m_applications, SIGNAL(triggered(bool)),
            this, SIGNAL(filterApplications(bool)));
#endif // HAVE_APPSTREAM
}

FiltersMenu::~FiltersMenu()
{
    KConfig config("apper");
    KConfigGroup filterMenuGroup(&config, "FilterMenu");

    // For usability we will only save ViewInGroups settings and Newest filter,
    // - The user might get angry when he does not find any packages because he didn't
    //   see that a filter is set by config

    // This entry does not depend on the backend it's ok to call this pointer
//     filterMenuGroup.writeEntry("ViewInGroups", m_filtersMenu->actionGrouped());

    // This entry does not depend on the backend it's ok to call this pointer
    filterMenuGroup.writeEntry("FilterNewest",
                               static_cast<bool>(filters() & Transaction::FilterNewest));
    // This entry does not depend on the backend it's ok to call this pointer
    filterMenuGroup.writeEntry("FilterNative",
                               static_cast<bool>(filters() & Transaction::FilterArch));
#ifdef HAVE_APPSTREAM
    filterMenuGroup.writeEntry("HidePackages", m_applications->isChecked());
#endif // HAVE_APPSTREAM
}

void FiltersMenu::setFilters(Transaction::Filters filters)
{
    emit enableCollections(filters & Transaction::FilterCollections || filters & Transaction::FilterNotCollections);
    emit enableInstalled(filters & Transaction::FilterInstalled || filters & Transaction::FilterNotInstalled);
    emit enableDevelopment(filters & Transaction::FilterDevel || filters & Transaction::FilterNotDevel);
    emit enableGraphical(filters & Transaction::FilterGui || filters & Transaction::FilterNotGui);
    emit enableFree(filters & Transaction::FilterFree || filters & Transaction::FilterNotFree);
    emit enableSupported(filters & Transaction::FilterSupported || filters & Transaction::FilterNotSupported);
    emit enableSource(filters & Transaction::FilterSource || filters & Transaction::FilterNotSource);
    emit enableBasenameNewestArchSeparator(filters & Transaction::FilterBasename ||
                                           filters & Transaction::FilterNewest ||
                                           filters & Transaction::FilterArch);
    emit enableBasename(filters & Transaction::FilterBasename);
    emit enableNewest(filters & Transaction::FilterNewest);
    emit enableArch(filters & Transaction::FilterArch);
}

bool FiltersMenu::filterApplications() const
{
#ifdef HAVE_APPSTREAM
    return m_applications->isChecked();
#else
    return false;
#endif // HAVE_APPSTREAM
}

Transaction::Filters FiltersMenu::filters() const
{
    Transaction::Filters filters;
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
        filters = Transaction::FilterNone;
    }
    return filters;
}

#include "FiltersMenu.moc"
