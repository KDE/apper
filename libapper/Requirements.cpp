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

Requirements::Requirements(SimulateModel *model, QWidget *parent) :
    KDialog(parent),
    m_embed(false),
    m_shouldShow(true),
    ui(new Ui::Requirements)
{
    ui->setupUi(mainWidget());
    connect(ui->confirmCB, SIGNAL(toggled(bool)), this, SLOT(on_confirmCB_Toggled(bool)));

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
    connect(group, SIGNAL(buttonClicked(int)), this, SLOT(actionClicked(int)));

    int count = 0;
    if (int c = model->countInfo(Package::InfoRemoving)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to remove", "%1 packages to remove", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleRemovePackages));
        group->addButton(button, Package::InfoRemoving);
        ui->verticalLayout->insertWidget(count++, button);

        m_hideAutoConfirm = true;
    }

    if (int c = model->countInfo(Package::InfoDowngrading)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to downgrade", "%1 packages to downgrade", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleRollback));
        group->addButton(button, Package::InfoDowngrading);
        ui->verticalLayout->insertWidget(count++, button);

        m_hideAutoConfirm = true;
    }

    if (int c = model->countInfo(Package::InfoReinstalling)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to reinstall", "%1 packages to reinstall", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleRemovePackages));
        group->addButton(button, Package::InfoReinstalling);
        ui->verticalLayout->insertWidget(count++, button);
    }

    if (int c = model->countInfo(Package::InfoInstalling)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to install", "%1 packages to install", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleInstallPackages));
        group->addButton(button, Package::InfoInstalling);
        ui->verticalLayout->insertWidget(count++, button);
    }

    if (int c = model->countInfo(Package::InfoUpdating)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to update", "%1 packages to update", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleUpdatePackages));
        group->addButton(button, Package::InfoUpdating);
        ui->verticalLayout->insertWidget(count++, button);
    }

    QList<QAbstractButton *> buttons = group->buttons();
    if (!buttons.isEmpty()) {
        QAbstractButton *button = group->buttons().first();
        button->setChecked(true);

        ui->packageView->header()->setResizeMode(0, QHeaderView::ResizeToContents);
//        ui->packageView->header()->setResizeMode(1, QHeaderView::Stretch);

        if (m_hideAutoConfirm) {
            ui->confirmCB->setVisible(false);
        } else {
            // if the confirmCB is visible means that we can skip this
            // dialog, but only if the user previusly set so
            ui->confirmCB->setChecked(requirementsDialog.readEntry("autoConfirm", false));
        }
    } else {
        // set this as false so the dialog is not shown
        m_shouldShow = false;
    }
}

Requirements::~Requirements()
{
    KConfig config("apper");
    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    saveDialogSize(requirementsDialog);

    delete ui;
}

bool Requirements::embedded() const
{
    return m_embed;
}

void Requirements::setEmbedded(bool embedded)
{
    m_embed = embedded;
    ui->label->setVisible(!embedded);
}

bool Requirements::shouldShow() const
{
    return (m_shouldShow && !ui->confirmCB->isChecked());
}

void Requirements::on_confirmCB_Toggled(bool checked)
{
    KConfig config("apper");
    KConfigGroup requirementsDialog(&config, "requirementsDialog");

    if (!m_hideAutoConfirm) {
        requirementsDialog.writeEntry("autoConfirm", checked);
    }
    config.sync();
}

void Requirements::actionClicked(int type)
{
    SimulateModel *model = static_cast<SimulateModel*>(ui->packageView->model());
    model->setCurrentInfo(static_cast<Package::Info>(type));
}

#include "Requirements.moc"
