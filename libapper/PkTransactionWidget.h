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

#include <Transaction>

using namespace PackageKit;

namespace Ui {
    class PkTransactionWidget;
}

class PkTransaction;
class PkTransactionWidgetPrivate;
class KDE_EXPORT PkTransactionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PkTransactionWidget(QWidget *parent = 0);
    ~PkTransactionWidget();

    void setTransaction(PkTransaction *trans, Transaction::Role role);
    void hideCancelButton();

    Transaction::Role role() const;
    Transaction* transaction() const;

    bool isFinished() const;
    bool isCancelVisible() const;

signals:
    void allowCancel(bool enable);
    void titleChanged(const QString &title);
    void titleChangedProgress(const QString &title);
    void dialog(KDialog *widget);
    void sorry(const QString &title, const QString &text, const QString &details);
    void error(const QString &title, const QString &text, const QString &details);

public slots:
    void cancel();

private slots:
    void updateUi();

    void reject();
    void followBottom(int value);
    void rangeChanged(int min, int max);

private:
    void unsetTransaction();

    PkTransaction *m_trans;
    bool m_keepScrollBarAtBottom;
    bool m_handlingActionRequired;
    bool m_showingError; //This might replace the above
    Transaction::Status m_status;
    Ui::PkTransactionWidget *ui;
    PkTransactionWidgetPrivate *d;
};

#endif
