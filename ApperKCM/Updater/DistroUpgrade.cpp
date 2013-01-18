/***************************************************************************
 *   Copyright (C) 2009-2011 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
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

#include "DistroUpgrade.h"

#include <PkIcons.h>

#include <QAction>

#include <KLocale>
#include <KMessageBox>
#include <KColorScheme>

#include <solid/powermanagement.h>
#include <solid/device.h>
#include <solid/acadapter.h>

#include <KDebug>

DistroUpgrade::DistroUpgrade(QWidget *parent) :
    KMessageWidget(parent)
{
    QAction *action = new QAction(i18n("Upgrade"), this);
    connect(action, SIGNAL(triggered()), this, SLOT(startDistroUpgrade()));
    addAction(action);
}

DistroUpgrade::~DistroUpgrade()
{
}

void DistroUpgrade::setName(const QString &name)
{
    setText(i18n("Distribution upgrade available: %1", name));
}

void DistroUpgrade::startDistroUpgrade()
{
    QList<Solid::Device> powerPlugs = Solid::Device::listFromType(Solid::DeviceInterface::AcAdapter);
    bool pluggedIn = true;
    bool hasBattery = Solid::Device::listFromType(Solid::DeviceInterface::Battery).size()>0;
    foreach(const Solid::Device &dev, powerPlugs) {
        if (!dev.as<Solid::AcAdapter>()->isPlugged()) {
            pluggedIn = false;
        }
    }

    QString warning = i18n("You are about to upgrade your distribution to the latest version. "
                           "This is usually a very lengthy process and takes a lot longer than "
                           "simply upgrading your packages.");

    if (!pluggedIn) {
        warning += ' ' + i18n("It is recommended to plug in your computer before proceeding.");
    } else if (hasBattery) {
        warning += ' ' + i18n("It is recommended that you keep your computer plugged in while the upgrade is being performed.");
    }

    if (KMessageBox::warningContinueCancel(this,warning) == KMessageBox::Continue) {
        m_distroUpgradeProcess = new QProcess;
        connect(m_distroUpgradeProcess, SIGNAL(error(QProcess::ProcessError)),
                this, SLOT(distroUpgradeError(QProcess::ProcessError)));
        connect(m_distroUpgradeProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(distroUpgradeFinished(int,QProcess::ExitStatus)));
        QStringList env = QProcess::systemEnvironment();
        env << "DESKTOP=kde";
        m_distroUpgradeProcess->setEnvironment(env);
        m_distroUpgradeProcess->start("/usr/share/PackageKit/pk-upgrade-distro.sh");
    }
}

void DistroUpgrade::distroUpgradeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        KMessageBox::information(this, i18n("Distribution upgrade complete."));
    } else if (exitStatus == QProcess::NormalExit) {
        KMessageBox::sorry(this, i18n("Distribution upgrade process exited with code %1.", exitCode));
    }
    m_distroUpgradeProcess->deleteLater();
    m_distroUpgradeProcess = 0;
}

void DistroUpgrade::distroUpgradeError(QProcess::ProcessError error)
{
    QString text;
    switch(error) {
        case QProcess::FailedToStart:
            KMessageBox::sorry(this,
                    i18n("The distribution upgrade process failed to start."));
            break;
        case QProcess::Crashed:
            KMessageBox::sorry(this,
                    i18n("The distribution upgrade process crashed some time after starting successfully."));
            break;
        default:
            KMessageBox::sorry(this,
                    i18n("The distribution upgrade process failed with an unknown error."));
            break;
    }
}

#include "DistroUpgrade.moc"
