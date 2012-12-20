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
#include <KDialog>

#include <Transaction>

using namespace PackageKit;

class PackageModel;
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
    explicit PkTransaction(QObject *parent = 0);
    ~PkTransaction();

    Q_INVOKABLE void installPackages(const QStringList &packages);
    Q_INVOKABLE void installFiles(const QStringList &files);
    Q_INVOKABLE void removePackages(const QStringList &packages);
    Q_INVOKABLE void updatePackages(const QStringList &packages, bool downloadOnly = false);

    QString title() const;
    Transaction::TransactionFlags flags() const;
    Q_INVOKABLE PkTransactionProgressModel* progressModel() const;
    Q_INVOKABLE void enableJobWatcher(bool enable);

    PkTransaction::ExitStatus exitStatus() const;
    bool isFinished() const;

    PackageModel* simulateModel() const;

public slots:
    void setTrusted(bool trusted);
    /**
     * When mediaChangeRequired(), eulaRequired() or repoSignatureRequired()
     * and the action is performed this method should be called
     */
    void requeueTransaction();

signals:
    void finished(PkTransaction::ExitStatus status);
    void titleChanged(const QString &title);
    void sorry(const QString &title, const QString &text, const QString &details);
    void errorMessage(const QString &title, const QString &text, const QString &details);
    void dialog(KDialog *widget);

private slots:
    void setupTransaction();
    void installPackages();
    void installFiles();
    void removePackages();
    void updatePackages();

    void installSignature();
    void acceptEula();

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

    void setExitStatus(PkTransaction::ExitStatus status = PkTransaction::Success);
    void reject();

private:
    void showDialog(KDialog *dialog);
    void showError(const QString &title, const QString &description, const QString &details = QString());
    void showSorry(const QString &title, const QString &description, const QString &details = QString());

    bool m_handlingActionRequired;
    bool m_showingError; //This might replace the above
    ExitStatus m_exitStatus;
    Transaction::Status m_status;
    PkTransactionPrivate *d;
};

#endif
