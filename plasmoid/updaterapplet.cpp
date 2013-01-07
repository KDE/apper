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

#include "DBusUpdaterInterface.h"

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
    m_declarativeWidget(0),
    m_initted(false)
{
    QAction *action = new QAction(i18n("Check for new updates"), this);
    action->setIcon(KIcon("view-refresh"));
    connect(action, SIGNAL(triggered()), this, SIGNAL(checkForNewUpdates()));
    connect(action, SIGNAL(triggered()), this, SLOT(showPopup()));
    connect(action, SIGNAL(triggered()), this, SLOT(setActive()));
    m_actions << action;

    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setActive(false);
    setPopupIcon("kpackagekit-updates");

    m_updatesModel = new PackageModel(this);
    m_updatesModel->setCheckable(true);

    m_interface = new DBusUpdaterInterface(this);
    connect(m_interface, SIGNAL(reviewUpdates()), this, SIGNAL(reviewUpdates()));
    connect(m_interface, SIGNAL(reviewUpdates()), this, SLOT(showPopup()));

    m_getUpdatesTimer = new QTimer(this);
    m_getUpdatesTimer->setInterval(1000);
    m_getUpdatesTimer->setSingleShot(true);
    connect(m_getUpdatesTimer, SIGNAL(timeout()), this, SIGNAL(getUpdates()));
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

    QTimer::singleShot(ONE_MIN, this, SIGNAL(getUpdates()));

    PopupApplet::init();
}

QList<QAction *> UpdaterApplet::contextualActions()
{
    return m_actions;
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
    Plasma::ToolTipContent content(text,
                                   QString(),
                                   KIcon("system-software-update"));

    Plasma::ToolTipManager::self()->setContent(this, content);
}

void UpdaterApplet::updateIcon()
{
    QString icon = "kpackagekit-updates";
    for (int i = 0; i < m_updatesModel->rowCount(); ++i) {
        Transaction::Info info = m_updatesModel->index(i, 0).data(PackageModel::InfoRole).value<Transaction::Info>();
        if (info == Transaction::InfoSecurity) {
            icon = "kpackagekit-security";
            break;
        } else if (info == Transaction::InfoImportant) {
            icon = "kpackagekit-important";
        }
    }
    setPopupIcon(icon);
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

    if (!m_interface->isRegistered() && isIconified()) {
        // Register the org.kde.ApperUpdaterIcon interface
        // so KDED can tell us to show the review list popup
        m_interface->registerService();
    } else if (m_interface->isRegistered() && !isIconified()) {
        m_interface->unregisterService();
    }

    if (isIconified()) {
        m_getUpdatesTimer->stop();
    } else {
        m_getUpdatesTimer->start();
    }
}

void UpdaterApplet::popupEvent(bool show)
{
    if (show) {
        emit getUpdates();

        // THIS IS A HACK
        // for some reason the active focus is not
        // set when the popup is shown...
        QTimer::singleShot(500, m_declarativeWidget->rootObject(), SLOT(forceActiveFocus()));
    } else if (status() != Plasma::NeedsAttentionStatus &&
               m_updatesModel->rowCount() == 0) {
        // if the plasmoid was open and your updates
        // were installed, after clicking the plasmoid
        // icon, and hiding it the icon is kept at active status
        setStatus(Plasma::PassiveStatus);
    }
}

#include "updaterapplet.moc"
