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

#ifndef PK_TRANSACTION_WIDGET_H
#define PK_TRANSACTION_WIDGET_H

#include <QWidget>
#include <KDialog>
#include <kdemacros.h>

#include "PkTransaction.h"

using namespace PackageKit;

namespace Ui {
    class PkTransactionWidget;
}

class PkTransactionWidgetPrivate;
class KDE_EXPORT PkTransactionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PkTransactionWidget(QWidget *parent = 0);
    ~PkTransactionWidget();

    void setTransaction(PkTransaction *trans, Transaction::Role role);
    void hideCancelButton();

    QString title() const;
    Transaction::Role role() const;

    bool isFinished() const;
    bool isCancelVisible() const;

signals:
    void allowCancel(bool enable);
    void titleChanged(const QString &title);
    void dialog(KDialog *widget);
    void sorry(const QString &title, const QString &text, const QString &details);
    void error(const QString &title, const QString &text, const QString &details);

public slots:
    void cancel();

private slots:
    void installSignature();
    void acceptEula();

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

    void reject();
    void followBottom(int value);
    void rangeChanged(int min, int max);

private:
    void showDialog(KDialog *dialog);
    void showError(const QString &title, const QString &description, const QString &details = QString());
    void showSorry(const QString &title, const QString &description, const QString &details = QString());
    void unsetTransaction();

    Transaction *m_trans;
    bool m_keepScrollBarAtBottom;
    bool m_handlingActionRequired;
    bool m_showingError; //This might replace the above
    Transaction::Status m_status;
    Ui::PkTransactionWidget *ui;
    PkTransactionWidgetPrivate *d;
};

#endif
