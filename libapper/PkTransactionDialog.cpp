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

#include <config.h>

#include "PkTransactionDialog.h"

#include <KLocale>
#include <KMessageBox>
#include <KPushButton>
#include <KService>
#include <KPixmapSequence>
#include <KPixmapSequenceOverlayPainter>

#include <KDebug>

#include <QPropertyAnimation>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QtGui/QTreeView>

#include <Transaction>

#include "ProgressView.h"
#include "PkTransaction.h"

class PkTransactionDialogPrivate
{
public:
    QString tid;
    bool showDetails;
    bool finished;
    bool allowDeps;
    bool onlyTrusted;
    Transaction::Role role;
    Transaction::Error error;
    QString errorDetails;
    QList<Package> packages;
    QStringList files;
    ProgressView *progressView;
    KPixmapSequenceOverlayPainter *busySeq;
};

PkTransactionDialog::PkTransactionDialog(Transaction *trans, Behaviors flags, QWidget *parent)
 : KDialog(parent),
   m_flags(flags),
   d(new PkTransactionDialogPrivate)
{
    m_ui = new PkTransaction(parent);
    m_ui->setTransaction(trans, trans->role());
    m_ui->hideCancelButton();

    connect(m_ui, SIGNAL(allowCancel(bool)), button(KDialog::Cancel), SLOT(setEnabled(bool)));
    connect(m_ui, SIGNAL(finished(PkTransaction::ExitStatus)),
            this, SIGNAL(finished(PkTransaction::ExitStatus)));
    connect(m_ui, SIGNAL(finished(PkTransaction::ExitStatus)),
            this, SLOT(finishedDialog(PkTransaction::ExitStatus)));

    setMainWidget(m_ui);

//     d->ui.setupUi(mainWidget());


    setButtons(KDialog::Details | KDialog::User1 | KDialog::Cancel);
    enableButton(KDialog::Details, false);
    button(KDialog::Details)->setCheckable(true);

    // Setup HIDE custom button
    setButtonText(KDialog::User1, i18n("Hide"));
    setButtonToolTip(KDialog::User1,
                     i18n("Allows you to hide the window whilst keeping the transaction task running."));
    setEscapeButton(KDialog::User1);

    KConfig config("apper");
    KConfigGroup transactionGroup(&config, "Transaction");

    d->progressView = new ProgressView;

    if (m_flags & Modal) {
        setWindowModality(Qt::WindowModal);
    }

    // after ALL set, lets set the transaction
    setTransaction(trans);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    setMaximumSize(QWIDGETSIZE_MAX, size().height());

    KConfigGroup transactionDialog(&config, "PkTransactionDialog");
    restoreDialogSize(transactionDialog);
}

PkTransactionDialog::~PkTransactionDialog()
{
    KConfig config("apper");
    if (isButtonEnabled(KDialog::Details)) {
        KConfigGroup transactionGroup(&config, "Transaction");
        transactionGroup.writeEntry("ShowDetails", d->showDetails);
    }
    KConfigGroup transactionDialog(&config, "PkTransactionDialog");
    saveDialogSize(transactionDialog);

    // DO NOT disconnect the transaction here,
    // it might not exist when this happen
    delete d->progressView;
    delete d;
}

void PkTransactionDialog::slotButtonClicked(int bt)
{
    switch(bt) {
    case KDialog::Cancel :
//         kDebug() << "KDialog::Cancel";
        m_ui->cancel();
/*        m_flags |= CloseOnFinish;*/
        break;
    case KDialog::User1 :
//         kDebug() << "KDialog::User1";
        // when we're done finishedDialog() is called
        done(KDialog::User1);
        break;
    case KDialog::Close :
//         kDebug() << "KDialog::Close";
        // Always disconnect BEFORE emitting finished
//         unsetTransaction();
//         setExitStatus(Cancelled);
        done(KDialog::Close);
        break;
    case KDialog::Details :
    {
        d->showDetails = !d->progressView->isVisible();
        button(KDialog::Details)->setChecked(d->showDetails);
        if (d->progressView->isVisible()) {
            QSize windowSize = size();
            windowSize.rheight() -= d->progressView->height();
            d->progressView->setVisible(false);
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
            setMaximumSize(QWIDGETSIZE_MAX, windowSize.height());
//             d->ui.gridLayout->removeWidget(d->progressView);
        } else {
            QSize windowSize = size();
            windowSize.rheight() += d->progressView->height();
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
//             d->ui.gridLayout->addWidget(d->progressView, 1, 0, 1, 2);
            d->progressView->setVisible(true);
            resize(windowSize);
        }
    }
        break;
    default : // Should be only details
        KDialog::slotButtonClicked(bt);
    }
}

void PkTransactionDialog::setTransaction(Transaction *trans)
{
    m_ui->setTransaction(trans, trans->role());
}

PkTransaction* PkTransactionDialog::transaction() const
{
    return m_ui;
}

void PkTransactionDialog::finishedDialog(PkTransaction::ExitStatus status)
{
    if (status == PkTransaction::Cancelled) {
        done(QDialog::Rejected);
    } else {
        done(QDialog::Accepted);
    }
}

void PkTransactionDialog::setFiles(const QStringList &files)
{
    d->files = files;
}

#include "PkTransactionDialog.moc"
