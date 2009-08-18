/***************************************************************************
 *   Copyright (C) 2009 by Daniel Nicoletti                                *
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
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#include "KpkPackageDetails.h"

#include <KpkStrings.h>

#include <KMessageBox>
#include <QPlainTextEdit>
#include <QTextDocument>

KpkPackageDetails::KpkPackageDetails(PackageKit::Package *package, const Client::Actions &actions, QWidget *parent)
 : QWidget(parent),
   m_package(package),
   currentWidget(0),
   m_gettingOrGotDescription(false),
   m_gettingOrGotFiles(false),
   m_gettingOrGotDepends(false),
   m_gettingOrGotRequires(false)
{
    setupUi(this);

    // we check to see if the actions are supported by the backend
    // if so we ask for information and create the containers
    if (actions & Client::ActionGetDetails) {
        descriptionKTB = new KTextBrowser(this);
        descriptionTB->click();

    } else {
        descriptionTB->setEnabled(false);
    }

    if (actions & Client::ActionGetFiles) {
        filesPTE = new QPlainTextEdit(this);
        filesPTE->setVisible(false);
        if (!currentWidget) {
            fileListTB->click();
        }
    } else {
        fileListTB->setEnabled(false);
    }

    if (actions & Client::ActionGetDepends) {
        dependsOnLV = new QListView(this);
        dependsOnLV->setVisible(false);
        dependsOnLV->setModel(m_pkg_model_dep = new KpkSimplePackageModel(this));
        if (!currentWidget) {
            dependsOnTB->click();
        }
    } else {
        dependsOnTB->setEnabled(false);
    }

    if (actions & Client::ActionGetRequires) {
        requiredByLV = new QListView(this);
        requiredByLV->setVisible(false);
        requiredByLV->setModel(m_pkg_model_req = new KpkSimplePackageModel(this));
        if (!currentWidget) {
            requiredByTB->click();
        }
    } else {
        requiredByTB->setEnabled(false);
    }
}

KpkPackageDetails::~KpkPackageDetails()
{
//     qDebug() << "~KpkPackageDetails()";
}

void KpkPackageDetails::setCurrentWidget(QWidget *widget)
{
    // if a current widget is set remove it
    if (currentWidget) {
        gridLayout->removeWidget(currentWidget);
        // hides the current widget
        currentWidget->setVisible(false);
    }
    // make sure the widget is visible
    widget->setVisible(true);
    // add the widget to the layout
    gridLayout->addWidget(currentWidget = widget);
}

void KpkPackageDetails::getDetails(PackageKit::Package *p)
{
    // create the description transaction
    Transaction *t = Client::instance()->getDetails(p);
    if (t->error()) {
        KMessageBox::error(this, KpkStrings::daemonError(t->error()));
    } else {
        connect(t, SIGNAL(details(PackageKit::Package *)),
                this, SLOT(description(PackageKit::Package *)));
        connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                this, SLOT(getDetailsFinished(PackageKit::Transaction::ExitStatus, uint)));
    }
}

void KpkPackageDetails::description(PackageKit::Package *p)
{
    descriptionKTB->clear();
    //format and show description
    Package::Details *details = p->details();
    QString description;
    description += "<table><tbody>";
    description += "<tr><td align=\"right\"><b>" + i18n("Package Name") + ":</b></td><td>" + p->name() + "</td></tr>";
    if (!details->license().isEmpty())
        description += "<tr><td align=\"right\"><b>" + i18n("License")
                    + ":</b></td><td>" + details->license()
                    + "</td></tr>";
    if (details->group() != Client::UnknownGroup)
        description += "<tr><td align=\"right\"><b>" + i18nc("Group of the package", "Group") + ":</b></td><td>"
                    + KpkStrings::groups(details->group())
                    + "</td></tr>";
    if (!details->description().isEmpty())
        description += "<tr><td align=\"right\"><b>" + i18n("Details")
                    + ":</b></td><td>" + details->description().replace('\n', "<br />")
                    + "</td></tr>";
    if (!details->url().isEmpty())
        description += "<tr><td align=\"right\"><b>" + i18n("Home Page")
                    + ":</b></td><td><a href=\"" + details->url() + "\">" + details->url()
                    + "</a></td></tr>";
    if (details->size() > 0)
        description += "<tr><td align=\"right\"><b>" + i18n("Size")
                    + ":</b></td><td>" + KGlobal::locale()->formatByteSize(details->size())
                    + "</td></tr>";
    description += "</table></tbody>";
    descriptionKTB->setHtml(description);
}

void KpkPackageDetails::getDetailsFinished(PackageKit::Transaction::ExitStatus status, uint runtime)
{
    Q_UNUSED(runtime)
    if (status != Transaction::ExitSuccess) {
        m_gettingOrGotDescription = false;
    }
}

void KpkPackageDetails::on_descriptionTB_clicked()
{
    setCurrentWidget(descriptionKTB);
    if (!m_gettingOrGotDescription) {
        getDetails(m_package);
        m_gettingOrGotDescription = true;
    }
}

void KpkPackageDetails::getFiles(PackageKit::Package *p)
{
    // create the files transaction
    Transaction *t = Client::instance()->getFiles(p);
    if (t->error()) {
        KMessageBox::error(this, KpkStrings::daemonError(t->error()));
    } else {
        connect(t, SIGNAL(files(PackageKit::Package *, const QStringList &)),
                this, SLOT(files(PackageKit::Package *, const QStringList &)));
        connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                this, SLOT(getFilesFinished(PackageKit::Transaction::ExitStatus, uint)));
    }
}

void KpkPackageDetails::files(PackageKit::Package *package, const QStringList &files)
{
    Q_UNUSED(package)
    filesPTE->clear();
    for (int i = 0; i < files.size(); ++i) {
        filesPTE->appendPlainText(files.at(i));
    }
}

void KpkPackageDetails::getFilesFinished(PackageKit::Transaction::ExitStatus status, uint runtime)
{
    Q_UNUSED(runtime)
    if (status == Transaction::ExitSuccess) {
        if (filesPTE->document()->toPlainText().isEmpty()) {
            filesPTE->appendPlainText(i18n("No files were found."));
        }
    } else {
        m_gettingOrGotFiles = false;
    }
}

void KpkPackageDetails::on_fileListTB_clicked()
{
    setCurrentWidget(filesPTE);
    if (!m_gettingOrGotFiles) {
        getFiles(m_package);
        m_gettingOrGotFiles = true;
    }
}

void KpkPackageDetails::getDepends(PackageKit::Package *p)
{
    // create a transaction for the dependecies not recursive
    Transaction *t = Client::instance()->getDepends(p, PackageKit::Client::NoFilter, false);
    m_pkg_model_dep->clear();
    if (t->error()) {
        KMessageBox::error(this, KpkStrings::daemonError(t->error()));
    } else {
        connect(t, SIGNAL(package(PackageKit::Package *)),
                m_pkg_model_dep, SLOT(addPackage(PackageKit::Package *)));
        connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                this, SLOT(getDependsFinished(PackageKit::Transaction::ExitStatus, uint)));
    }
}

void KpkPackageDetails::getDependsFinished(PackageKit::Transaction::ExitStatus status, uint runtime)
{
    Q_UNUSED(runtime)
    if (status != Transaction::ExitSuccess) {
        m_gettingOrGotDepends = false;
    }
}

void KpkPackageDetails::on_dependsOnTB_clicked()
{
    setCurrentWidget(dependsOnLV);
    if (!m_gettingOrGotDepends) {
        getDepends(m_package);
        m_gettingOrGotDepends = true;
    }
}

void KpkPackageDetails::getRequires(PackageKit::Package *p)
{
    // create a transaction for the requirements not recursive
    Transaction *t = Client::instance()->getRequires(p, PackageKit::Client::NoFilter, false);
    m_pkg_model_req->clear();
    if (t->error()) {
        KMessageBox::error(this, KpkStrings::daemonError(t->error()));
    } else {
        connect(t, SIGNAL(package(PackageKit::Package *)),
                m_pkg_model_req, SLOT(addPackage(PackageKit::Package *)));
        connect(t, SIGNAL(finished(PackageKit::Transaction::ExitStatus, uint)),
                this, SLOT(getRequiresFinished(PackageKit::Transaction::ExitStatus, uint)));
    }
}

void KpkPackageDetails::getRequiresFinished(PackageKit::Transaction::ExitStatus status, uint runtime)
{
    Q_UNUSED(runtime)
    if (status != Transaction::ExitSuccess) {
        m_gettingOrGotRequires = false;
    }
}

void KpkPackageDetails::on_requiredByTB_clicked()
{
    setCurrentWidget(requiredByLV);
    if (!m_gettingOrGotRequires) {
        getRequires(m_package);
        m_gettingOrGotRequires = true;
    }
}

#include "KpkPackageDetails.moc"
