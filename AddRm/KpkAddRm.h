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

#ifndef PKADDRM_H
#define PKADDRM_H

#include <QtGui/QtGui>
#include <QtCore/QtCore>

#include <KIcon>
#include <KToolBarPopupAction>

#include "KpkPackageModel.h"
#include "KpkPackageDetails.h"
#include <KpkDelegate.h>

#include <QPackageKit>
#include "ui_KpkAddRm.h"

using namespace PackageKit;

class KpkAddRm : public QWidget, public Ui::KpkAddRm
{
    Q_OBJECT
public:
    KpkAddRm( QWidget *parent=0 );
    ~KpkAddRm();

signals:
    void getInfo(PackageKit::Package *package);
    void changed(bool state);

public slots:
    void load();
    void save();

private slots:
void actionFindNameK();

    void on_actionFindName_triggered();
    void on_actionFindDescription_triggered();
    void on_actionFindFile_triggered();

    void on_groupsCB_currentIndexChanged(int index);
    void on_packageView_pressed(const QModelIndex &index);

    void finished(PackageKit::Transaction::ExitStatus status, uint runtime);
    void message(PackageKit::Client::MessageType message, const QString &details);
    void errorCode(PackageKit::Client::ErrorType error, const QString &detail);

    void checkChanged();

    void packageViewSetRootIsDecorated(bool value);

private:
    QMenu *m_findMenu;

    QAction *m_actionBasename;
    QAction *m_actionNewestOnly;
    QAction *m_actionViewInGroups;

    QToolBar *toolBar;
    KToolBarPopupAction *m_genericActionK;
    QAction *m_currentAction;
    void setCurrentAction(QAction *action);
    bool m_mTransRuning;//main trans
    KpkPackageModel *m_pkg_model_main;
    KpkDelegate *pkg_delegate;
//     QHash<QModelIndex, Client::Filter> m_filtersAction;

    Client *m_client;
    Transaction *m_pkClient_main;

    QMenu *m_filtersQM;
    KIcon m_findIcon;
    KIcon m_cancelIcon;
    KIcon m_filterIcon;

    // We need to keep a list to build the filters string
    QList<QAction*> actions;
    QHash<QAction *, Client::Filter> m_filtersAction;
    void filterMenu(Client::Filters filters);
    Client::Filters filters();

    void updateColumnsWidth(bool force = false);
    void setActionsDefaults();
    void setActionCancel(bool enabled);
    void search();
    void connectTransaction(Transaction *transaction);

    Client::Actions m_actions;

    // Old search cache
    Client::Action  m_searchAction;
    QString         m_searchString;
    Client::Group   m_searchGroup;
    Client::Filters m_searchFilters;

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual bool event(QEvent *event);

};

#endif
