/***************************************************************************
 *   Copyright (C) 2009-2010 by Daniel Nicoletti                           *
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

#include <KpkSimplePackageModel.h>
#include <KpkStrings.h>

#include <KMessageBox>
#include <QPlainTextEdit>
#include <QTextDocument>

KpkPackageDetails::KpkPackageDetails(QSharedPointer<PackageKit::Package> package, const Enum::Roles &roles, QWidget *parent)
 : QWidget(parent),
   m_package(package),
   currentWidget(0),
   m_gettingOrGotDescription(false),
   m_gettingOrGotFiles(false),
   m_gettingOrGotDepends(false),
   m_gettingOrGotRequires(false)
{
    setupUi(this);

    // we check to see which roles are supported by the backend
    // if so we ask for information and create the containers
    if (roles & Enum::RoleGetDetails) {
        descriptionKTB = new KTextBrowser(this);
        descriptionTB->click();

    } else {
        descriptionTB->setEnabled(false);
    }

    if (roles & Enum::RoleGetFiles) {
        filesPTE = new QPlainTextEdit(this);
        filesPTE->setVisible(false);
        if (!currentWidget) {
            fileListTB->click();
        }
    } else {
        fileListTB->setEnabled(false);
    }

    if (roles & Enum::RoleGetDepends) {
        dependsOnLV = new QListView(this);
        dependsOnLV->setVisible(false);
        dependsOnLV->setModel(m_pkg_model_dep = new KpkSimplePackageModel(this));
        if (!currentWidget) {
            dependsOnTB->click();
        }
    } else {
        dependsOnTB->setEnabled(false);
    }

    if (roles & Enum::RoleGetRequires) {
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

void KpkPackageDetails::getDetails(QSharedPointer<PackageKit::Package> p)
{
    // create the description transaction
    Transaction *t = Client::instance()->getDetails(p);
    if (t->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
    } else {
        connect(t, SIGNAL(details(QSharedPointer<PackageKit::Package>)),
                this, SLOT(description(QSharedPointer<PackageKit::Package>)));
        connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                this, SLOT(getDetailsFinished(PackageKit::Enum::Exit, uint)));
    }
}

void KpkPackageDetails::description(QSharedPointer<PackageKit::Package> p)
{
    descriptionKTB->clear();
    //format and show description
    Package::Details *details = p->details();
    QString description;
    description += "<table><tbody>";
    if (!details->description().isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("Details")
                    + ":</b></td><td>" + details->description().replace('\n', "<br />")
                    + "</td></tr>";
    }
    if (!details->url().isEmpty()) {
        description += "<tr><td align=\"right\"><b>" + i18n("Home Page")
                    + ":</b></td><td><a href=\"" + details->url() + "\">" + details->url()
                    + "</a></td></tr>";
    }
    if (!details->license().isEmpty() && details->license() != "unknown") {
        description += "<tr><td align=\"right\"><b>" + i18n("License")
                    + ":</b></td><td>" + details->license()
                    + "</td></tr>";
    }
    if (details->group() != Enum::UnknownGroup) {
        description += "<tr><td align=\"right\"><b>" + i18nc("Group of the package", "Group") + ":</b></td><td>"
                    + KpkStrings::groups(details->group())
                    + "</td></tr>";
    }

    if (details->size() > 0) {
        description += "<tr><td align=\"right\"><b>" + i18n("Size")
                    + ":</b></td><td>" + KGlobal::locale()->formatByteSize(details->size())
                    + "</td></tr>";
    }
    description += "</table></tbody>";
    descriptionKTB->setHtml(description);
}

void KpkPackageDetails::getDetailsFinished(PackageKit::Enum::Exit status, uint runtime)
{
    Q_UNUSED(runtime)
    if (status != Enum::ExitSuccess) {
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

void KpkPackageDetails::getFiles(QSharedPointer<PackageKit::Package> p)
{
    // create the files transaction
    Transaction *t = Client::instance()->getFiles(p);
    if (t->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
    } else {
        connect(t, SIGNAL(files(QSharedPointer<PackageKit::Package>, const QStringList &)),
                this, SLOT(files(QSharedPointer<PackageKit::Package>, const QStringList &)));
        connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                this, SLOT(getFilesFinished(PackageKit::Enum::Exit, uint)));
    }
}

void KpkPackageDetails::files(QSharedPointer<PackageKit::Package> package, const QStringList &files)
{
    Q_UNUSED(package)
    filesPTE->clear();
    for (int i = 0; i < files.size(); ++i) {
        filesPTE->appendPlainText(files.at(i));
    }
}

void KpkPackageDetails::getFilesFinished(PackageKit::Enum::Exit status, uint runtime)
{
    Q_UNUSED(runtime)
    if (status == Enum::ExitSuccess) {
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

void KpkPackageDetails::getDepends(QSharedPointer<PackageKit::Package> p)
{
    // create a transaction for the dependecies not recursive
    Transaction *t = Client::instance()->getDepends(p, PackageKit::Enum::NoFilter, false);
    m_pkg_model_dep->clear();
    if (t->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
    } else {
        connect(t, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                m_pkg_model_dep, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
        connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                this, SLOT(getDependsFinished(PackageKit::Enum::Exit, uint)));
    }
}

void KpkPackageDetails::getDependsFinished(PackageKit::Enum::Exit status, uint runtime)
{
    Q_UNUSED(runtime)
    if (status != Enum::ExitSuccess) {
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

void KpkPackageDetails::getRequires(QSharedPointer<PackageKit::Package> p)
{
    // create a transaction for the requirements not recursive
    Transaction *t = Client::instance()->getRequires(p, PackageKit::Enum::NoFilter, false);
    m_pkg_model_req->clear();
    if (t->error()) {
        KMessageBox::sorry(this, KpkStrings::daemonError(t->error()));
    } else {
        connect(t, SIGNAL(package(QSharedPointer<PackageKit::Package>)),
                m_pkg_model_req, SLOT(addPackage(QSharedPointer<PackageKit::Package>)));
        connect(t, SIGNAL(finished(PackageKit::Enum::Exit, uint)),
                this, SLOT(getRequiresFinished(PackageKit::Enum::Exit, uint)));
    }
}

void KpkPackageDetails::getRequiresFinished(PackageKit::Enum::Exit status, uint runtime)
{
    Q_UNUSED(runtime)
    if (status != Enum::ExitSuccess) {
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
