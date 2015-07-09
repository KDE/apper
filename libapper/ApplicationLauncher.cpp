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
#include <KLocalizedString>
#include <KDebug>
#include <KService>
#include <KConfig>

ApplicationLauncher::ApplicationLauncher(QWidget *parent) :
    KDialog(parent),
    m_embed(false),
    ui(new Ui::ApplicationLauncher)
{
    ui->setupUi(mainWidget());
    connect(ui->showCB, SIGNAL(toggled(bool)), this, SLOT(on_showCB_toggled(bool)));
    setObjectName("ApplicationLauncher");

    connect(ui->kdialogbuttonbox, SIGNAL(rejected()), this, SLOT(accept()));
    setButtons(KDialog::None);
    setWindowIcon(QIcon::fromTheme("task-complete"));

    connect(ui->applicationsView, SIGNAL(clicked(QModelIndex)),
            this, SLOT(itemClicked(QModelIndex)));
}

ApplicationLauncher::~ApplicationLauncher()
{
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

QStringList ApplicationLauncher::packages() const
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
        // we use KService to parse the .desktop file because findByDestopPath
        // might fail because the Sycoca database is not up to date yet.
        KService service(desktop);
        if (service.isApplication() &&
           !service.noDisplay() &&
           !service.exec().isEmpty())
        {
            QString name = service.genericName().isEmpty() ? service.name()
                                                           : service.name() + " - " + service.genericName();
            item = new QStandardItem(name);
            item->setIcon(QIcon::fromTheme(service.icon()));
            item->setData(service.entryPath(), Qt::UserRole);
            item->setFlags(Qt::ItemIsEnabled);
            model->appendRow(item);
        }
    }

    setWindowTitle(i18np("New application available",
                         "New applications available",
                         model->rowCount()));
    ui->label->setText(i18np("The following application was just installed. Click on it to launch:",
                             "The following applications were just installed. Click on them to launch:",
                             model->rowCount()));

    return model->rowCount();
}

void ApplicationLauncher::addPackage(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary)
{
    Q_UNUSED(info)
    Q_UNUSED(summary)
    if (!m_packages.contains(packageID)) {
        m_packages << packageID;
    }
}

void ApplicationLauncher::files(const QString &packageID, const QStringList &files)
{
    Q_UNUSED(packageID)
    m_files.append(files.filter(".desktop"));
}

void ApplicationLauncher::itemClicked(const QModelIndex &index)
{
//     kDebug() << index.data(Qt::UserRole).toString();
    KToolInvocation::startServiceByDesktopPath(index.data(Qt::UserRole).toString());
}

void ApplicationLauncher::on_showCB_toggled(bool checked)
{
    KConfig config("apper");
    KConfigGroup transactionGroup(&config, "Transaction");
    transactionGroup.writeEntry("ShowApplicationLauncher", !checked);
    config.sync();
}

#include "ApplicationLauncher.moc"
