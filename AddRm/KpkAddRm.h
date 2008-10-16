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

#include "KpkAddRmModel.h"
#include "../Common/KpkDelegate.h"

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

public slots:
    void load();
    void save();

private:
    bool m_mTransRuning;//main trans
    KpkAddRmModel *m_pkg_model_main;
    KpkAddRmModel *m_pkg_model_dep;
    KpkAddRmModel *m_pkg_model_req;
    KpkDelegate *pkg_delegate;

    Client *m_client;
    Transaction *m_pkClient_main;

    QTimer m_notifyT;
    QMenu *m_filtersQM;
    KIcon m_findIcon;
    KIcon m_cancelIcon;

    // We need to keep a list to build the filters string
    QList<QAction*> actions;
    QHash<QAction *,Client::Filter> m_filtersAction;
    void filterMenu(Client::Filters filters);
    Client::Filters filters();

    void updateColumnsWidth(bool force = false);
    void search();
    void connectTransaction(Transaction *transaction);

    Client::Actions m_actions;

    // Old search cache
    Client::Action  m_searchAction;
    QString         m_searchString;
    Client::Group   m_searchGroup;
    Client::Filters m_searchFilters;

private slots:
    void getDetails(PackageKit::Package *p);
    void getFiles(PackageKit::Package *p);
    void getDepends(PackageKit::Package *p);
    void getRequires(PackageKit::Package *p);
    void getInfoFinished(PackageKit::Transaction::ExitStatus status, uint runtime);

    void on_findPB_clicked();
    void on_groupsCB_currentIndexChanged(int index);
    void on_packageView_pressed(const QModelIndex &index);

    void description(PackageKit::Package *package);
    void files(PackageKit::Package *package, const QStringList &files);
    void finished(PackageKit::Transaction::ExitStatus status, uint runtime);
    void message(PackageKit::Client::MessageType message, const QString &details);
    void errorCode(PackageKit::Client::ErrorType error, const QString &detail);
    void statusChanged(PackageKit::Transaction::Status status);

    void notifyUpdate();
    void progressChanged(PackageKit::Transaction::ProgressInfo info);

signals:
    void changed(bool state);

protected:
    virtual void resizeEvent ( QResizeEvent * event );
    virtual bool event ( QEvent * event );

};

#endif
