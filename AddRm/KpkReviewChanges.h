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

#ifndef KPKREVIEWCHANGES_H
#define KPKREVIEWCHANGES_H

#include <KDialog>
#include <KProgressDialog>

#include <KpkDelegate.h>
#include <KpkTransaction.h>
#include "KpkPackageModel.h"
#include "ui_KpkReviewChanges.h"
#include <QPackageKit>

using namespace PackageKit;

class KpkReviewChanges : public KDialog, Ui::KpkReviewChanges
{
    Q_OBJECT
public:
    KpkReviewChanges( const QList<Package*> &packages, QWidget *parent=0);
    ~KpkReviewChanges();

public slots:
    void remFinished(KpkTransaction::ExitStatus);
    void addFinished(KpkTransaction::ExitStatus);

    void reqFinished(PackageKit::Transaction::ExitStatus status, uint runtime);
    void depFinished(PackageKit::Transaction::ExitStatus status, uint runtime);
    void installPackages();
    void removePackages();

private:
    KpkPackageModel *m_pkgModelMain, *m_pkgModelReq, *m_pkgModelDep;
    KpkDelegate *m_pkgDelegate;
    
    KProgressDialog *m_waitPD;
    
    Client *m_client;
    Transaction *m_transactionReq;
    Transaction *m_transactionDep;
    QTimer m_notifyT;
    int m_viewWidth;

    void updateColumnsWidth(bool force = false);
    void doAction();
    void checkTask();

    QList<Package*> m_remPackages;
    QList<Package*> m_addPackages;
    QList<Package*> m_reqDepPackages;

    Client::Actions m_actions;

private slots:
    void errorCode(PackageKit::Client::ErrorType error, const QString &details);

protected slots:
    virtual void slotButtonClicked(int button);

protected:
    virtual void resizeEvent( QResizeEvent * event );
    virtual bool event( QEvent * event );
};

#endif
