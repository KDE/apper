/***************************************************************************
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *   Copyright (C) 2012 by Luís Gabriel Lima <lampih@gmail.com>            *
 *   Copyright (C) 2012 by Daniel Nicoletti <dantti12@gmail.com>           *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "updaterapplet.h"

#include <QtGui/QGraphicsLinearLayout>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative>

#include <Plasma/ToolTipManager>
#include <Plasma/ToolTipContent>
#include <Plasma/DeclarativeWidget>
#include <Plasma/Package>

#include <PackageModel.h>
#include <Transaction>
#include <PackageDetails>
#include <PackageUpdateDetails>

using namespace PackageKit;

UpdaterApplet::UpdaterApplet(QObject *parent, const QVariantList &args) :
    PopupApplet(parent, args),
    m_declarativeWidget(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setPopupIcon("kpackagekit-updates");

    m_updatesModel = new PackageModel(this);
    m_updatesModel->setCheckable(true);
}

void UpdaterApplet::init()
{
    switch (formFactor()) {
    case Plasma::Horizontal:
    case Plasma::Vertical:
        Plasma::ToolTipManager::self()->registerWidget(this);
        break;
    default:
        Plasma::ToolTipManager::self()->unregisterWidget(this);
        break;
    }

    PopupApplet::init();
}

UpdaterApplet::~UpdaterApplet()
{
}

QML_DECLARE_TYPE(PackageKit::PackageUpdateDetails)
QML_DECLARE_TYPE(PackageKit::PackageUpdateDetails::UpdateState)
QML_DECLARE_TYPE(PackageKit::PackageUpdateDetails::Restart)

QGraphicsWidget *UpdaterApplet::graphicsWidget()
{
    if (!m_declarativeWidget) {
        m_declarativeWidget = new Plasma::DeclarativeWidget(this);

        m_declarativeWidget->engine()->rootContext()->setContextProperty("updatesModel", m_updatesModel);
        qmlRegisterType<PackageModel>("org.kde.apper", 0, 1, "PackageModel");
        qmlRegisterType<PackageKit::Transaction>("org.packagekit", 0, 1, "Transaction");
        qmlRegisterType<PackageKit::Package>("org.packagekit", 0, 1, "Pkg");
        qmlRegisterType<PackageKit::PackageDetails>("org.packagekit", 0, 1, "PkgDetails");
        qmlRegisterType<PackageKit::PackageUpdateDetails>("org.packagekit", 0, 1, "PkgUpdateDetails");
        qmlRegisterUncreatableType<PackageKit::PackageUpdateDetails::UpdateState>("org.packagekit", 0, 1, "PkgUpdateDetailsState", "enum");
        qmlRegisterUncreatableType<PackageKit::PackageUpdateDetails::Restart>("org.packagekit", 0, 1, "PkgUpdateDetailsRestart", "enum");

        Plasma::PackageStructure::Ptr structure = Plasma::PackageStructure::load("Plasma/Generic");
        Plasma::Package package(QString(), "org.packagekit.updater", structure);
        m_declarativeWidget->setQmlPath(package.filePath("mainscript"));
    }
    return m_declarativeWidget;
}

void UpdaterApplet::toolTipAboutToShow()
{
    if (isPopupShowing()) {
        Plasma::ToolTipManager::self()->clearContent(this);
        return;
    }

    QString text;
    text = i18np("You have one update", "You have %1 updates", m_updatesModel->rowCount());
    Plasma::ToolTipContent content(i18n("Software Updater"),
                                   text,
                                   KIcon("system-software-update"));

    QString icon = "kpackagekit-updates";
    for (int i = 0; i < m_updatesModel->rowCount(); ++i) {
        Package::Info info = m_updatesModel->index(i, 0).data(PackageModel::InfoRole).value<Package::Info>();
        if (info == Package::InfoSecurity) {
            icon = "kpackagekit-security";
            break;
        } else if (info == Package::InfoSecurity) {
            icon = "kpackagekit-important";
        }
    }
    setPopupIcon(icon);

    Plasma::ToolTipManager::self()->setContent(this, content);
}

void UpdaterApplet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        switch (formFactor()) {
        case Plasma::Horizontal:
        case Plasma::Vertical:
            Plasma::ToolTipManager::self()->registerWidget(this);
            break;
        default:
            Plasma::ToolTipManager::self()->unregisterWidget(this);
            break;
        }
    }
}

#include "updaterapplet.moc"
