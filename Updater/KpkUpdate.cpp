/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "KpkUpdate.h"
#include <KpkStrings.h>

#include <KDebug>
#include <KMessageBox>
#include <KProgressDialog>
#include <KColorScheme>
#include <solid/powermanagement.h>
#include <solid/device.h>
#include <solid/acadapter.h>
#include <KpkTransactionBar.h>

#define UNIVERSAL_PADDING 6

KpkUpdate::KpkUpdate( QWidget *parent )
    : QWidget( parent ),
    m_distroUpgradeProcess(0),
    m_distroUpgradeDialog(0)
{
    setupUi( this );
    detailsDW->hide();
    
    updatePB->setIcon(KIcon("package-update"));
    refreshPB->setIcon(KIcon("view-refresh"));
    historyPB->setIcon(KIcon("view-history"));
    transactionBar->setBehaviors(KpkTransactionBar::AutoHide);

    Client::instance()->setLocale(KGlobal::locale()->language() + "." + KGlobal::locale()->encoding());

    //initialize the model, delegate, client and  connect it's signals
    packageView->setItemDelegate(pkg_delegate = new KpkDelegate(this));
    packageView->setModel(m_pkg_model_updates = new KpkPackageModel(this, packageView));
    m_pkg_model_updates->setGrouped(true);
    connect(m_pkg_model_updates, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)),
            this, SLOT(checkEnableUpdateButton()));

    // Create a new client
    m_client = Client::instance();

    // check to see what roles the backend
    m_actions = m_client->getActions();
    
    // Setup the distro upgrade banner
    //TODO: Find the distribution's logo
    distroTitle->setPixmap(KIcon("system-software-update"));
    /*QPalette titleColors(distroTitle->palette());
    //FIXME: This is a bug in kdelibs. The background color doesn't get changed.
    KColorScheme::adjustBackground(titleColors, KColorScheme::PositiveBackground);
    distroTitle->setPalette(titleColors);*/
    distroTitle->hide();
    distroDescription->hide();
    distroUpgradeBtn->hide();
    
    connect(distroUpgradeBtn, SIGNAL(clicked(bool)), this, SLOT(startDistroUpgrade()));
    
    
}

void KpkUpdate::startDistroUpgrade()
{
    QList<Solid::Device> powerPlugs = Solid::Device::listFromType(Solid::DeviceInterface::AcAdapter);
    bool pluggedIn = true;
    bool hasBattery = Solid::Device::listFromType(Solid::DeviceInterface::Battery).size()>0;
    foreach(const Solid::Device dev, powerPlugs)
        if (!dev.as<Solid::AcAdapter>()->isPlugged())
            pluggedIn = false;
    
    QString warning = i18n("You are about to upgrade your distribution to the latest version. "
    "This is usually a very lengthy process and takes a lot longer than "
    "simply upgrading your packages.");
    
    if (!pluggedIn)
        warning+=" "+i18n("It is recommended to plug in your computer before proceeding.");
    else if (hasBattery)
        warning+=" "+i18n("It is recommended that you keep your computer plugged in while the upgrade is being performed.");
    
    if (KMessageBox::warningContinueCancel(this,warning) == KMessageBox::Continue) {
        m_distroUpgradeProcess = new QProcess;
        connect (m_distroUpgradeProcess, SIGNAL (error ( QProcess::ProcessError )),
            this, SLOT(distroUpgradeError( QProcess::ProcessError  ) ));
        connect (m_distroUpgradeProcess, SIGNAL (finished(int, QProcess::ExitStatus)),
            this, SLOT(distroUpgradeFinished(int, QProcess::ExitStatus  ) ));

        
        m_distroUpgradeDialog = new KProgressDialog(this);
        m_distroUpgradeDialog->setLabelText("Waiting for distribution upgrade to complete");
        m_distroUpgradeDialog->showCancelButton(false);
        m_distroUpgradeDialog->setModal(true);
        m_distroUpgradeDialog->progressBar()->setMaximum(0); //Makes it a busy indicator
        m_distroUpgradeDialog->progressBar()->setMinimum(0);
        m_distroUpgradeDialog->show();
        m_distroUpgradeProcess->start("/usr/share/PackageKit/pk-upgrade-distro.sh");
        suppressSleep(true);
    }
}

