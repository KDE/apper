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

#include "ApplicationLauncher.h"

#include <QStandardItemModel>
#include <KToolInvocation>
#include <KLocale>
#include <KDebug>

ApplicationLauncher::ApplicationLauncher(const QVector<KService*> &applications, QWidget *parent )
 : QDialog(parent)
{
    setupUi(this);

    label->setText(i18np("The following application was just installed, click on it to launch:",
                         "The following applications where just installed, click on them to launch:",
                         applications.size()));

    QStandardItemModel *model = new QStandardItemModel(this);
    QStandardItem *item;
    foreach (const KService *service, applications) {
        QString name = service->genericName().isEmpty() ?
                           service->property("Name").toString() :
                           service->property("Name").toString() + " - " + service->genericName();
        item = new QStandardItem(name);
        item->setIcon(KIcon(service->icon()));
        item->setData(service->desktopEntryPath(), Qt::UserRole);
        model->appendRow(item);
    }
    applicationsView->setModel(model);
}

ApplicationLauncher::~ApplicationLauncher()
{
    if (showCB->isChecked()) {
        KConfig config("KPackageKit");
        KConfigGroup transactionGroup(&config, "Transaction");
        transactionGroup.writeEntry("ShowApplicationLauncher", false);
    }
}

void ApplicationLauncher::on_applicationsView_clicked(const QModelIndex &index)
{
//     kDebug() << index.data(Qt::UserRole).toString();
    KToolInvocation::startServiceByDesktopPath(index.data(Qt::UserRole).toString());
}

#include "ApplicationLauncher.moc"
