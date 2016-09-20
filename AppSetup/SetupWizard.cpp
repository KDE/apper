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

#include <limba.h>
#include <appstream.h>

#include "SetupWizard.h"
#include "ui_SetupWizard.h"

#include <QLabel>
#include <QApplication>
#include <QAction>
#include <QProgressBar>
#include <QCheckBox>
#include <QPushButton>
#include <KDebug>
#include <KMessageBox>
#include <KGlobalSettings>
#include <KLocalizedString>

#include "InfoWidget.h"
#include "SimplePage.h"

class SetupWizardPrivate
{
public:
    SetupWizardPrivate()
        : setup(NULL), cpt(NULL)
    {
        preparationPageCount = -1;
    }

    ~SetupWizardPrivate()
    {
        if (setup != NULL)
            g_object_unref (setup);
        if (cpt != NULL)
            g_object_unref (cpt);
    }

    int preparationPageCount;

    // Package information
    LiInstaller *setup;
    AsComponent *cpt;
    LiPkgInfo *pki;

    // UI components
    InfoWidget *infoPage;
    SimplePage *execPage;
    QProgressBar *progressBar;
};

SetupWizard::SetupWizard(QWidget *parent)
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
}

SetupWizard::~SetupWizard()
{
    delete d;
    delete ui;
}

void SetupWizard::showError(const QString& details)
{
    d->infoPage->reset();
    d->infoPage->setWindowTitle(i18n("Error"));
    d->infoPage->setDescription(i18n("An error occurred"));
    d->infoPage->setIcon(KIcon("dialog-error"));
    d->infoPage->setDetails(details);
    this->setButtons(KDialog::Close);
    this->button(KDialog::Close)->setFocus();
    this->setCurrentPage(d->infoPage);
}

bool SetupWizard::constructWizardLayout()
{
    if (d->preparationPageCount > 1)
        return true;

    if (d->preparationPageCount == 0) {
    ui->stackedWidget->addWidget(d->infoPage);
    }


    if (d->cpt == NULL) {
        kDebug() << "AppStream component was NULL!";
        return false;
    }

    QString appName = as_component_get_name(d->cpt);

    // Welcome page
    SimplePage *introP = new SimplePage(this);
    introP->setTitle(i18n("Welcome!"));
    introP->setDescription(i18n("Welcome to the installation of %1!", appName));
    introP->setDetails(i18n("Please be careful while installing a 3rd-party application.<br>"
                            "It might eventually <b>damage</b> your system or "
                            "have <b>malicious behaviour</b>, like spying out your passwords.<br>"
                            "Please do only install this software if you <b>trust its "
                            "publisher</b>."));
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

    LiTrustLevel trust = li_installer_get_package_trust_level(d->setup, NULL);
    if (trust == LI_TRUST_LEVEL_HIGH)
        pix->setPixmap(KIcon("security-high").pixmap (32, 32));
    else if ((trust == LI_TRUST_LEVEL_LOW) || (trust == LI_TRUST_LEVEL_MEDIUM))
        pix->setPixmap(KIcon("security-medium").pixmap (32, 32));
    else if ((trust == LI_TRUST_LEVEL_NONE) || (trust == LI_TRUST_LEVEL_INVALID))
        pix->setPixmap(KIcon("security-low").pixmap (32, 32));
    else
        pix->setPixmap(KIcon("task-reject").pixmap (32, 32));
    secWgLayout->addWidget(pix);

    QLabel *secInfoL = new QLabel(detailsP);
    secInfoL->setText(QString::fromUtf8(li_trust_level_to_text(trust)));
    secWgLayout->addWidget(secInfoL);

    // Install mode select checkbox
    secWgLayout->addStretch();
    detailsP->addWidget(securityWidget);

    ui->stackedWidget->addWidget(detailsP);
    d->preparationPageCount++;

    // Application description page
    QString desc = as_component_get_description(d->cpt);
    if (desc.isEmpty())
        desc = as_component_get_summary(d->cpt);
    SimplePage *descP = new SimplePage(this);
    descP->setTitle(i18n("Application description"));
    descP->setDescription(i18n("Description"));
    descP->setDetails(desc);
    ui->stackedWidget->addWidget(descP);
    d->preparationPageCount++;

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
    GError *error = NULL;

    // We now show the install dialog, so run the installation now!
    li_installer_install(d->setup, &error);

    if (error == NULL) {
        /* we installed successfully */
        QString appName = QString::fromUtf8(as_component_get_name (d->cpt));
        d->infoPage->reset();
        d->infoPage->setWindowTitle(i18n("Installation finished!"));
        d->infoPage->setDescription(i18n("%1 has been installed successfully!", appName));
        d->infoPage->setIcon(KIcon("dialog-ok-apply"));
        setButtons(KDialog::Close);
        button(KDialog::Close)->setFocus();
        setCurrentPage(d->infoPage);
    } else {
        /* there was an error */
        showError(QString::fromUtf8(error->message));
        g_error_free (error);
    }
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

bool SetupWizard::initialize(const QString& ipkFName)
{
    GError *error = NULL;
    AsMetadata *mdata;
    gchar *data;

    // Create a new installer instance
    d->setup = li_installer_new ();

    // Create info page to notify about errors
    d->infoPage = new InfoWidget(this);
    d->infoPage->setWindowTitle("Information");
    ui->stackedWidget->addWidget(d->infoPage);

    // Load the package file, required to have metadata
    li_installer_open_file(d->setup, ipkFName.toUtf8().data(), &error);
    if (error != NULL) {
        this->showError(QString::fromUtf8(error->message));
        g_error_free (error);
        return false;
    }

    data = li_installer_get_appstream_data (d->setup);
    mdata = as_metadata_new ();
    as_metadata_parse (mdata, data, AS_FORMAT_KIND_XML, &error);
    g_free (data);
    if (error != NULL) {
        this->showError(QString::fromUtf8(error->message));
        g_error_free (error);
        g_object_unref (mdata);
        return false;
    }

    d->cpt = as_metadata_get_component (mdata);
    g_object_ref (d->cpt);
    d->pki = li_installer_get_package_info (d->setup);
    g_object_unref (mdata);

    // Build layout of our setup wizard
    constructWizardLayout();

    return true;
}

void SetupWizard::updatePallete()
{
    QPalette pal;
    pal.setColor(QPalette::Window, KGlobalSettings::activeTitleColor());
    pal.setColor(QPalette::WindowText, KGlobalSettings::activeTextColor());
    ui->backgroundFrame->setPalette(pal);
}
