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

#include "PkTransactionWidget.h"
#include "ui_PkTransactionWidget.h"

#include <KLocalizedString>
#include <QPushButton>
#include <KPixmapSequence>
#include <KPixmapSequenceOverlayPainter>
#include <KIconLoader>

#include <QLoggingCategory>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusConnection>
#include <QScrollBar>

#include <Daemon>

#include "Enum.h"
#include "PkStrings.h"
#include "RepoSig.h"
#include "LicenseAgreement.h"
#include "PkIcons.h"
#include "ApplicationLauncher.h"
#include "Requirements.h"
#include "PkTransaction.h"
#include "TransactionDelegate.h"
#include "PkTransactionProgressModel.h"
#include "PackageModel.h"

Q_DECLARE_LOGGING_CATEGORY(APPER_LIB)

class PkTransactionWidgetPrivate
{
public:
    ApplicationLauncher *launcher;
    Transaction::Role role;
    KPixmapSequenceOverlayPainter *busySeq;
};

PkTransactionWidget::PkTransactionWidget(QWidget *parent) :
    QWidget(parent),
    m_trans(0),
    m_keepScrollBarAtBottom(true),
    m_handlingActionRequired(false),
    m_showingError(false),
    m_status(Transaction::StatusUnknown),
    ui(new Ui::PkTransactionWidget),
    d(new PkTransactionWidgetPrivate)
{
    ui->setupUi(this);

    // Setup the animation sequence
    d->busySeq = new KPixmapSequenceOverlayPainter(this);
    d->busySeq->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    d->busySeq->setWidget(ui->label);
    ui->label->clear();

    // Connect stuff from the progressView
    QScrollBar *scrollBar = ui->progressView->verticalScrollBar();
    connect(scrollBar, &QScrollBar::sliderMoved, this, &PkTransactionWidget::followBottom);
    connect(scrollBar, &QScrollBar::valueChanged, this, &PkTransactionWidget::followBottom);
    connect(scrollBar, &QScrollBar::rangeChanged, this, &PkTransactionWidget::rangeChanged);

    ui->progressView->setItemDelegate(new TransactionDelegate(this));
    
    connect(ui->cancelButton, &QDialogButtonBox::rejected, this, &PkTransactionWidget::cancel);
}

PkTransactionWidget::~PkTransactionWidget()
{
    // DO NOT disconnect the transaction here,
    // it might not exist when this happen
    delete d;
}

void PkTransactionWidget::hideCancelButton()
{
    // On the session installed we hide the
    // cancel button to use the KDialog main one
    ui->cancelButton->hide();
}

void PkTransactionWidget::cancel()
{
    if (m_trans) {
        m_trans->cancel();
    }
}

void PkTransactionWidget::setTransaction(PkTransaction *trans, Transaction::Role role)
{
    Q_ASSERT(trans);

    m_trans = trans;
    d->role = role;

    // This makes sure the Columns will properly resize to contents
        ui->progressView->header()->setStretchLastSection(false);
    if (role == Transaction::RoleRefreshCache) {
        trans->progressModel()->setColumnCount(1);
        ui->progressView->setModel(trans->progressModel());
        ui->progressView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    } else {
        trans->progressModel()->setColumnCount(3);
        ui->progressView->setModel(trans->progressModel());
        ui->progressView->header()->reset();
        ui->progressView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        ui->progressView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        ui->progressView->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    }

    connect(m_trans, &PkTransaction::percentageChanged, this, &PkTransactionWidget::updateUi);
    connect(m_trans, &PkTransaction::speedChanged, this, &PkTransactionWidget::updateUi);
    connect(m_trans, &PkTransaction::statusChanged, this, &PkTransactionWidget::updateUi);
    connect(m_trans, &PkTransaction::downloadSizeRemainingChanged, this, &PkTransactionWidget::updateUi);
    connect(m_trans, &PkTransaction::remainingTimeChanged, this, &PkTransactionWidget::updateUi);
    connect(m_trans, &PkTransaction::roleChanged, this, &PkTransactionWidget::updateUi);
    connect(m_trans, &PkTransaction::allowCancelChanged, this, &PkTransactionWidget::updateUi);
    connect(m_trans, &PkTransaction::transactionFlagsChanged, this, &PkTransactionWidget::updateUi);

    // Forward Q_SIGNALS:
    connect(m_trans, &PkTransaction::sorry, this, &PkTransactionWidget::sorry);
    connect(m_trans, &PkTransaction::errorMessage, this, &PkTransactionWidget::error);
    connect(m_trans, &PkTransaction::dialog, this, &PkTransactionWidget::dialog);

    // sets ui
    updateUi();
}

