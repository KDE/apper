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

#include <listaller.h>
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

#include "InfoWidget.h"
#include "SimplePage.h"
#include "LicensePage.h"

class SetupWizardPrivate
{
public:
    SetupWizardPrivate()
        : setup(NULL)
    {
        preparationPageCount = -1;
    }

    ~SetupWizardPrivate()
    {
        if (setup != NULL)
            g_object_unref (setup);
    }

    int preparationPageCount;

    // Listaller structures
    ListallerSetup *setup;
    ListallerAppItem *appID;
    ListallerIPKControl *metaInfo;

    // UI components
    InfoWidget *infoPage;
    SimplePage *execPage;
    QCheckBox *sharedInstallCb;
    QProgressBar *progressBar;
};

void on_lisetup_message(GObject *sender, ListallerMessageItem *message, SetupWizard *self)
{
    Q_UNUSED(sender);
    Q_UNUSED(self);
    kDebug() << "Listaller message:" << listaller_message_item_get_details(message);
}

void on_lisetup_status_changed(GObject *sender, ListallerStatusItem *status, SetupWizard *self)
{
    Q_UNUSED(sender);
    Q_UNUSED(status);
    SetupWizardPrivate *d = self->getPriv();

    ListallerStatusEnum statusType = listaller_status_item_get_status(status);

    if (statusType == LISTALLER_STATUS_ENUM_INSTALLATION_FINISHED) {
        AsComponent *info = listaller_app_item_get_metainfo (d->appID);
        QString appName = QString::fromUtf8(as_component_get_name (info));
        d->infoPage->reset();
        d->infoPage->setWindowTitle(i18n("Installation finished!"));
        d->infoPage->setDescription(i18n("%1 has been installed successfully!", appName));
        d->infoPage->setIcon(KIcon("dialog-ok-apply"));
        self->setButtons(KDialog::Close);
        self->button(KDialog::Close)->setFocus();
        self->setCurrentPage(d->infoPage);
    }
}

void on_lisetup_error_code(GObject *sender, ListallerErrorItem *error, SetupWizard *self)
{
    Q_UNUSED(sender);
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

void on_lisetup_error_code_simple(GObject *sender, ListallerErrorItem *error, SetupWizard *self)
{
    Q_UNUSED(sender);

    KMessageBox::error(self,
                       QString::fromUtf8(listaller_error_item_get_details(error)),
                       i18n("An error occurred"));
}

void on_lisetup_progress(GObject *sender, ListallerProgressItem *item, SetupWizard *self)
{
    Q_UNUSED(sender);
    SetupWizardPrivate *d = self->getPriv();

    if (listaller_progress_item_get_prog_type (item) != LISTALLER_PROGRESS_ENUM_MAIN_PROGRESS)
        return;

    // TODO: Handle item-progress too

    int value = listaller_progress_item_get_value (item);
    if (value > 0)
        d->progressBar->setValue(value);
}

SetupWizard::SetupWizard(const QString& ipkFName, QWidget *parent)
    : KDialog (parent),
      d(new SetupWizardPrivate()),
      ui(new Ui::SetupWizard),
      m_ipkFName(ipkFName)
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
}

SetupWizard::~SetupWizard()
{
    delete d;
    delete ui;
}

bool SetupWizard::constructWizardLayout()
{
    if (d->preparationPageCount > 1)
        return true;

    if (d->preparationPageCount == 0) {
	ui->stackedWidget->addWidget(d->infoPage);
    }


    if (d->appID == NULL) {
        kDebug() << "AppID was NULL!";
        return false;
    }

    AsComponent *info = listaller_app_item_get_metainfo (d->appID);
    QString appName = as_component_get_name(info);

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
    d->preparationPageCount = 1;

    // Setup details page
    SimplePage *detailsP = new SimplePage(this);
    detailsP->setTitle(i18n("Details"));
    detailsP->setDescription(i18n("Details about this installation"));

    QWidget *securityWidget = new QWidget(detailsP);
    QHBoxLayout *secWgLayout = new QHBoxLayout();
    securityWidget->setLayout(secWgLayout);
    QLabel *pix = new QLabel(detailsP);

    ListallerIPKSecurityInfo *secInfo = listaller_setup_get_security_info(d->setup);
    ListallerSecurityLevel secLev = listaller_ipk_security_info_get_level(secInfo);
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

    QLabel *secInfoL = new QLabel(detailsP);
    secInfoL->setText(QString::fromUtf8(listaller_ipk_security_info_get_level_as_sentence(secInfo)));
    secWgLayout->addWidget(secInfoL);

    QPushButton *infoBtn = new QPushButton (detailsP);
    infoBtn->setIcon(KIcon("dialog-information"));
    infoBtn->setFlat(true);
    infoBtn->setMinimumWidth(16);
    infoBtn->setMinimumHeight(16);
    connect(infoBtn, SIGNAL(clicked()), this, SLOT(securityInfoBtnClicked()));
    secWgLayout->addWidget(infoBtn);

    // Install mode select checkbox
    secWgLayout->addStretch();
    detailsP->addWidget(securityWidget);
    d->sharedInstallCb = new QCheckBox(detailsP);
    d->sharedInstallCb->setText(i18n("Install for all users"));

    // Check if current package allows shared installations (if not disable the checkbox)
    ListallerIPKInstallMode modes = listaller_setup_supported_install_modes (d->setup);
    if (listaller_setup_get_install_mode (d->setup) == LISTALLER_IPK_INSTALL_MODE_SHARED)
        d->sharedInstallCb->setChecked(true);
    if (listaller_ipk_install_mode_is_all_set(modes, LISTALLER_IPK_INSTALL_MODE_PRIVATE))
        d->sharedInstallCb->setEnabled(true);
    else
        d->sharedInstallCb->setEnabled(false);

    connect(d->sharedInstallCb, SIGNAL(toggled(bool)), this, SLOT(sharedInstallCbToggled(bool)));
    detailsP->addWidget(d->sharedInstallCb);
    ui->stackedWidget->addWidget(detailsP);
    d->preparationPageCount++;

    // Application description page
    SimplePage *descP = new SimplePage(this);
    descP->setTitle(i18n("Application description"));
    descP->setDescription(i18n("Description"));
    descP->setDetails(as_component_get_description(info));
    ui->stackedWidget->addWidget(descP);
    d->preparationPageCount++;

    // License page
    // (don't add the license page, if user does not have to accept it (during installation))
    if (listaller_ipk_control_get_user_accept_license(d->metaInfo)) {
        ListallerAppLicense appLicense;
        listaller_app_item_get_license(d->appID, &appLicense);
        LicensePage *licenseP = new LicensePage(this);
        licenseP->setTitle(i18n("Software license"));
        licenseP->setDescription(i18n("Please read the following terms and conditions carefully!"));
        licenseP->setLicenseText(QString::fromUtf8(appLicense.text));
        connect(licenseP, SIGNAL(licenseAccepted(bool)), this, SLOT(licenseAccepted(bool)));
        ui->stackedWidget->addWidget(licenseP);
	d->preparationPageCount++;
    }

    // Setup exec page
    d->execPage = new SimplePage(this);
    d->execPage->setTitle(i18n("Running installation"));
    d->execPage->setDescription(i18n("Installing %1...", appName));
    d->progressBar = new QProgressBar(d->execPage);
    d->progressBar->setMinimumHeight(40);
    d->execPage->addWidget(d->progressBar);
    ui->stackedWidget->addWidget(d->execPage);
    d->preparationPageCount++;

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
    QString pkgStr = QString::fromUtf8(listaller_setup_get_replaced_native_packs (d->setup));
    QStringList pkgs = pkgStr.split('\n');
    if (!pkgStr.isEmpty()) {
        KMessageBox::informationList(this, i18n("Installing this package will make the following native packages obsolete. You might consider removing them manually."),
                                     pkgs, i18n("Similar native packages found"));
    }

    // We now show the install dialog, so run the installation now!
    listaller_setup_run_installation(d->setup);
}


