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

#ifndef KPK_REVIEWCHANGES_H
#define KPK_REVIEWCHANGES_H

#include <KDialog>
#include <KProgressDialog>

#include <KpkDelegate.h>
#include <KpkTransaction.h>
#include "KpkPackageModel.h"
#include "KpkSimplePackageModel.h"

#include <QPackageKit>

using namespace PackageKit;

class KpkReviewChangesPrivate;

class KDE_EXPORT KpkReviewChanges : public KDialog
{
    Q_OBJECT
public:
    explicit KpkReviewChanges(const QList<Package*> &packages, QWidget *parent = 0);
    ~KpkReviewChanges();

    void setTitle(const QString &title);
    void setText(const QString &text);

public slots:
    void remFinished(KpkTransaction::ExitStatus);
    void addFinished(KpkTransaction::ExitStatus);

    void reqFinished(PackageKit::Transaction::ExitStatus status, uint runtime);
    void depFinished(PackageKit::Transaction::ExitStatus status, uint runtime);
    void installPackages();
    void removePackages();

private:
    KpkReviewChangesPrivate* d;
    KpkPackageModel *m_pkgModelMain;
    KpkSimplePackageModel *m_pkgModelReq, *m_pkgModelDep;
    KpkDelegate *m_pkgDelegate;

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
    void checkChanged();

protected slots:
    virtual void slotButtonClicked(int button);

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual bool event(QEvent *event);
};

#endif