void KpkUpdate::distroUpgradeError(QProcess::ProcessError error)
{
    QString text;
    switch(error) {
        case QProcess::FailedToStart:
            KMessageBox::error(this, i18n("The distribution upgrade process failed to start."));
            break;
        case QProcess::Crashed:
            KMessageBox::error(this, i18n("The distribution upgrade process crashed some time after starting successfully."));
            break;
        default:
            KMessageBox::error(this, i18n("The distribution upgrade process failed with an unknown error."));
            break;
    }
}

void KpkUpdate::distroUpgradeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if ( exitStatus == QProcess::NormalExit && exitCode == 0 ) {
        KMessageBox::information(this, i18n("Distribution upgrade complete."));
    } else if ( exitStatus == QProcess::NormalExit ) {
        KMessageBox::error(this, i18n("Distribution upgrade process exited with code %1.", exitCode));
    }
    m_distroUpgradeProcess->deleteLater();
    m_distroUpgradeProcess = 0;
    m_distroUpgradeDialog->close();
    m_distroUpgradeDialog->deleteLater();
    m_distroUpgradeDialog = 0;
    suppressSleep(false);
}

//TODO: We should add some kind of configuration to let users show unstable distributions
//That way, by default, users only see stable ones.
void KpkUpdate::distroUpgrade(PackageKit::Client::UpgradeType type, const QString& name, const QString& description)
{
    Q_UNUSED(type)
    distroDescription->setText(description);
    distroUpgradeBtn->setText(i18n("Upgrade to %1", name));
    distroTitle->show();
    distroDescription->show();
    distroUpgradeBtn->show();
}

void KpkUpdate::checkEnableUpdateButton()
{
    if (m_pkg_model_updates->selectedPackages().size() > 0) {
        emit changed(true);
    } else {
        emit changed(false);
    }
}

void KpkUpdate::on_updatePB_clicked()
{
    m_pkg_model_updates->checkAll();
    applyUpdates();
}

void KpkUpdate::load()
{
    displayUpdates(KpkTransaction::Success);
}

void KpkUpdate::updateFinished(KpkTransaction::ExitStatus status)
{
    Q_UNUSED(status)
    suppressSleep(false);
}

void KpkUpdate::applyUpdates()
{
    QList<Package*> packages = m_pkg_model_updates->selectedPackages();
    //check to see if the user selected all selectable packages
    if (m_pkg_model_updates->allSelected()) {
        // if so let's do system-update instead
        if ( Transaction *t = m_client->updateSystem() ) {
            KpkTransaction *frm = new KpkTransaction(t, KpkTransaction::CloseOnFinish, this);
            suppressSleep(true);
            connect(frm, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
                     this, SLOT(updateFinished(KpkTransaction::ExitStatus)));
            frm->exec();
        } else {
            KMessageBox::error( this, i18n("Authentication failed"), i18n("KPackageKit") );
        }
    } else {
        // else lets install only the selected ones
        if ( Transaction *t = m_client->updatePackages(packages) ) {
            KpkTransaction *frm = new KpkTransaction(t, KpkTransaction::CloseOnFinish, this);
            suppressSleep(true);
            connect(frm, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
                     this, SLOT(updateFinished(KpkTransaction::ExitStatus)));
            frm->exec();
        } else {
            KMessageBox::error(this, i18n("Authentication failed"), i18n("KPackageKit"));
        }
    }
    load();
}

