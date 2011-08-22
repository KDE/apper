/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
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

#include "ApplicationLauncher.h"
#include "ui_ApplicationLauncher.h"

#include <QStandardItemModel>
#include <KToolInvocation>
#include <KLocale>
#include <KDebug>

ApplicationLauncher::ApplicationLauncher(QWidget *parent) :
    KDialog(parent),
    m_embed(false),
    ui(new Ui::ApplicationLauncher)
{
    ui->setupUi(mainWidget());
    setObjectName("ApplicationLauncher");

    connect(ui->kdialogbuttonbox, SIGNAL(rejected()), this, SLOT(accept()));
    setButtons(KDialog::None);

    connect(ui->applicationsView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(itemClicked(QModelIndex)));
}

ApplicationLauncher::~ApplicationLauncher()
{
    if (ui->showCB->isChecked()) {
        KConfig config("apper");
        KConfigGroup transactionGroup(&config, "Transaction");
        transactionGroup.writeEntry("ShowApplicationLauncher", false);
    }
    delete ui;
}

bool ApplicationLauncher::embedded() const
{
    return m_embed;
}

void ApplicationLauncher::setEmbedded(bool embedded)
{
    m_embed = embedded;
    ui->showCB->setVisible(!embedded);
    ui->kdialogbuttonbox->setVisible(!embedded);
    kDebug() << embedded;
}

QList<PackageKit::Package> ApplicationLauncher::packages() const
{
    return m_packages;
}

bool ApplicationLauncher::hasApplications()
{
    QStandardItemModel *model = new QStandardItemModel(this);
    ui->applicationsView->setModel(model);
    m_files.removeDuplicates();

    QStandardItem *item;
    foreach (const QString &desktop, m_files) {
        // we create a new KService because findByDestopPath
        // might fail because the Sycoca database is not up to date yet.
        KService *service = new KService(desktop);
        if (service->isApplication() &&
           !service->noDisplay() &&
           !service->exec().isEmpty())
        {
            QString name;
            name = service->genericName().isEmpty() ?
                        service->property("Name").toString() :
                        service->property("Name").toString() + " - " + service->genericName();
            item = new QStandardItem(name);
            item->setIcon(KIcon(service->icon()));
            item->setData(service->desktopEntryPath(), Qt::UserRole);
            item->setFlags(Qt::ItemIsEnabled);
            model->appendRow(item);
        }
    }

    ui->label->setText(i18np("The following application was just installed. Click on it to launch:",
                             "The following applications were just installed. Click on them to launch:",
                             model->rowCount()));

    return model->rowCount();
}

void ApplicationLauncher::addPackage(const PackageKit::Package &package)
{
    if (!m_packages.contains(package)) {
        m_packages.append(package);
    }
}

void ApplicationLauncher::files(const PackageKit::Package &package, const QStringList &files)
{
    Q_UNUSED(package)
    m_files.append(files.filter(".desktop"));
}

void ApplicationLauncher::itemClicked(const QModelIndex &index)
{
//     kDebug() << index.data(Qt::UserRole).toString();
    KToolInvocation::startServiceByDesktopPath(index.data(Qt::UserRole).toString());
}

#include "ApplicationLauncher.moc"
