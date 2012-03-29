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
#include <QProgressBar>
#include <KPushButton>
#include <KMenuBar>
#include <KDebug>
#include <listaller.h>

#include "InfoWidget.h"
#include "SimplePage.h"
#include "LicensePage.h"

class SetupWizardPrivate
{
public:
    SetupWizardPrivate()
        : liSetup(NULL)
    {
        // Create a new Listaller settings module
        liConf = listaller_settings_new (false);
        PAGE_COUNT = -1;
    }
    ~SetupWizardPrivate()
    {
        if (liSetup != NULL)
            g_object_unref (liSetup);
        g_object_unref (liConf);
    }

    int PAGE_COUNT;
    ListallerSettings *liConf;
    ListallerSetup *liSetup;
    ListallerAppItem *appID;
    InfoWidget *infoPage;
    SimplePage *execPage;
    QProgressBar *mainProgressBar;
};

void on_lisetup_message (GObject *sender, ListallerMessageItem *message, SetupWizard *self)
{
    Q_UNUSED(sender);
    Q_UNUSED(self);
    kDebug() << "Listaller message:" << listaller_message_item_get_details(message);
}

void on_lisetup_status_changed (GObject *sender, ListallerStatusItem *status, SetupWizard *self)
{
    Q_UNUSED(sender);
    Q_UNUSED(status);
    SetupWizardPrivate *d = self->getPriv();

    ListallerStatusEnum statusType = listaller_status_item_get_status(status);

    if (statusType == LISTALLER_STATUS_ENUM_INSTALLATION_FINISHED) {
        QString appName = listaller_app_item_get_full_name(d->appID);
        d->infoPage->reset();
        d->infoPage->setWindowTitle(i18n("Installation finished!"));
        d->infoPage->setDescription(i18n("%1 has been installed successfully!", appName));
        d->infoPage->setIcon(KIcon("dialog-ok-apply"));
        self->setButtons(KDialog::Close);
        self->button(KDialog::Close)->setFocus();
        self->setCurrentPage(d->infoPage);
    }
}

void on_lisetup_error_code (GObject *sender, ListallerErrorItem *error, SetupWizard *self)
{
    Q_UNUSED(*sender);
    SetupWizardPrivate *d = self->getPriv();

    d->infoPage->reset();
    d->infoPage->setWindowTitle(i18n("Error"));
    d->infoPage->setDescription(i18n("An error occured"));
    d->infoPage->setIcon(KIcon("dialog-error"));
    d->infoPage->setDetails(listaller_error_item_get_details(error));
    self->setButtons(KDialog::Close);
    self->button(KDialog::Close)->setFocus();
    self->setCurrentPage(d->infoPage);
}

void on_lisetup_progress_changed (GObject *sender, int progress, int subProgress, SetupWizard *self)
{
    Q_UNUSED(sender);
    Q_UNUSED(subProgress);
    SetupWizardPrivate *d = self->getPriv();

    d->mainProgressBar->setValue(progress);
}

SetupWizard::SetupWizard(const QString& ipkFName, QWidget *parent)
    : KDialog (parent),
      d(new SetupWizardPrivate()),
      ui(new Ui::SetupWizard)
{
    ui->setupUi(KDialog::mainWidget());
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowIcon(KIcon("applications-other"));
    setButtons(KDialog::User1 | KDialog::Ok | KDialog::Cancel);
    setButtonText(KDialog::Ok, i18n("Continue"));
    setButtonIcon(KDialog::Ok, KIcon("go-next"));
    setButtonText(KDialog::User1, i18n("Back"));
    setButtonIcon(KDialog::User1, KIcon("go-previous"));

    enableButtonOk(true);

    connect(ui->stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(currentPageChanged(int)));

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

    // Initialize the setup, required to have an AppItem present
    initialize();

    // Build layout of our setup wizard
    constructWizardLayout();
}

SetupWizard::~SetupWizard()
{
    delete d;
    delete ui;
}