void KpkUpdate::suppressSleep(bool enable)
{
    if ( enable ) {
        kDebug() << "Disabling powermanagement sleep";
        m_inhibitCookie = Solid::PowerManagement::beginSuppressingSleep( i18n("Installing updates.") );
        if (m_inhibitCookie == -1)
            kDebug() << "Sleep suppression denied!";
    } else {
        kDebug() << "Enable powermanagement sleep";
        if (m_inhibitCookie == -1)
            if ( ! Solid::PowerManagement::stopSuppressingSleep( m_inhibitCookie ))
                kDebug() << "Enable failed: invalid cookie.";
    }
}

void KpkUpdate::refresh()
{
    if ( Transaction *t = m_client->refreshCache(true) ) {
        KpkTransaction *frm = new KpkTransaction(t, KpkTransaction::Modal | KpkTransaction::CloseOnFinish, this);
        connect(frm, SIGNAL(kTransactionFinished(KpkTransaction::ExitStatus)),
                 this, SLOT(displayUpdates(KpkTransaction::ExitStatus)));
        frm->exec();
    } else {
        KMessageBox::error( this, i18n("Authentication failed"), i18n("KPackageKit") );
    }
}

void KpkUpdate::displayUpdates(KpkTransaction::ExitStatus status)
{
    checkEnableUpdateButton();
    if (status == KpkTransaction::Success) {
        m_pkg_model_updates->clear();
        m_pkg_model_updates->uncheckAll();
        m_updatesT = m_client->getUpdates();
        transactionBar->addTransaction(m_updatesT);
        connect(m_updatesT, SIGNAL(package(PackageKit::Package *)),
                m_pkg_model_updates, SLOT(addPackage(PackageKit::Package *)));
        connect(m_updatesT, SIGNAL(errorCode(PackageKit::Client::ErrorType, const QString &)),
                this, SLOT(errorCode(PackageKit::Client::ErrorType, const QString &)));
        Transaction* t = m_client->getDistroUpgrades();
        transactionBar->addTransaction(t);
        connect(t,
                 SIGNAL( distroUpgrade( PackageKit::Client::UpgradeType, const QString&, const QString& ) ),
             this, SLOT( distroUpgrade(PackageKit::Client::UpgradeType, const QString&, const QString& ) ) );
    }
}

void KpkUpdate::on_refreshPB_clicked()
{
    refresh();
}

void KpkUpdate::on_packageView_pressed(const QModelIndex &index)
{
    if (index.column() == 0) {
        Package *p = m_pkg_model_updates->package(index);
        // check to see if the backend support
        if (p && m_actions.contains(Client::ActionGetUpdateDetail)) {
            Transaction *t = m_client->getUpdateDetail(p);
            connect(t, SIGNAL(updateDetail(PackageKit::Client::UpdateInfo)),
                    this, SLOT(updateDetail(PackageKit::Client::UpdateInfo)));
        }
    }
}

