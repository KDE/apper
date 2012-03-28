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
#include <KPushButton>
#include <KMenuBar>
#include <KDebug>
#include <listaller.h>

#include "InfoWidget.h"

class SetupWizardPrivate
{
    public:
        SetupWizardPrivate()
            : liSetup(NULL)
        {
            // Create a new Listaller settings module
            liConf = listaller_settings_new (false);
        }
        ~SetupWizardPrivate()
        {
	   if (liSetup != NULL)
	      g_object_unref (liSetup);
            g_object_unref (liConf);
        }

        ListallerSettings *liConf;
        ListallerSetup *liSetup;
	InfoWidget *infoPage;
};

void on_lisetup_message (GObject *sender, ListallerMessageItem *message, SetupWizard *self)
{
    Q_UNUSED(*sender);
    Q_UNUSED(self);
    kDebug() << "Listaller message:" << listaller_message_item_get_details(message);
}

void on_lisetup_status_changed (GObject *sender, ListallerStatusItem *status, SetupWizard *self)
{
    Q_UNUSED(*sender);
    Q_UNUSED(self);
    Q_UNUSED(status);
}

void on_lisetup_error_code (GObject *sender, ListallerErrorItem *error, SetupWizard *self)
{
    Q_UNUSED(*sender);

    SetupWizardPrivate *d = self->getPriv();
    ListallerErrorEnum etype = listaller_error_item_get_error(error);
    d->infoPage->reset();
    d->infoPage->setWindowTitle(listaller_error_enum_to_string(etype));
    d->infoPage->setDescription("An error occured");
    d->infoPage->setIcon(KIcon("dialog-error"));
    d->infoPage->setDetails(listaller_error_item_get_details(error));
    //self->setMainWidget(d->infoPage);
    self->setButtons(KDialog::Close);
    self->button(KDialog::Close)->setFocus();
}

void on_lisetup_progress_changed (GObject *sender, int progress, int subProgress, SetupWizard *self)
{
    Q_UNUSED(*sender);
    Q_UNUSED(self);
}

SetupWizard::SetupWizard(const QString& ipkFName, QWidget *parent)
    : KDialog (parent),
      d(new SetupWizardPrivate()),
      ui(new Ui::SetupWizard)
{
    ui->setupUi(KDialog::mainWidget());
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowIcon(KIcon("applications-other"));
    setButtons(KDialog::Ok | KDialog::Cancel);
    setButtonText(KDialog::Ok, i18n("Continue"));
    setButtonIcon(KDialog::Ok, KIcon("go-next"));
    enableButtonOk(true);

    d->infoPage = new InfoWidget(this);
    d->infoPage->setDescription("AppSetup template");
    d->infoPage->setDetails("This will become a simple Listaller setup wizard");

    ui->stackedWidget->addWidget(d->infoPage);

    setMinimumSize(QSize(430,280));
    KConfig config("apper");
    KConfigGroup configGroup(&config, "AppInstaller");
    restoreDialogSize(configGroup);

    // Create a new Listaller application setup instance
    d->liSetup = listaller_setup_new (ipkFName.toUtf8(), d->liConf);
    g_signal_connect (d->liSetup, "message", (GCallback) on_lisetup_message, this);
    g_signal_connect (d->liSetup, "status-changed", (GCallback) on_lisetup_status_changed, this);
    g_signal_connect (d->liSetup, "progress-changed", (GCallback) on_lisetup_progress_changed, this);
    g_signal_connect (d->liSetup, "error-code", (GCallback) on_lisetup_error_code, this);
}

SetupWizard::~SetupWizard()
{
    delete d;
    delete ui;
}

void SetupWizard::slotButtonClicked(int button)
{
    //if (button == KDialog::Ok) {
    //     initialize();
    //}
}

bool SetupWizard::initialize()
{
    return listaller_setup_initialize(d->liSetup);
}
