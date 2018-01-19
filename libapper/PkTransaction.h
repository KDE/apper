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
//#include <kdemacros.h>
#include <QDialog>

#include <Transaction>

using namespace PackageKit;

class PackageModel;
class PkTransactionPrivate;
class PkTransactionProgressModel;
class Q_DECL_EXPORT PkTransaction : public QObject
{
    Q_OBJECT
    Q_ENUMS(ExitStatus)
public:
    typedef enum {
        Success,
        Failed,
        Cancelled
    } ExitStatus;
    explicit PkTransaction(QObject *parent = 0);
    ~PkTransaction();

    void setupTransaction(Transaction *transaction);

    Q_INVOKABLE void installPackages(const QStringList &packages);
    Q_INVOKABLE void installFiles(const QStringList &files);
    Q_INVOKABLE void removePackages(const QStringList &packages);
    Q_INVOKABLE void updatePackages(const QStringList &packages, bool downloadOnly = false);
    Q_INVOKABLE void refreshCache(bool force);


    QString title() const;
    Transaction::Role cachedRole() const;
    Transaction::TransactionFlags flags() const;
    Q_INVOKABLE PkTransactionProgressModel* progressModel() const;
    Q_INVOKABLE void enableJobWatcher(bool enable);

    PkTransaction::ExitStatus exitStatus() const;
    bool isFinished() const;

    PackageModel* simulateModel() const;

    Q_PROPERTY(uint percentage READ percentage NOTIFY percentageChanged)
    uint percentage() const;

    Q_PROPERTY(uint remainingTime READ remainingTime NOTIFY remainingTimeChanged)
    uint remainingTime() const;

    Q_PROPERTY(uint speed READ speed NOTIFY speedChanged)
    uint speed() const;

    Q_PROPERTY(qulonglong downloadSizeRemaining READ downloadSizeRemaining NOTIFY downloadSizeRemainingChanged)
    qulonglong downloadSizeRemaining() const;

    Q_PROPERTY(PackageKit::Transaction::Status status READ status NOTIFY statusChanged)
    Transaction::Status status() const;

    Q_PROPERTY(PackageKit::Transaction::Role role READ role NOTIFY roleChanged)
    Transaction::Role role() const;

    Q_PROPERTY(bool allowCancel READ allowCancel NOTIFY allowCancelChanged)
    bool allowCancel() const;

    Q_PROPERTY(PackageKit::Transaction::TransactionFlags transactionFlags READ transactionFlags NOTIFY transactionFlagsChanged)
    Transaction::TransactionFlags transactionFlags() const;

public Q_SLOTS:
    void getUpdateDetail(const QString &packageID);
    void getUpdates();
    void cancel();
    void setTrusted(bool trusted);
    /**
     * When mediaChangeRequired(), eulaRequired() or repoSignatureRequired()
     * and the action is performed this method should be called
     */
    void requeueTransaction();

Q_SIGNALS:
    void package(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary);
    void updateDetail(const QString &packageID,
                      const QStringList &updates,
                      const QStringList &obsoletes,
                      const QStringList &vendorUrls,
                      const QStringList &bugzillaUrls,
                      const QStringList &cveUrls,
                      PackageKit::Transaction::Restart restart,
                      const QString &updateText,
                      const QString &changelog,
                      PackageKit::Transaction::UpdateState state,
                      const QDateTime &issued,
                      const QDateTime &updated);
    void errorCode(PackageKit::Transaction::Error error, const QString &details);
    void finished(PkTransaction::ExitStatus status);
    void titleChanged(const QString &title);
    void sorry(const QString &title, const QString &text, const QString &details);
    void errorMessage(const QString &title, const QString &text, const QString &details);
    void dialog(QDialog *widget);

    void allowCancelChanged();
    void isCallerActiveChanged();
    void downloadSizeRemainingChanged();
    void elapsedTimeChanged();
    void lastPackageChanged();
    void percentageChanged();
    void remainingTimeChanged();
    void roleChanged();
    void speedChanged();
    void statusChanged();
    void transactionFlagsChanged();
    void uidChanged();

private Q_SLOTS:
    void installPackages();
    void installFiles();
    void removePackages();
    void updatePackages();

    void slotChanged();
    void slotFinished(PackageKit::Transaction::Exit status);
    void slotErrorCode(PackageKit::Transaction::Error error, const QString &details);
    void slotEulaRequired(const QString &eulaID, const QString &packageID, const QString &vendor, const QString &licenseAgreement);
    void slotMediaChangeRequired(PackageKit::Transaction::MediaType type, const QString &id, const QString &text);
    void slotRepoSignature(const QString &packageID,
                           const QString &repoName,
                           const QString &keyUrl,
                           const QString &keyUserid,
                           const QString &keyId,
                           const QString &keyFingerprint,
                           const QString &keyTimestamp,
                           PackageKit::Transaction::SigType type);

    void setExitStatus(int status = PkTransaction::Success);
    void reject();

private:
    void showDialog(QDialog *dialog);
    void showError(const QString &title, const QString &description, const QString &details = QString());
    void showSorry(const QString &title, const QString &description, const QString &details = QString());

    PkTransactionPrivate *d;
};

#endif