void PkTransactionWidget::unsetTransaction()
{
    if (m_trans == 0) {
        return;
    }

    connect(m_trans, &PkTransaction::percentageChanged, this, &PkTransactionWidget::updateUi);
    connect(m_trans, &PkTransaction::speedChanged, this, &PkTransactionWidget::updateUi);
    connect(m_trans, &PkTransaction::statusChanged, this, &PkTransactionWidget::updateUi);
    connect(m_trans, &PkTransaction::downloadSizeRemainingChanged, this, &PkTransactionWidget::updateUi);
    connect(m_trans, &PkTransaction::remainingTimeChanged, this, &PkTransactionWidget::updateUi);
    connect(m_trans, &PkTransaction::roleChanged, this, &PkTransactionWidget::updateUi);
    connect(m_trans, &PkTransaction::allowCancelChanged, this, &PkTransactionWidget::updateUi);
    connect(m_trans, &PkTransaction::transactionFlagsChanged, this, &PkTransactionWidget::updateUi);
}

void PkTransactionWidget::updateUi()
{
    // sets the action icon to be the window icon
    auto transaction = qobject_cast<PkTransaction*>(sender());
    if (transaction == 0 && (transaction = m_trans) == 0) {
        qCWarning(APPER_LIB) << "no transaction object";
        return;
    }

    uint percentage = transaction->percentage();
    QString percentageString;
    if (percentage <= 100) {
        if (ui->progressBar->value() != static_cast<int>(percentage)) {
            ui->progressBar->setMaximum(100);
            ui->progressBar->setValue(percentage);
            percentageString = QString::number(percentage);
        }
    } else if (ui->progressBar->maximum() != 0) {
        ui->progressBar->setMaximum(0);
        ui->progressBar->reset();
        percentageString = QLatin1String("");
    }

    ui->progressBar->setRemaining(transaction->remainingTime());

    // Status & Speed
    Transaction::Status status = transaction->status();
    uint speed = transaction->speed();
    qulonglong downloadSizeRemaining = transaction->downloadSizeRemaining();
    if (m_status != status) {
        m_status = status;
        ui->currentL->setText(PkStrings::status(status,
                                                speed,
                                                downloadSizeRemaining));

        KPixmapSequence sequence = KPixmapSequence(PkIcons::statusAnimation(status),
                                                   KIconLoader::SizeLarge);
        if (sequence.isValid()) {
            d->busySeq->setSequence(sequence);
            d->busySeq->start();
        }
    } else if (status == Transaction::StatusDownload) {
        ui->currentL->setText(PkStrings::status(status,
                                                speed,
                                                downloadSizeRemaining));
    }

    QString windowTitle;
    QString windowTitleProgress;
    QIcon windowIcon;
    Transaction::Role role = transaction->role();
    if (role == Transaction::RoleUnknown) {
        windowTitle  = PkStrings::status(Transaction::StatusSetup);
        if (percentageString.isEmpty()) {
            windowTitleProgress = PkStrings::status(status,
                                                    speed,
                                                    downloadSizeRemaining);
        } else {
            QString statusText = PkStrings::status(status,
                                                   speed,
                                                   downloadSizeRemaining);
            windowTitleProgress = i18n("%1 (%2%)", statusText, percentageString);
        }
        windowIcon = PkIcons::statusIcon(Transaction::StatusSetup);
    } else {
        windowTitle = PkStrings::action(role, transaction->transactionFlags());
        if (percentageString.isEmpty()) {
            windowTitleProgress = PkStrings::status(status,
                                                    speed,
                                                    downloadSizeRemaining);
        } else {
            QString statusText = PkStrings::status(status,
                                                   speed,
                                                   downloadSizeRemaining);
            windowTitleProgress = i18n("%1 (%2%)", statusText, percentageString);
        }
        windowIcon = PkIcons::actionIcon(role);
    }

    if (d->role != role) {
        d->role = role;
        setWindowIcon(PkIcons::actionIcon(role));
        setWindowTitle(windowTitle);

        emit titleChanged(windowTitle);
        emit titleChangedProgress(windowTitleProgress);
    } else if (!percentageString.isNull()) {
        emit titleChangedProgress(windowTitleProgress);
    }

    // check to see if we can cancel
    bool cancel = transaction->allowCancel();
    emit allowCancel(cancel);
    ui->cancelButton->setEnabled(cancel);
}

bool PkTransactionWidget::isFinished() const
{
//    return d->finished;
    return false;
}

bool PkTransactionWidget::isCancelVisible() const
{
    return ui->cancelButton->isVisible();
}

void PkTransactionWidget::reject()
{
//    d->finished = true;
    //    setExitStatus(Cancelled);
}

void PkTransactionWidget::followBottom(int value)
{
    // If the user moves the slider to the bottom
    // keep it there as the list expands
    QScrollBar *scrollBar = qobject_cast<QScrollBar*>(sender());
    m_keepScrollBarAtBottom = value == scrollBar->maximum();
}

void PkTransactionWidget::rangeChanged(int min, int max)
{
    Q_UNUSED(min)
    QScrollBar *scrollBar = qobject_cast<QScrollBar*>(sender());
    if (m_keepScrollBarAtBottom && scrollBar->value() != max) {
        scrollBar->setValue(max);
    }
}

Transaction::Role PkTransactionWidget::role() const
{
    return d->role;
}

PkTransaction *PkTransactionWidget::transaction() const
{
    return m_trans;
}

#include "moc_PkTransactionWidget.cpp"
