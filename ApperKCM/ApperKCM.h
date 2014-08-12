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

#ifndef APPER_KCM_U
#define APPER_KCM_U

#include <PkTransaction.h>

#include <QtGui/QStandardItemModel>

#include <KCModule>
#include <KCModuleProxy>
#include <KIcon>
#include <KToolBarPopupAction>
#include <KCategorizedSortFilterProxyModel>

#include <Transaction>

using namespace PackageKit;

namespace Ui {
    class ApperKCM;
}

class PackageModel;
class FiltersMenu;
class TransactionHistory;
class CategoryModel;
class Settings;
class Updater;
class ApperKCM : public KCModule
{
    Q_OBJECT
    Q_PROPERTY(QString page READ page WRITE setPage USER true)
public:
    ApperKCM(QWidget *parent, const QVariantList &args);
    ~ApperKCM();
    
    QString page() const;

signals:
    void changed(bool state);
    void caption(const QString &title = QString());

public slots:
    void daemonChanged();
    void load();
    void save();
    void defaults();
    void setPage(const QString &page);

private slots:
    void search();
    void setupHomeModel();
    void genericActionKTriggered();

    void on_backTB_clicked();
    void on_changesPB_clicked();

    void on_actionFindName_triggered();
    void on_actionFindDescription_triggered();
    void on_actionFindFile_triggered();

    void on_homeView_activated(const QModelIndex &index);

    void finished();
    void errorCode(PackageKit::Transaction::Error error, const QString &detail);

    void checkChanged();
    void changed();

    void refreshCache();

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    void disconnectTransaction();
    bool canChangePage();
    void setCurrentActionEnabled(bool state);
    void setCurrentAction(QAction *action);
    void setCurrentActionCancel(bool cancel);

    void setActionCancel(bool enabled);
    void keyPressEvent(QKeyEvent *event);

    Ui::ApperKCM *ui;
    KToolBarPopupAction *m_genericActionK;
    QAction             *m_currentAction;
    CategoryModel       *m_groupsModel;
    KCategorizedSortFilterProxyModel *m_groupsProxyModel;
    PackageModel     *m_browseModel;
    PackageModel     *m_changesModel;
    Settings            *m_settingsPage;
    Updater             *m_updaterPage;

    Transaction *m_searchTransaction;

    KIcon m_findIcon;
    KIcon m_cancelIcon;

    FiltersMenu *m_filtersMenu;
    Transaction::Roles m_roles;
    bool m_forceRefreshCache;

    TransactionHistory *m_history;

    // Old search cache
    Transaction::Role    m_searchRole;
    QString       m_searchString;
    QString       m_searchGroupCategory;
    PackageKit::Transaction::Group   m_searchGroup;
    QModelIndex   m_searchParentCategory;
    QStringList   m_searchCategory;
};

#endif