bool SetupWizard::constructWizardLayout()
{
    if (d->PAGE_COUNT > 0)
        return true;

    d->infoPage = new InfoWidget(this);
    d->infoPage->setWindowTitle("Information");
    ui->stackedWidget->addWidget(d->infoPage);

    if (d->appID == NULL) {
        kDebug() << "AppID was NULL!";
        return false;
    }

    QString appName = listaller_app_item_get_full_name(d->appID);

    // Welcome page
    SimplePage *introP = new SimplePage(this);
    introP->setTitle(i18n("Welcome!"));
    introP->setDescription(i18n("Welcome to the installation of %1!", appName));
    introP->setDetails(i18n("Please be careful while installing 3rd-party applications.<br>"
                            "They may eventually <b>damage</b> your system or "
                            "have <b>malicious behaviour</b>, like spying out your passwords.<br>"
                            "Please do only install this software if you <b>trust the "
                            "publisher and the author</b> of it."));
    ui->stackedWidget->addWidget(introP);

    // Setup details page
    SimplePage *detailsP = new SimplePage(this);
    detailsP->setTitle(i18n("Details"));
    detailsP->setDescription(i18n("Details about this installation"));
    detailsP->setDetails("::TODO");
    ui->stackedWidget->addWidget(detailsP);

    // Application description page
    SimplePage *descP = new SimplePage(this);
    descP->setTitle(i18n("Application description"));
    descP->setDescription(i18n("Description"));
    descP->setDetails(listaller_app_item_get_description(d->appID));
    ui->stackedWidget->addWidget(descP);

    // License page
    ListallerAppLicense appLicense;
    listaller_app_item_get_license(d->appID, &appLicense);
    LicensePage *licenseP = new LicensePage(this);
    licenseP->setTitle(i18n("Software license"));
    licenseP->setDescription(i18n("Please read the following terms and conditions carefully!"));
    licenseP->setLicenseText(appLicense.text);
    connect(licenseP, SIGNAL(licenseAccepted(bool)), this, SLOT(licenseAccepted(bool)));
    ui->stackedWidget->addWidget(licenseP);

    // Setup exec page
    d->execPage = new SimplePage(this);
    d->execPage->setTitle(i18n("Running installation"));
    d->execPage->setDescription(i18n("Installing %1...", appName));
    d->mainProgressBar = new QProgressBar(d->execPage);
    d->execPage->addWidget(d->mainProgressBar);
    ui->stackedWidget->addWidget(d->execPage);

    d->PAGE_COUNT = 5;
    setCurrentPage(introP);

    return true;
}

void SetupWizard::currentPageChanged(int index)
{
    // Set the page title
    ui->titleL->setText(ui->stackedWidget->currentWidget()->windowTitle());

    // Show "back" button if necessary
    if (index > 1)
        enableButton(KDialog::User1, true);
    else
        enableButton(KDialog::User1, false);
}

void SetupWizard::setCurrentPage(QWidget* widget)
{
    ui->stackedWidget->setCurrentWidget(widget);
}

void SetupWizard::slotButtonClicked(int button)
{
    if (button == KDialog::Ok) {
        // We go forward

        int index = ui->stackedWidget->currentIndex();
        if (index < d->PAGE_COUNT) {
            index++;
            ui->stackedWidget->setCurrentIndex(index);
        }

        if (index == d->PAGE_COUNT) {
            // We can install the software now, so disable prev/fwd & abort buttons
            enableButton(KDialog::User1, false);
            enableButton(KDialog::Ok, false);
            enableButton(KDialog::Cancel, false);

            // We now show the install dialog, so run the installation now!
            listaller_setup_run_installation(d->liSetup);
        }

    } else if (button == KDialog::User1) {
        // We go back

        int index = ui->stackedWidget->currentIndex();
        if (index > 1) {
            index--;
            ui->stackedWidget->setCurrentIndex(index);
            if (index == 1)
                enableButton(KDialog::User1, false);
        }
    } else if (button == KDialog::Cancel) {
        close();
    } else if (button == KDialog::Close) {
        close();
    }
}

void SetupWizard::licenseAccepted(bool accepted)
{
    enableButton(KDialog::Ok, accepted);
}

bool SetupWizard::initialize()
{
    bool ret;
    ret = listaller_setup_initialize(d->liSetup);
    if (ret) {
        d->appID = listaller_setup_get_current_application(d->liSetup);
    }
    return ret;
}
