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

#include <QObject>
#include <kdemacros.h>

#include <Transaction>

using namespace PackageKit;

class PkTransactionPrivate;
class PkTransactionProgressModel;
class KDE_EXPORT PkTransaction : public Transaction
{
    Q_OBJECT
    Q_ENUMS(ExitStatus)
public:
    typedef enum {
        Success,
        Failed,
        Cancelled
    } ExitStatus;
    explicit PkTransaction(QWidget *parent = 0);
    ~PkTransaction();

    void setTransaction(Transaction *trans, Transaction::Role role);

    void installPackages(const QStringList &packages);
    void installFiles(const QStringList &files);
    void removePackages(const QStringList &packages);
    void updatePackages(const QStringList &packages);
//    void refreshCache(bool force = false);

    QString title() const;
    Transaction::Role role() const;
    Transaction::TransactionFlags flags() const;
    PkTransactionProgressModel* progressModel() const;

    PkTransaction::ExitStatus exitStatus() const;
    bool isFinished() const;

signals:
    void finished(PkTransaction::ExitStatus status);
    void allowCancel(bool enable);
    void titleChanged(const QString &title);
    void sorry(const QString &title, const QString &text, const QString &details);
    void errorMessage(const QString &title, const QString &text, const QString &details);

public slots:
    void cancel();

private slots:
    void setupTransaction(PackageKit::Transaction *transaction);
    void installPackages();
    void installFiles();
    void removePackages();
    void updatePackages();

    void installSignature();
    void acceptEula();

    void transactionFinished(PackageKit::Transaction::Exit status);
    void errorCode(PackageKit::Transaction::Error error, const QString &details);
    void updateUi();
    void eulaRequired(const QString &eulaID, const QString &packageID, const QString &vendor, const QString &licenseAgreement);
    void mediaChangeRequired(PackageKit::Transaction::MediaType type, const QString &id, const QString &text);
    void repoSignatureRequired(const QString &packageID,
                               const QString &repoName,
                               const QString &keyUrl,
                               const QString &keyUserid,
                               const QString &keyId,
                               const QString &keyFingerprint,
                               const QString &keyTimestamp,
                               PackageKit::Transaction::SigType type);

    void setExitStatus(PkTransaction::ExitStatus status = PkTransaction::Success);
    void reject();

private:
    void showError(const QString &title, const QString &description, const QString &details = QString());
    void showSorry(const QString &title, const QString &description, const QString &details = QString());
    void unsetTransaction();
    void requeueTransaction();

    Transaction *m_trans;
    bool m_handlingActionRequired;
    bool m_showingError; //This might replace the above
    ExitStatus m_exitStatus;
    Transaction::Status m_status;
    PkTransactionPrivate *d;
};

#endif
