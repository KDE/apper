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

#ifndef KPK_TRANSACTION_H
#define KPK_TRANSACTION_H

#include <KDialog>

#include <QPackageKit>

using namespace PackageKit;

class KpkSimulateModel;
class KpkTransactionPrivate;
class KDE_EXPORT KpkTransaction : public KDialog
{
    Q_OBJECT
    Q_ENUMS(ExitStatus)
public:
    enum BehaviorFlag {
        Modal = 1,
        CloseOnFinish = 2
    };
    Q_DECLARE_FLAGS(Behaviors, BehaviorFlag)

    explicit KpkTransaction(Transaction *trans, Behaviors flags = 0, QWidget *parent = 0);
    ~KpkTransaction();

    void setTransaction(Transaction *trans);
    // Do not create a method to retrieve the internal pointer
    // of Transaction, instead compare the tid with the tids from
    // Client::getTransaction(), to avoid deleted pointers.
    QString tid() const;
    bool allowDeps() const;
    bool onlyTrusted() const;
    QList<QSharedPointer<PackageKit::Package> > packages() const;
    QStringList files() const;
    KpkSimulateModel* simulateModel() const;

    Enum::Role role() const;
    Enum::Error error() const;
    QString errorDetails() const;

    void setAllowDeps(bool allowDeps);
    void setPackages(const QList<QSharedPointer<PackageKit::Package> > &packages);
    void setFiles(const QStringList &files);

    typedef enum {
        Success,
        Failed,
        Cancelled
    } ExitStatus;

    KpkTransaction::ExitStatus exitStatus() const;

signals:
    void finished(KpkTransaction::ExitStatus status);

private slots:
    void finishedDialog();
    void transactionFinished(PackageKit::Enum::Exit status);
    void errorCode(PackageKit::Enum::Error error, const QString &details);
    void updateUi();
    void eulaRequired(PackageKit::Client::EulaInfo info);
    void mediaChangeRequired(PackageKit::Enum::MediaType type, const QString &id, const QString &text);
    void repoSignatureRequired(PackageKit::Client::SignatureInfo info);

    void setExitStatus(KpkTransaction::ExitStatus status);

private:
    void unsetTransaction();
    void requeueTransaction();

    Transaction *m_trans;
    bool m_handlingActionRequired;
    bool m_showingError; //This might replace the above
    Behaviors m_flags;
    ExitStatus m_exitStatus;
    Enum::Status m_status;
    KpkTransactionPrivate *d;

protected slots:
    virtual void slotButtonClicked(int button);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KpkTransaction::Behaviors)

#endif
