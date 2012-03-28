/* This file is part of Apper
 *
 * Copyright (C) 2012 Matthias Klumpp <matthias@tenstral.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "SetupWizard.h"
#include "ui_SetupWizard.h"

#include <QLabel>
#include <QApplication>
#include <QAction>
#include <KMenuBar>
#include <KDebug>

#include "InfoWidget.h"

SetupWizard::SetupWizard(QWidget *parent)
    : KDialog (parent),
      ui(new Ui::SetupWizard)
{
    ui->setupUi(KDialog::mainWidget());
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowIcon(KIcon("applications-other"));
    setButtons(KDialog::Ok | KDialog::Cancel);
    setButtonText(KDialog::Ok, i18n("Continue"));
    setButtonIcon(KDialog::Ok, KIcon("go-next"));
    enableButtonOk(false);

    InfoWidget *info = new InfoWidget(this);
    info->setDescription("AppSetup template");
    info->setDetails("This will become a simple Listaller setup wizard");

    ui->stackedWidget->addWidget(info);

    setMinimumSize(QSize(430,280));
    KConfig config("apper");
    KConfigGroup configGroup(&config, "AppInstaller");
    restoreDialogSize(configGroup);
}

SetupWizard::~SetupWizard()
{
    delete ui;
}
