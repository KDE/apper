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
#include <KDialog>
#include <kdemacros.h>

#include <Transaction>

using namespace PackageKit;

namespace Ui {
    class PkTransaction;
}

class PkTransactionPrivate;
class KDE_EXPORT PkTransaction : public QWidget
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
    void hideCancelButton();

    void installPackages(const QList<Package> &packages);
    void installFiles(const QStringList &files);
    void removePackages(const QList<Package> &packages);
    void updatePackages(const QList<Package> &packages);
    void refreshCache(bool force = false);

    QString title() const;
    Transaction::Role role() const;

    PkTransaction::ExitStatus exitStatus() const;
    bool isFinished() const;

signals:
    void finished(PkTransaction::ExitStatus status);
    void allowCancel(bool enable);
    void titleChanged(const QString &title);
    void dialog(KDialog *widget);
    void sorry(const QString &title, const QString &text, const QString &details);
    void error(const QString &title, const QString &text, const QString &details);

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
    void eulaRequired(PackageKit::Eula info);
    void mediaChangeRequired(PackageKit::Transaction::MediaType type, const QString &id, const QString &text);
    void repoSignatureRequired(PackageKit::Signature info);

    void setExitStatus(PkTransaction::ExitStatus status = PkTransaction::Success);
    void reject();

private:
    void showDialog(KDialog *dialog);
    void showError(const QString &title, const QString &description, const QString &details = QString());
    void showSorry(const QString &title, const QString &description, const QString &details = QString());
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
