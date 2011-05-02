/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
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

#include "TransactionTrayIcon.h"

#include <KMenu>
#include <KLocale>
#include <KActionCollection>
#include <KDebug>

#include <Transaction>

using namespace PackageKit;

TransactionTrayIcon::TransactionTrayIcon(QObject *parent)
 : KStatusNotifierItem(parent)
{
    setCategory(KStatusNotifierItem::SystemServices);
    setStatus(KStatusNotifierItem::Active);

    // Remove standard quit action, as it would quit all of KDED
    KActionCollection *actions = actionCollection();
    actions->removeAction(actions->action(KStandardAction::name(KStandardAction::Quit)));
    connect(contextMenu(), SIGNAL(triggered(QAction *)),
            this, SLOT(actionActivated(QAction *)));
    setAssociatedWidget(contextMenu());
}

TransactionTrayIcon::~TransactionTrayIcon()
{
}

void TransactionTrayIcon::actionActivated(QAction *action)
{
    kDebug();
    // Check to see if there is transaction id in action
    if (!action->data().isNull()) {
        // we need to find if the action clicked has already a dialog
        Transaction *t = new Transaction(action->data().toString(), this);
        if (!t->error()) {
            emit transactionActivated(t);
        }
    }
}
