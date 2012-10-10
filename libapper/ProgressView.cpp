/***************************************************************************
 *   Copyright (C) 2010-2011 by Daniel Nicoletti                           *
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

#include "ProgressView.h"

#include <QVBoxLayout>
#include <QScrollBar>
#include <QHeaderView>

#include <KLocale>
#include <KConfigGroup>
#include <KDebug>

#include <PkStrings.h>

#include "TransactionDelegate.h"

using namespace PackageKit;

ProgressView::ProgressView(QWidget *parent)
 : QTreeView(parent),
   m_keepScrollBarBottom(true)
{
    m_model = new QStandardItemModel(this);
    m_delegate = new TransactionDelegate(this);
    m_defaultDelegate = new QStyledItemDelegate(this);

    setModel(m_model);
    setRootIsDecorated(false);
    setHeaderHidden(true);
    setSelectionMode(QAbstractItemView::NoSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
//     verticalScrollBar()->value();

    m_scrollBar = verticalScrollBar();
    connect(m_scrollBar, SIGNAL(sliderMoved(int)),
            this, SLOT(followBottom(int)));
    connect(m_scrollBar, SIGNAL(valueChanged(int)),
            this, SLOT(followBottom(int)));
    connect(m_scrollBar, SIGNAL(rangeChanged(int,int)),
            this, SLOT(rangeChanged(int,int)));

    KConfig config("apper");
    KConfigGroup transactionDialog(&config, "TransactionDialog");

    resize(width(), transactionDialog.readEntry("detailsHeight", QWidget::height()));
}

ProgressView::~ProgressView()
{
    KConfig config("apper");
    KConfigGroup transactionDialog(&config, "TransactionDialog");
    transactionDialog.writeEntry("detailsHeight", height());
}

void ProgressView::handleRepo(bool handle) {
    if (handle) {
        setItemDelegate(m_defaultDelegate);
        m_model->setColumnCount(1);
    } else {
        setItemDelegate(m_delegate);
        m_model->setColumnCount(3);
        header()->setResizeMode(0, QHeaderView::ResizeToContents);
        header()->setResizeMode(1, QHeaderView::ResizeToContents);
        header()->setStretchLastSection(true);
    }
}

void ProgressView::currentRepo(const QString &repoId, const QString &description, bool enabled)
{
    Q_UNUSED(repoId)
    Q_UNUSED(enabled)
    QStandardItem *item = new QStandardItem(description);
    m_model->appendRow(item);
}

void ProgressView::itemProgress(const QString &id, Transaction::Status status, uint percentage)
{
    Q_UNUSED(status)
    QStandardItem *stdItem = findLastItem(id);
    if (stdItem && !stdItem->data(RoleFinished).toBool()) {
        // if the progress is unknown (101), make it empty
        if (percentage == 101) {
            percentage = 0;
        }
        if (stdItem->data(RoleProgress).toUInt() != percentage) {
            stdItem->setData(percentage, RoleProgress);
        }
    }
}

void ProgressView::clear()
{
    m_model->clear();
}

void ProgressView::currentPackage(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary)
{
    kDebug() << info << packageID << summary;
    if (!packageID.isEmpty()) {
        QStandardItem *stdItem = findLastItem(packageID);
        // If there is alread some packages check to see if it has
        // finished, if the progress is 100 create a new item for the next task
        if (stdItem && !stdItem->data(RoleFinished).toBool()) {
            // if the item status (info) changed update it
            if (stdItem->data(RoleInfo).value<Transaction::Info>() != info) {
                // If the package task has finished set progress to 100
                if (info == Transaction::InfoFinished) {
                    itemFinished(stdItem);
                } else {
                    stdItem->setData(qVariantFromValue(info), RoleInfo);
                    stdItem->setText(PkStrings::infoPresent(info));
                }
            }
        } else if (info != Transaction::InfoFinished) {
            QList<QStandardItem *> items;
            // It's a new package create it and append it
            stdItem = new QStandardItem;
            stdItem->setText(PkStrings::infoPresent(info));
            stdItem->setData(qVariantFromValue(info), RoleInfo);
            stdItem->setData(0,         RoleProgress);
            stdItem->setData(false,     RoleFinished);
            stdItem->setData(packageID, RoleId);
            items << stdItem;

            stdItem = new QStandardItem(Transaction::packageName(packageID));
            stdItem->setToolTip(Transaction::packageVersion(packageID));
            items << stdItem;

            stdItem = new QStandardItem(summary);
            stdItem->setToolTip(summary);
            items << stdItem;

            m_model->appendRow(items);
        }
    }
}

void ProgressView::itemFinished(QStandardItem *item)
{
    // Point to the item before it
    int count = item->row() - 1;

    // Find the last finished item
    bool found = false;
    while (count >= 0) {
        // Put it after the finished item
        // so that running items can be kept
        // at the bottom
        if (m_model->item(count)->data(RoleFinished).toBool()) {
            // make sure it won't end in the same position
            if (count + 1 != item->row()) {
                QList<QStandardItem*> items;
                items = m_model->takeRow(item->row());
                m_model->insertRow(count + 1, items);
            }
            found = true;
            break;
        }
        --count;
    }

    // If it's not at the top of the list
    // and no FINISHED Item was found move it there
    if (!found && item->row() != 0) {
        QList<QStandardItem*> items;
        items = m_model->takeRow(item->row());
        m_model->insertRow(0, items);
    }

    Transaction::Info info = item->data(RoleInfo).value<Transaction::Info>();
    item->setText(PkStrings::infoPast(info));
    item->setData(100,  RoleProgress);
    item->setData(true, RoleFinished);
}

void ProgressView::followBottom(int value)
{
    // If the user moves the slider to the bottom
    // keep it there as the list expands
    m_keepScrollBarBottom = value == m_scrollBar->maximum();
}

void ProgressView::rangeChanged(int min, int max)
{
    Q_UNUSED(min)
    if (m_keepScrollBarBottom && m_scrollBar->value() != max) {
        m_scrollBar->setValue(max);
    }
}

QStandardItem* ProgressView::findLastItem(const QString &packageID)
{
    int rows = m_model->rowCount() - 1;
    for (int i = rows; i >= 0; --i) {
        QStandardItem *item = m_model->item(i);
        if (item->data(RoleId).toString() == packageID) {
            return item;
        }
    }
    return 0;
}