void KpkUpdate::updateDetail(PackageKit::Client::UpdateInfo info)
{
    //format and show description
    QString description;
    description += "<table><tbody>";
    description += "<tr><td align=\"right\"><b>" + i18n("New version") + ":</b></td><td>" + info.package->name()
                + "-" + info.package->version()
                + "</td></tr>";

    if ( info.updates.size() ) {
        QStringList updates;
        foreach (Package *p, info.updates) updates << p->name() + "-" + p->version();
        description += "<tr><td align=\"right\"><b>" + i18n("Updates") + ":</b></td><td>"
                    + updates.join(", ")
                    + "</td></tr>";
    }
    if ( info.obsoletes.size() ) {
        QStringList obsoletes;
        foreach (Package *p, info.obsoletes) obsoletes << p->id() + "-" + p->version();
        description += "<tr><td align=\"right\"><b>" + i18n("Obsoletes") + ":</b></td><td>"
                    + obsoletes.join(", ")
                    + "</td></tr>";
    }
    if ( !info.updateText.isEmpty() )
        description += "<tr><td align=\"right\"><b>" + i18n("Details") + ":</b></td><td>"
                    + info.updateText.replace('\n', "<br />")
                    + "</td></tr>";
    if ( !info.vendorUrl.isEmpty() )
        description += "<tr><td align=\"right\"><b>" + i18n("Vendor Home Page")
                    + ":</b></td><td><a href=\"" + info.vendorUrl.section(';', 0, 0) + "\">"
                    + info.vendorUrl.section(';', -1)
                    + "</a></td></tr>";
    if ( !info.bugzillaUrl.isEmpty() )
        description += "<tr><td align=\"right\"><b>" + i18n("Bugzilla Home Page")
                    + ":</b></td><td><a href=\"" + info.bugzillaUrl.section(';', 0, 0) + "\">"
                    + info.bugzillaUrl.section(';', -1)
                    + "</a></td></tr>";
    if ( !info.cveUrl.isEmpty() )
        description += "<tr><td align=\"right\"><b>" + i18n("CVE Home Page")
                    + ":</b></td><td><a href=\"" + info.cveUrl.section(';', 0, 0) + "\">"
                    + info.cveUrl.section(';', -1)
                    + "</a></td></tr>";
    if ( !info.changelog.isEmpty() )
        description += "<tr><td align=\"right\"><b>" + i18n("Change Log") + ":</b></td><td>"
                    + info.changelog.replace('\n', "<br />")
                    + "</td></tr>";
    if ( info.state != Client::UnknownUpgradeType)
        description += "<tr><td align=\"right\"><b>" + i18n("State") + ":</b></td><td>"
                    + KpkStrings::updateState(info.state)
                    + "</td></tr>";
    if ( info.restart != Client::UnknownRestartType)
        description += "<tr><td align=\"right\"><b>" + i18n("Restart") + ":</b></td><td>"
                    + KpkStrings::restartTypeFuture(info.restart)
                    + "</td></tr>";
    if ( !info.issued.toString().isEmpty() )
        description += "<tr><td align=\"right\"><b>" + i18n("Issued") + ":</b></td><td>"
                    + KGlobal::locale()->formatDate(info.issued.date(), KLocale::ShortDate)
                    + "</td></tr>";
    if ( !info.updated.toString().isEmpty() )
        description += "<tr><td align=\"right\"><b>" + i18n("Updated") + ":</b></td><td>"
                    + KGlobal::locale()->formatDate(info.updated.date(), KLocale::ShortDate)
                    + "</td></tr>";
    description += "</table></tbody>";
    descriptionKTB->setHtml(description);
    detailsDW->show();
}

void KpkUpdate::on_historyPB_clicked()
{
//     QStringList history;
    QString text;
//     history << KGlobal::locale()->formatDuration( m_daemon->getTimeSinceAction(Role::Refresh_cache) * 1000);
    text.append("Time since last cache refresh: " + KGlobal::locale()->formatDuration( m_client->getTimeSinceAction(Client::ActionRefreshCache) * 1000) );
// // TODO improve this to show more info
// //     text.append("transactions:");
    KMessageBox::information( this, text, i18n("History") );
// //     KMessageBox::informationList( this, text, history, i18n("History") );
}

void KpkUpdate::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateColumnsWidth();
}

bool KpkUpdate::event(QEvent *event)
{
    switch (event->type()) {
        case QEvent::Paint:
        case QEvent::PolishRequest:
        case QEvent::Polish:
            updateColumnsWidth(true);
            break;
        default:
            break;
    }

    return QWidget::event(event);
}

void KpkUpdate::errorCode(PackageKit::Client::ErrorType error, const QString &details)
{
    KMessageBox::detailedSorry( this, KpkStrings::errorMessage(error), details, KpkStrings::error(error), KMessageBox::Notify );
}

void KpkUpdate::updateColumnsWidth(bool force)
{
    int m_viewWidth = packageView->viewport()->width();

    if (force) {
        m_viewWidth -= style()->pixelMetric(QStyle::PM_ScrollBarExtent) + UNIVERSAL_PADDING;
    }

    packageView->setColumnWidth(0, pkg_delegate->columnWidth(0, m_viewWidth));
    packageView->setColumnWidth(1, pkg_delegate->columnWidth(1, m_viewWidth));
}

#include "KpkUpdate.moc"
