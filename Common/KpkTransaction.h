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

#ifndef KPKTRANSACTION_H
#define KPKTRANSACTION_H

#include <KDialog>

#include "ui_KpkTransaction.h"
#include <QPackageKit>

using namespace PackageKit;

class KpkTransaction : public KDialog, Ui::KpkTransaction
{
    Q_OBJECT
    Q_ENUMS(ExitStatus)
public:
    KpkTransaction( Transaction  *trans, bool modal = true, QWidget *parent=0);
    ~KpkTransaction();

    void setTransaction(Transaction *trans);

    typedef enum {
	Success,
	Failed,
	Cancelled,
	ReQueue
    } ExitStatus;

signals:
    void kTransactionFinished(KpkTransaction::ExitStatus status);

private:
    Transaction *m_trans;
    bool m_handlyingGpgOrEula;

private slots:
    void finished(PackageKit::Transaction::ExitStatus status, uint runtime);
    void errorCode(PackageKit::Client::ErrorType error, const QString &details);
    void statusChanged(PackageKit::Transaction::Status status);
    void currPackage(PackageKit::Package *);
    void progressChanged(PackageKit::Transaction::ProgressInfo info);
    void eulaRequired(PackageKit::Client::EulaInfo info);
    void repoSignatureRequired(PackageKit::Client::SignatureInfo info);

protected slots:
    virtual void slotButtonClicked(int button);
};

#endif
