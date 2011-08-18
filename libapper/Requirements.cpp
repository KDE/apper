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

#include "Requirements.h"
#include "ui_Requirements.h"

#include "PkIcons.h"
#include "SimulateModel.h"

#include <QToolButton>
#include <KDebug>

Requirements::Requirements(SimulateModel *model, QWidget *parent)
 : KDialog(parent),
   ui(new Ui::Requirements)
{
    ui->setupUi(mainWidget());

    ui->packageView->setModel(model);
    m_hideAutoConfirm = false;

    setCaption(i18n("Additional changes"));
    setButtons(KDialog::Ok | KDialog::Cancel);
    setButtonText(KDialog::Ok, i18n("Continue"));
    // restore size
    setMinimumSize(QSize(450,300));
    setInitialSize(QSize(450,300));
    KConfig config("apper");
    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    restoreDialogSize(requirementsDialog);

    QButtonGroup *group = new QButtonGroup(this);
    if (int c = model->countInfo(Package::InfoRemoving)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setAutoExclusive(true);
        button->setChecked(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to remove", "%1 packages to remove", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleRemovePackages));
        group->addButton(button, Package::InfoRemoving);
        ui->verticalLayout->addWidget(button);

        model->setCurrentInfo(Package::InfoRemoving);
        m_hideAutoConfirm = true;
    }

    if (int c = model->countInfo(Package::InfoDowngrading)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setAutoExclusive(true);
        button->setChecked(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to downgrade", "%1 packages to downgrade", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleRollback));
        group->addButton(button, Package::InfoDowngrading);
        ui->verticalLayout->addWidget(button);

        if (model->currentInfo() == Package::UnknownInfo) {
            model->setCurrentInfo(Package::InfoDowngrading);
        }
        m_hideAutoConfirm = true;
    }

    if (int c = model->countInfo(Package::InfoReinstalling)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setAutoExclusive(true);
        button->setChecked(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to reinstall", "%1 packages to reinstall", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleRemovePackages));
        group->addButton(button, Package::InfoReinstalling);
        ui->verticalLayout->addWidget(button);

        if (model->currentInfo() == Package::UnknownInfo) {
            model->setCurrentInfo(Package::InfoReinstalling);
        }
    }

    if (int c = model->countInfo(Package::InfoInstalling)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setAutoExclusive(true);
        button->setChecked(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to install", "%1 packages to install", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleInstallPackages));
        group->addButton(button, Package::InfoInstalling);
        ui->verticalLayout->addWidget(button);

        if (model->currentInfo() == Package::UnknownInfo) {
            model->setCurrentInfo(Package::InfoInstalling);
        }
    }

    QToolButton *button = new QToolButton(this);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    button->setCheckable(true);
    button->setAutoRaise(true);
    button->setAutoExclusive(true);
    button->setChecked(true);
    button->setIconSize(QSize(32, 32));
    button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    button->setText(i18np("1 package to update", "%1 packages to update", 350));
    button->setIcon(PkIcons::actionIcon(Transaction::RoleUpdatePackages));
    group->addButton(button, Package::InfoUpdating);
    ui->verticalLayout->addWidget(button);

    if (model->currentInfo() == Package::UnknownInfo) {
        model->setCurrentInfo(Package::InfoUpdating);
    }
    if (int c = model->countInfo(Package::InfoUpdating)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setAutoExclusive(true);
        button->setChecked(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to update", "%1 packages to update", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleUpdatePackages));
        group->addButton(button, Package::InfoUpdating);
        ui->verticalLayout->addWidget(button);

        if (model->currentInfo() == Package::UnknownInfo) {
            model->setCurrentInfo(Package::InfoUpdating);
        }
    }

    ui->verticalLayout->addStretch(group->buttons().size());
    ui->packageView->header()->setResizeMode(0, QHeaderView::Stretch);
    ui->packageView->header()->setResizeMode(1, QHeaderView::ResizeToContents);

    if (m_hideAutoConfirm) {
        ui->confirmCB->setVisible(false);
    } else {
        // if the confirmCB is visible means that we can skip this
        // dialog, but only if the user previusly set so
        ui->confirmCB->setChecked(requirementsDialog.readEntry("autoConfirm", false));
    }
//    ui->toolButton->setIcon(PkIcons::actionIcon(Transaction::RoleUpdatePackages));
}

Requirements::~Requirements()
{
    // save size
    KConfig config("apper");
    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    saveDialogSize(requirementsDialog);

    if (!m_hideAutoConfirm) {
        requirementsDialog.writeEntry("autoConfirm", ui->confirmCB->isChecked());
    }
    config.sync();
    delete ui;
}

void Requirements::show()
{
    if (ui->confirmCB->isChecked()) {
        emit accepted();
    } else{
        KDialog::show();
    }
}

void Requirements::setPlainCaption(const QString &caption)
{
    Q_UNUSED(caption)
    kDebug();
    KDialog::setPlainCaption(ui->label->text());
    ui->label->hide();
}

void Requirements::actionClicked(const QModelIndex &index)
{
//    Package::Info state = index.data(Qt::UserRole+1).value<Package::Info>();
//    static_cast<SimulateModel*>(ui->packageView->model())->setCurrentInfo(state);
}

#include "Requirements.moc"
