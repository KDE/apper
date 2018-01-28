/***************************************************************************
 *   Copyright (C) 2009-2010 by Daniel Nicoletti                           *
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

#include "TransactionModel.h"

#include <KUser>
#include <KFormat>
#include <KLocalizedString>
#include <QLoggingCategory>

#include <PkIcons.h>
#include <PkStrings.h>

using namespace PackageKit;

TransactionModel::TransactionModel(QObject *parent)
: QStandardItemModel(parent)
{
    setSortRole(Qt::DisplayRole);
    clear();
}

void TransactionModel::clear()
{
    QStandardItemModel::clear();
    setHorizontalHeaderItem(0, new QStandardItem(i18n("Date")));
    setHorizontalHeaderItem(1, new QStandardItem(i18n("Action")));
    setHorizontalHeaderItem(2, new QStandardItem(i18n("Details")));
    setHorizontalHeaderItem(3, new QStandardItem(i18nc("Machine user who issued the transaction", "Username")));
    setHorizontalHeaderItem(4, new QStandardItem(i18n("Application")));
}

void TransactionModel::addTransaction(PackageKit::Transaction *trans)
{
    auto dateI    = new QStandardItem;
    auto roleI    = new QStandardItem;
    auto detailsI = new QStandardItem;
    auto userI    = new QStandardItem;
    auto appI     = new QStandardItem;

    dateI->setText(QLocale::system().toString(trans->timespec().date()));
    // this is for the filterSort model
    dateI->setData(trans->timespec(), Qt::UserRole);
    dateI->setEditable(false);

    roleI->setText(PkStrings::actionPast(trans->role()));
    roleI->setIcon(PkIcons::actionIcon(trans->role()));
    roleI->setEditable(false);

    detailsI->setText(getDetailsLocalized(trans->data()));
    detailsI->setEditable(false);

    KUser user(trans->uid());
    QString display;
    if (!user.property(KUser::FullName).toString().isEmpty()) {
        display = user.property(KUser::FullName).toString() + QLatin1String(" (") + user.loginName() + QLatin1Char(')');
    } else {
        display = user.loginName();
    }
    userI->setText(display);
    userI->setEditable(false);

    appI->setText(trans->cmdline());
    appI->setEditable(false);

    appendRow({
                  dateI,
                  roleI,
                  detailsI,
                  userI,
                  appI
              });
    delete trans;
}

QString TransactionModel::getDetailsLocalized(const QString &data) const
{
    QStringList lines = data.split(QLatin1Char('\n'));
    QStringList ret;

    QString text;
    text = getTypeLine(lines, Transaction::StatusInstall);
    if (!text.isNull()) {
        ret << text;
    }

    text = getTypeLine(lines, Transaction::StatusRemove);
    if (!text.isNull()) {
        ret << text;
    }

    text = getTypeLine(lines, Transaction::StatusUpdate);
    if (!text.isNull()) {
        ret << text;
    }

    return ret.join(QLatin1Char('\n'));
}

QString TransactionModel::getTypeLine(const QStringList &lines, Transaction::Status status) const
{
    QStringList text;
    for (const QString &line : lines) {
        const QStringList sections = line.split(QLatin1Char('\t'));
        if (sections.size() > 1) {
            switch (status) {
                case Transaction::StatusInstall:
                    if (sections.at(0) != QLatin1String("installing")) {
                        continue;
                    }
                    break;
                case Transaction::StatusRemove:
                    if (sections.at(0) != QLatin1String("removing")) {
                        continue;
                    }
                    break;
                case Transaction::StatusUpdate:
                    if (sections.at(0) != QLatin1String("updating")) {
                        continue;
                    }
                    break;
                default:
                    continue;
            }
            QStringList packageData = sections.at(1).split(QLatin1Char(';'));
            if (packageData.size()) {
                text << packageData.at(0);
            }
        }
    }

    if (text.size()) {
        // TODO make the status BOLD
        return PkStrings::statusPast(status) + QLatin1String(": ") + text.join(QLatin1String(", "));
    } else {
        return QString();
    }
}

#include "moc_TransactionModel.cpp"
