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

#ifndef PK_TRANSACTION_H
#define PK_TRANSACTION_H

#include <QWidget>
#include <kdemacros.h>

#include <Transaction>

using namespace PackageKit;

namespace Ui {
    class PkTransaction;
}

class SimulateModel;
class PkTransactionPrivate;
class KDE_EXPORT PkTransaction : public QWidget
{
    Q_OBJECT
    Q_ENUMS(ExitStatus)
public:
    explicit PkTransaction(Transaction *trans, QWidget *parent = 0);
    ~PkTransaction();

    void hideCancelButton();

    void installPackages(const QList<Package> &packages);
    void installFiles(const QStringList &files);
    void removePackages(const QList<Package> &packages);
    void updatePackages(const QList<Package> &packages);
    void refreshCache();

    void setTransaction(Transaction *trans);
    // Do not create a method to retrieve the internal pointer
    // of Transaction, instead compare the tid with the tids from
    // Client::getTransaction(), to avoid deleted pointers.
    QString tid() const;
    bool allowDeps() const;
    bool onlyTrusted() const;
    QList<Package> packages() const;
    QStringList files() const;
    SimulateModel* simulateModel() const;

    QString title() const;
    Transaction::Role role() const;
    Transaction::Error error() const;
    QString errorDetails() const;

    void setAllowDeps(bool allowDeps);
    void setPackages(const QList<Package> &packages);
    void setFiles(const QStringList &files);
    void setupDebconfDialog(const QString &tid);

    typedef enum {
        Success,
        Failed,
        Cancelled
    } ExitStatus;

    PkTransaction::ExitStatus exitStatus() const;
    bool isFinished() const;

signals:
    void finished(PkTransaction::ExitStatus status);
    void allowCancel(bool enable);
    void titleChanged(const QString &title);

public slots:
    void cancel();

private slots:
    void setupTransaction(PackageKit::Transaction *transaction);
    void installPackages();
    void installFiles();
    void removePackages(bool allow_deps = true);
    void updatePackages();

    void transactionFinished(PackageKit::Transaction::Exit status);
    void errorCode(PackageKit::Transaction::Error error, const QString &details);
    void updateUi();
    void eulaRequired(PackageKit::Eula info);
    void mediaChangeRequired(PackageKit::Transaction::MediaType type, const QString &id, const QString &text);
    void repoSignatureRequired(PackageKit::Signature info);
    void files(const PackageKit::Package &package, const QStringList &files);

    void setExitStatus(PkTransaction::ExitStatus status);
    void reject();

private:
    void unsetTransaction();
    void requeueTransaction();

    Transaction *m_trans;
    bool m_handlingActionRequired;
    bool m_showingError; //This might replace the above
    ExitStatus m_exitStatus;
    Transaction::Status m_status;
    Ui::PkTransaction *ui;
    PkTransactionPrivate *d;
};

#endif
