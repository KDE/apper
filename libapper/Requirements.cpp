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
#include "PackageModel.h"
#include "ApplicationSortFilterModel.h"

#include <QToolButton>
#include <KPushButton>
#include <KDebug>

Requirements::Requirements(PackageModel *model, QWidget *parent) :
    KDialog(parent),
    m_embed(false),
    m_shouldShow(true),
    m_untrustedButton(0),
    ui(new Ui::Requirements)
{
    setAttribute(Qt::WA_DeleteOnClose);

    ui->setupUi(mainWidget());
    connect(ui->confirmCB, SIGNAL(toggled(bool)), this, SLOT(on_confirmCB_Toggled(bool)));

    ApplicationSortFilterModel *proxy = new ApplicationSortFilterModel(this);
    proxy->setSourceModel(model);
    ui->packageView->setModel(proxy);
    ui->packageView->header()->setResizeMode(PackageModel::NameCol, QHeaderView::ResizeToContents);
    ui->packageView->header()->hideSection(PackageModel::ActionCol);
    ui->packageView->header()->hideSection(PackageModel::ArchCol);
    ui->packageView->header()->hideSection(PackageModel::CurrentVersionCol);
    ui->packageView->header()->hideSection(PackageModel::OriginCol);
    ui->packageView->header()->hideSection(PackageModel::SizeCol);

    m_hideAutoConfirm = false;

    setCaption(i18n("Additional changes"));
    setWindowIcon(KIcon("dialog-warning"));
    setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Help);
    setButtonText(KDialog::Ok, i18n("Continue"));
    // restore size
    setMinimumSize(QSize(600,480));
    setInitialSize(QSize(600,600));
    KConfig config("apper");
    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    restoreDialogSize(requirementsDialog);

    button(KDialog::Help)->setFlat(true);
    button(KDialog::Help)->setEnabled(false);
    button(KDialog::Help)->setIcon(KIcon("download"));

    m_buttonGroup = new QButtonGroup(this);
    connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(actionClicked(int)));

    int count = 0;
    if (int c = model->countInfo(Transaction::InfoRemoving)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to remove", "%1 packages to remove", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleRemovePackages));
        m_buttonGroup->addButton(button, Transaction::InfoRemoving);
        ui->verticalLayout->insertWidget(count++, button);

        m_hideAutoConfirm = true;
    }

    if (int c = model->countInfo(Transaction::InfoDowngrading)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to downgrade", "%1 packages to downgrade", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleRepairSystem));
        m_buttonGroup->addButton(button, Transaction::InfoDowngrading);
        ui->verticalLayout->insertWidget(count++, button);

        m_hideAutoConfirm = true;
    }

    if (int c = model->countInfo(Transaction::InfoReinstalling)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to reinstall", "%1 packages to reinstall", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleRemovePackages));
        m_buttonGroup->addButton(button, Transaction::InfoReinstalling);
        ui->verticalLayout->insertWidget(count++, button);
    }

    if (int c = model->countInfo(Transaction::InfoInstalling)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to install", "%1 packages to install", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleInstallPackages));
        m_buttonGroup->addButton(button, Transaction::InfoInstalling);
        ui->verticalLayout->insertWidget(count++, button);
    }

    if (int c = model->countInfo(Transaction::InfoUpdating)) {
        QToolButton *button = new QToolButton(this);
        button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        button->setCheckable(true);
        button->setAutoRaise(true);
        button->setIconSize(QSize(32, 32));
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        button->setText(i18np("1 package to update", "%1 packages to update", c));
        button->setIcon(PkIcons::actionIcon(Transaction::RoleUpdatePackages));
        m_buttonGroup->addButton(button, Transaction::InfoUpdating);
        ui->verticalLayout->insertWidget(count++, button);
    }

    if (int c = model->countInfo(Transaction::InfoUntrusted)) {
        m_untrustedButton = new QToolButton(this);
        m_untrustedButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        m_untrustedButton->setCheckable(true);
        m_untrustedButton->setAutoRaise(true);
        m_untrustedButton->setIconSize(QSize(32, 32));
        m_untrustedButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        m_untrustedButton->setText(i18np("1 untrusted package", "%1 untrusted packages", c));
        m_untrustedButton->setIcon(KIcon("security-low"));
        m_untrustedButton->setVisible(false);
        ui->verticalLayout->insertWidget(count++, m_untrustedButton);
    }

    if (!m_buttonGroup->buttons().isEmpty()) {
        m_buttonGroup->buttons().first()->click();

        if (m_hideAutoConfirm) {
            ui->confirmCB->setVisible(false);
        } else {
            // if the confirmCB is visible means that we can skip this
            // dialog, but only if the user previusly set so
            ui->confirmCB->setChecked(requirementsDialog.readEntry("autoConfirm", false));
        }
    } else if (m_untrustedButton) {
        showUntrustedButton();
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

void Requirements::setDownloadSizeRemaining(qulonglong size)
{
    if (size) {
        QString text;
        text = i18nc("how many bytes are required for download",
                     "Need to get %1 of archives",
                     KGlobal::locale()->formatByteSize(size));
        button(KDialog::Help)->setText(text);
        button(KDialog::Help)->setToolTip(text);
        button(KDialog::Help)->show();
    } else {
        button(KDialog::Help)->hide();
    }
}

bool Requirements::trusted() const
{
    // There are untrusted packages if the button was created...
    return !m_untrustedButton;
}

bool Requirements::shouldShow() const
{
    return (m_shouldShow && !ui->confirmCB->isChecked());
}

void Requirements::slotButtonClicked(int button)
{
    if (button == KDialog::Ok &&
            m_untrustedButton &&
            !m_untrustedButton->isVisible()) {
        showUntrustedButton();
    } else {
        KDialog::slotButtonClicked(button);
    }
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
    ApplicationSortFilterModel *proxy;
    proxy = qobject_cast<ApplicationSortFilterModel*>(ui->packageView->model());
    proxy->setInfoFilter(static_cast<Transaction::Info>(type));
}

void Requirements::showUntrustedButton()
{
    // Clear the other buttons
    foreach (QAbstractButton *button, m_buttonGroup->buttons()) {
        delete button;
    }

    // Hide the auto confirm button since we will be showing this dialog anyway
    ui->confirmCB->setVisible(false);

    ui->label->setText(i18n("You are about to install unsigned packages that can compromise your system, "
                            "as it is impossible to verify if the software came from a trusted source."));
    m_untrustedButton->setVisible(true);
    m_buttonGroup->addButton(m_untrustedButton, Transaction::InfoUntrusted);
    m_untrustedButton->click();
}

#include "Requirements.moc"
