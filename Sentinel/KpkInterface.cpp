/***************************************************************************
 *   Copyright (C) 2008-2010 Daniel Nicoletti <dantti85-pk@yahoo.com.br>   *
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

#include "KpkInterface.h"

#include "appersentineladaptor.h"

#include <QtDBus/QDBusConnection>
#include <KpkIcons.h>
#include <KpkMacros.h>
#include <KIcon>
#include <KNotification>

#ifdef HAVE_DEBCONFKDE
#include <KDialog>
#include <KWindowSystem>
#endif

#include <KDebug>

using namespace PackageKit;

KpkInterface::KpkInterface(QObject *parent)
        : QObject(parent)
{
    kDebug() << "Creating Helper";
    (void) new ApperSentinelAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService("org.kde.ApperSentinel")) {
        kDebug() << "another helper is already running";
        return;
    }

    if (!QDBusConnection::sessionBus().registerObject("/", this)) {
        kDebug() << "unable to register service interface to dbus";
        return;
    }
}

KpkInterface::~KpkInterface()
{
    kDebug();
#ifdef HAVE_DEBCONFKDE
    foreach (const DebconfGui *gui, m_debconfGuis.values()) {
        delete gui;
    }
#endif //HAVE_DEBCONFKDE
}

void KpkInterface::RefreshCache()
{
    emit refresh();
}

void KpkInterface::Update()
{
    // This is to show updates when the session
    // starts and it's not time to refresh the cache
    emit refreshAndUpdate(false);
}

void KpkInterface::RefreshAndUpdate()
{
    // This is to show updates and refresh the cache
    emit refreshAndUpdate(true);
}

void KpkInterface::SetupDebconfDialog(const QString &socketPath, uint xidParent)
{
    kDebug() << socketPath << xidParent;
#ifdef HAVE_DEBCONFKDE
    DebconfGui *gui;
    if (m_debconfGuis.contains(socketPath)) {
        gui = m_debconfGuis[socketPath];
    } else {
        gui = new DebconfGui(socketPath);
        gui->setWindowModality(Qt::WindowModal);
        gui->setWindowFlags(Qt::Dialog);
        m_debconfGuis[socketPath] = gui;
        connect(gui, SIGNAL(activated()), this, SLOT(debconfActivate()));
        connect(gui, SIGNAL(deactivated()), gui, SLOT(hide()));
    }
    gui->setProperty("xidParent", xidParent);
#else
    Q_UNUSED(socketPath)
    Q_UNUSED(xidParent)
    kDebug() << "Not compiled with Debconf support - ignoring";
#endif //HAVE_DEBCONFKDE
}

#ifdef HAVE_DEBCONFKDE
void KpkInterface::debconfActivate()
{
    // Correct the parent
    DebconfGui *gui = qobject_cast<DebconfGui*>(sender());
    uint xidParent  = gui->property("xidParent").toUInt();
    KWindowSystem::setMainWindow(gui, xidParent);
    gui->show();
}
#endif

#include "KpkInterface.moc"
