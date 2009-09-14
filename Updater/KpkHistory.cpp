/***************************************************************************
 *   Copyright (C) 2009 by Daniel Nicoletti                                *
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

#include "KpkHistory.h"

#include <KpkIcons.h>
#include <KpkStrings.h>
#include <QPackageKit>

#include <KMessageBox>

#include <KDebug>

KpkHistory::KpkHistory(QWidget *parent)
 : KDialog(parent)
{
    setupUi(mainWidget());

    m_transactionModel = new KpkSimpleTransactionModel(this);
    m_proxyModel = new KpkTransactionFilterModel(this);
    m_proxyModel->setSourceModel(m_transactionModel);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1);
    treeView->setModel(m_proxyModel);
    connect(searchLineKLE, SIGNAL(textChanged(const QString &)),
            m_proxyModel, SLOT(setFilterRegExp(const QString &)));

    setButtons(KDialog::User2 | KDialog::User1 | KDialog::Close);

    setButtonText(KDialog::User2, i18n("Rollback"));
    setButtonIcon(KDialog::User2, KpkIcons::getIcon("go-previous"));
    // Dummy backend does not support this so we can't test
    enableButton(KDialog::User2, false);

    setButtonText(KDialog::User1, i18n("Refresh transactions list"));
    setButtonIcon(KDialog::User1, KpkIcons::getIcon("view-refresh"));

    setModal(true);

    slotButtonClicked(KDialog::User1);

    incrementInitialSize(QSize(450,0));
    KConfig config("KPackageKit");
    KConfigGroup historyDialog(&config, "HistoryDialog");
    restoreDialogSize(historyDialog);
}

KpkHistory::~KpkHistory()
{
    KConfig config("KPackageKit");
    KConfigGroup historyDialog(&config, "HistoryDialog");
    saveDialogSize(historyDialog);
}

void KpkHistory::slotButtonClicked(int button)
{
    Transaction *t;
    switch (button) {
        case KDialog::User2 :
            // TODO implement rollback
            kDebug() << "Roolback";
            break;
        case KDialog::User1 :
            // Refresh transaction list
            kDebug() << "Refresh transaction list";
            //TODO make sure this deletes the transaction since they are pointers
            m_transactionModel->clear();
            t = Client::instance()->getOldTransactions(0);
            if (t->error()) {
                KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
            } else {
                connect(t, SIGNAL(transaction(PackageKit::Transaction *)),
                        m_transactionModel, SLOT(addTransaction(PackageKit::Transaction *)));
                connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                        this, SLOT(finished()));
            }
            break;
        default :
            KDialog::slotButtonClicked(button);
    }
    QString text;
    uint time = Client::instance()->getTimeSinceAction(Client::ActionRefreshCache) * 1000;
    text = i18n("Time since last cache refresh: %1", KGlobal::locale()->prettyFormatDuration(time));
    timeCacheLabel->setText(text);
}

void KpkHistory::finished()
{
    treeView->resizeColumnToContents(0);
    treeView->resizeColumnToContents(1);
    treeView->resizeColumnToContents(2);
    treeView->resizeColumnToContents(3);
    treeView->resizeColumnToContents(4);
}

#include "KpkHistory.moc"