void SetupWizard::slotButtonClicked(int button)
{
    if (button == KDialog::Ok) {
        // We go forward

        int index = ui->stackedWidget->currentIndex();
        if (index < d->preparationPageCount) {
            index++;
            ui->stackedWidget->setCurrentIndex(index);
        }

        if (index == d->preparationPageCount-1) {
            setButtonText(KDialog::Ok, i18n("Install"));
            setButtonIcon(KDialog::Ok, KIcon("dialog-ok-apply"));
        }

        if (index == d->preparationPageCount) {
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
            // Make sure go-forward button is enabled
            enableButton(KDialog::Ok, true);
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

    // Create a new Listaller application setup instance
    d->setup = listaller_setup_new (m_ipkFName.toUtf8());
    // Only connect to simple error handling at first
    uint signal_error;
    signal_error = g_signal_connect (d->setup, "error-code", (GCallback) on_lisetup_error_code_simple, this);

    // Create info page to notify about errors
    d->infoPage = new InfoWidget(this);
    d->infoPage->setWindowTitle("Information");
    ui->stackedWidget->addWidget(d->infoPage);

    // Initialize the setup, required to have an AppItem present
    ret = listaller_setup_initialize(d->setup);
    if (ret) {
        d->appID = listaller_setup_get_current_application(d->setup);
        d->metaInfo = listaller_setup_get_control(d->setup);
    }
    if (!ret)
        return false;

    // Disconnect the simple error handler
    // (we can use the bigger error dialog then, because Setup is initialized and UI constructed)
    g_signal_handler_disconnect (d->setup, signal_error);
    // Now connect all signals
    g_signal_connect (d->setup, "message", (GCallback) on_lisetup_message, this);
    g_signal_connect (d->setup, "status-changed", (GCallback) on_lisetup_status_changed, this);
    g_signal_connect (d->setup, "progress", (GCallback) on_lisetup_progress, this);
    g_signal_connect (d->setup, "error-code", (GCallback) on_lisetup_error_code, this);

    // Build layout of our setup wizard
    constructWizardLayout();

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
    if (shared)
        listaller_setup_set_install_mode (d->setup, LISTALLER_IPK_INSTALL_MODE_SHARED);
    else
        listaller_setup_set_install_mode (d->setup, LISTALLER_IPK_INSTALL_MODE_PRIVATE);
}

void SetupWizard::securityInfoBtnClicked()
{
    // fix all caracters in user-names string to display it
    ListallerIPKSecurityInfo *secInfo = listaller_setup_get_security_info(d->setup);
    QString userNames = QString::fromUtf8(listaller_ipk_security_info_get_user_names(secInfo)).replace ('<', '[').replace('>', ']').replace('\n', "<br>");

    QString infoText = i18n("This package has the following security level: <b>%1</b>", QString::fromUtf8(listaller_ipk_security_info_get_level_as_string(secInfo)));
    infoText += "<br>";
    infoText += i18n("It was signed with a key belonging to these user-ids: %1", "<br>" + userNames);
    infoText += "<br>";
    // TODO: Add the remaining infotmation too

    KMessageBox::information(this, infoText, i18n("Security hints"));
}

void SetupWizard::updatePallete()
{
    QPalette pal;
    pal.setColor(QPalette::Window, KGlobalSettings::activeTitleColor());
    pal.setColor(QPalette::WindowText, KGlobalSettings::activeTextColor());
    ui->backgroundFrame->setPalette(pal);
}
