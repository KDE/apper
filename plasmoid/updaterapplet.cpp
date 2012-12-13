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
#include <PkTransaction.h>
#include <PkTransactionProgressModel.h>
#include <PkStrings.h>
#include <PkIcons.h>
#include <ApplicationSortFilterModel.h>

#include <Transaction>
#include <Daemon>

#define FIVE_MIN 360000
#define ONE_MIN   72000

using namespace PackageKit;

UpdaterApplet::UpdaterApplet(QObject *parent, const QVariantList &args) :
    PopupApplet(parent, args),
    m_declarativeWidget(0)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setActive(false);
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

    QTimer::singleShot(FIVE_MIN, this, SIGNAL(getUpdates()));

    PopupApplet::init();
}

UpdaterApplet::~UpdaterApplet()
{
}

Q_DECLARE_METATYPE(PackageKit::Transaction::Status)

QGraphicsWidget *UpdaterApplet::graphicsWidget()
{
    if (!m_declarativeWidget) {
        m_declarativeWidget = new Plasma::DeclarativeWidget(this);
        m_declarativeWidget->engine()->rootContext()->setContextProperty("Daemon", Daemon::global());
        m_declarativeWidget->engine()->rootContext()->setContextProperty("PkStrings", new PkStrings);
        m_declarativeWidget->engine()->rootContext()->setContextProperty("PkIcons", new PkIcons);
        m_declarativeWidget->engine()->rootContext()->setContextProperty("updatesModel", m_updatesModel);
        m_declarativeWidget->engine()->rootContext()->setContextProperty("UpdaterPlasmoid", this);
        qmlRegisterType<PackageModel>("org.kde.apper", 0, 1, "PackageModel");
        qmlRegisterType<PkTransaction>("org.kde.apper", 0, 1, "PkTransaction");
        qmlRegisterType<PkTransactionProgressModel>("org.kde.apper", 0, 1, "PkTransactionProgressModel");
        qmlRegisterType<ApplicationSortFilterModel>("org.kde.apper", 0, 1, "ApplicationSortFilterModel");
        qmlRegisterType<PackageKit::Transaction>("org.packagekit", 0, 1, "Transaction");
        qmlRegisterUncreatableType<PackageKit::Daemon>("org.packagekit", 0, 1, "Daemon", "Global");
        qRegisterMetaType<PackageKit::Transaction::Info>("PackageKit::Transaction::Info");
        qRegisterMetaType<PackageKit::Transaction::Exit>("PackageKit::Transaction::Exit");
        qRegisterMetaType<PackageKit::Transaction::Status>("PackageKit::Transaction::Status");
        qRegisterMetaType<PackageKit::Transaction::Role>("PackageKit::Transaction::Role");
        qRegisterMetaType<PkTransaction::ExitStatus>("PkTransaction::ExitStatus");

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
    if (m_updatesModel->rowCount() == 0) {
        text = i18n("Your system is up to date");
    } else {
        text = i18np("You have one update", "You have %1 updates", m_updatesModel->rowCount());
    }
    Plasma::ToolTipContent content(i18n("Software Updater"),
                                   text,
                                   KIcon("system-software-update"));

    QString icon = "kpackagekit-updates";
    for (int i = 0; i < m_updatesModel->rowCount(); ++i) {
        Transaction::Info info = m_updatesModel->index(i, 0).data(PackageModel::InfoRole).value<Transaction::Info>();
        if (info == Transaction::InfoSecurity) {
            icon = "kpackagekit-security";
            break;
        } else if (info == Transaction::InfoSecurity) {
            icon = "kpackagekit-important";
        }
    }
    setPopupIcon(icon);

    Plasma::ToolTipManager::self()->setContent(this, content);
}

void UpdaterApplet::setActive(bool active)
{
    if (active) {
        setStatus(Plasma::ActiveStatus);
    } else if (status() != Plasma::PassiveStatus && status() != Plasma::NeedsAttentionStatus) {
        setStatus(Plasma::PassiveStatus);
    }
}

uint UpdaterApplet::getTimeSinceLastRefresh()
{
    return Daemon::global()->getTimeSinceAction(Transaction::RoleRefreshCache);
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

    if (!isIconified()) {
        emit getUpdates();
    }
}

void UpdaterApplet::popupEvent(bool show)
{
    if (show) {
        emit getUpdates();
    } else if (status() != Plasma::NeedsAttentionStatus &&
               m_updatesModel->rowCount() == 0) {
        // if the plasmoid was open and your updates
        // were installed, after clicking the plasmoid
        // icon, and hiding it the icon is kept at active status
        setStatus(Plasma::PassiveStatus);
    }
}

#include "updaterapplet.moc"
