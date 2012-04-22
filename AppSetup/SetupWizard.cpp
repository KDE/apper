/* This file is part of Apper
 *
 * Copyright (C) 2012 Matthias Klumpp <matthias@tenstral.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "SetupWizard.h"
#include "ui_SetupWizard.h"

#include <QLabel>
#include <QApplication>
#include <QAction>
#include <QProgressBar>
#include <QCheckBox>
#include <KPushButton>
#include <KMenuBar>
#include <KDebug>
#include <KMessageBox>
#include <KGlobalSettings>
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
        // Create a new Listaller settings provider
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

    // Listaller structures
    ListallerSettings *liConf;
    ListallerSetup *liSetup;
    ListallerAppItem *appID;
    ListallerIPKPackSecurity *packSecurity;

    // UI components
    InfoWidget *infoPage;
    SimplePage *execPage;
    QCheckBox *sharedInstallCb;
    QProgressBar *mainProgressBar;
    QProgressBar *subProgressBar;
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
        QString appName = QString::fromUtf8(listaller_app_item_get_full_name(d->appID));
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
    d->infoPage->setDescription(i18n("An error occurred"));
    d->infoPage->setIcon(KIcon("dialog-error"));
    d->infoPage->setDetails(QString::fromUtf8(listaller_error_item_get_details(error)));
    self->setButtons(KDialog::Close);
    self->button(KDialog::Close)->setFocus();
    self->setCurrentPage(d->infoPage);
}

void on_lisetup_progress_changed (GObject *sender, int progress, int subProgress, SetupWizard *self)
{
    Q_UNUSED(sender);
    SetupWizardPrivate *d = self->getPriv();

    d->mainProgressBar->setValue(progress);
    d->subProgressBar->setValue(subProgress);
}

SetupWizard::SetupWizard(const QString& ipkFName, QWidget *parent)
    : KDialog (parent),
      d(new SetupWizardPrivate()),
      ui(new Ui::SetupWizard)
{
    ui->setupUi(KDialog::mainWidget());
    setAttribute(Qt::WA_DeleteOnClose);

    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
            this, SLOT(updatePallete()));
    updatePallete();

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

    // Create info page to notify about errors
    d->infoPage = new InfoWidget(this);
    d->infoPage->setWindowTitle("Information");
    ui->stackedWidget->addWidget(d->infoPage);

    // Initialize the setup, required to have an AppItem present
    bool ret = initialize();

    // Build layout of our setup wizard
    if (ret)
	constructWizardLayout();
}

SetupWizard::~SetupWizard()
{
    delete d;
    delete ui;
}

bool SetupWizard::constructWizardLayout()
{
    if (d->PAGE_COUNT > 1)
        return true;

    if (d->PAGE_COUNT == 0)
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

    QWidget *securityWidget = new QWidget(detailsP);
    QHBoxLayout *secWgLayout = new QHBoxLayout();
    securityWidget->setLayout(secWgLayout);
    QLabel *pix = new QLabel(detailsP);

    ListallerSecurityLevel secLev = listaller_ipk_pack_security_get_level(d->packSecurity);
    if (secLev == LISTALLER_SECURITY_LEVEL_HIGH)
        pix->setPixmap(KIcon("security-high").pixmap (32, 32));
    else if (secLev == LISTALLER_SECURITY_LEVEL_MEDIUM)
        pix->setPixmap(KIcon("security-medium").pixmap (32, 32));
    else if (secLev == LISTALLER_SECURITY_LEVEL_LOW)
        pix->setPixmap(KIcon("security-low").pixmap (32, 32));
    else if (secLev == LISTALLER_SECURITY_LEVEL_DANGEROUS)
        pix->setPixmap(KIcon("security-low").pixmap (32, 32));
    else
	pix->setPixmap(KIcon("task-reject").pixmap (32, 32));
    secWgLayout->addWidget(pix);

    QLabel *secInfo = new QLabel(detailsP);
    secInfo->setText(listaller_security_level_to_string(secLev));
    secWgLayout->addWidget(secInfo);

    secWgLayout->addStretch();
    detailsP->addWidget(securityWidget);
    d->sharedInstallCb = new QCheckBox(detailsP);
    d->sharedInstallCb->setText(i18n("Install for all users (as root)"));
    connect(d->sharedInstallCb, SIGNAL(toggled(bool)), this, SLOT(sharedInstallCbToggled(bool)));
    detailsP->addWidget(d->sharedInstallCb);
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
    licenseP->setLicenseText(QString::fromUtf8(appLicense.text));
    connect(licenseP, SIGNAL(licenseAccepted(bool)), this, SLOT(licenseAccepted(bool)));
    ui->stackedWidget->addWidget(licenseP);

    // Setup exec page
    d->execPage = new SimplePage(this);
    d->execPage->setTitle(i18n("Running installation"));
    d->execPage->setDescription(i18n("Installing %1...", appName));
    d->mainProgressBar = new QProgressBar(d->execPage);
    d->mainProgressBar->setMinimumHeight(40);
    d->subProgressBar = new QProgressBar(d->execPage);
    d->execPage->addWidget(d->mainProgressBar);
    d->execPage->addWidget(d->subProgressBar);
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

void SetupWizard::runInstallation()
{
    //
    QString pkgStr = QString::fromUtf8(listaller_setup_get_replaced_native_packs (d->liSetup));
    QStringList pkgs = pkgStr.split('\n');
    if (!pkgStr.isEmpty()) {
        KMessageBox::informationList(this, i18n("Installing this package will make the following native packages obsolete. You might consider removing them manually."),
                                     pkgs, i18n("Similar native packages found"));
    }

    // We now show the install dialog, so run the installation now!
    listaller_setup_run_installation(d->liSetup);
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

	    ui->stackedWidget->update();
	    runInstallation();
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

bool SetupWizard::initialize()
{
    bool ret;
    ret = listaller_setup_initialize(d->liSetup);
    if (ret) {
        d->appID = listaller_setup_get_current_application(d->liSetup);
        d->packSecurity = listaller_setup_get_security_info(d->liSetup);
    }

    return ret;
}

void SetupWizard::licenseAccepted(bool accepted)
{
    enableButton(KDialog::Ok, accepted);
}

void SetupWizard::sharedInstallCbToggled(bool shared)
{
    if (shared) {
        int btnCode = KMessageBox::warningYesNo(this, i18n("You want to install this application as root!\n"
                                                "This can damage your system or install malicious software for all users.\n"
                                                "Please only proceed, if you really know what you're doing!\n"
                                                "Continue?"), i18n("Really install as superuser?"));
        if (btnCode == KMessageBox::No) {
            d->sharedInstallCb->setChecked(false);
            return;
        }
    }
    listaller_settings_set_sumode(d->liConf, shared);
}

void SetupWizard::updatePallete()
{
    QPalette pal;
    pal.setColor(QPalette::Window, KGlobalSettings::activeTitleColor());
    pal.setColor(QPalette::WindowText, KGlobalSettings::activeTextColor());
    ui->backgroundFrame->setPalette(pal);
}
