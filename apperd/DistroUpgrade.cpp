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

#include <Daemon>

#include <Enum.h>

#include <KNotification>
#include <KLocale>
#include <KIcon>

#include <KDebug>

DistroUpgrade::DistroUpgrade(QObject *parent) :
    QObject(parent),
    m_distroUpgradeProcess(0),
    m_transaction(0)
{
}

DistroUpgrade::~DistroUpgrade()
{
}

void DistroUpgrade::setConfig(const QVariantHash &configs)
{
    m_configs = configs;
}

void DistroUpgrade::checkDistroUpgrades()
{
    // Ignore check if the user disabled it
    if (m_configs[CFG_DISTRO_UPGRADE].toInt() == Enum::DistroNever) {
        return;
    }

    if (!m_transaction) {
        m_transaction = Daemon::getDistroUpgrades();
        connect(m_transaction, SIGNAL(distroUpgrade(PackageKit::Transaction::DistroUpgrade,QString,QString)),
                this, SLOT(distroUpgrade(PackageKit::Transaction::DistroUpgrade,QString,QString)));
        connect(m_transaction, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                this, SLOT(checkDistroFinished(PackageKit::Transaction::Exit,uint)));

    }
}

void DistroUpgrade::distroUpgrade(PackageKit::Transaction::DistroUpgrade type, const QString &name, const QString &description)
{
    // TODO make use of the type
    switch (m_configs[CFG_DISTRO_UPGRADE].toInt()) {
    case Enum::DistroNever:
        return;
    case Enum::DistroStable:
        if (type != Transaction::DistroUpgradeStable) {
            // The user only wants to know about stable releases
            return;
        }
    default:
        break;
    }

    kDebug() << "Distro upgrade found!" << name << description;
    if (m_shownDistroUpgrades.contains(name)) {
        // ignore distro upgrade if the user already saw it
        return;
    }

    KNotification *notify = new KNotification("DistroUpgradeAvailable", 0, KNotification::Persistent);
    notify->setComponentData(KComponentData("apperd"));
    notify->setTitle(i18n("Distribution upgrade available"));
    notify->setText(description);

    QStringList actions;
    actions << i18n("Start upgrade now");
    notify->setActions(actions);
    connect(notify, SIGNAL(activated(uint)),
            this, SLOT(handleDistroUpgradeAction(uint)));
    notify->sendEvent();
    m_shownDistroUpgrades << name;
}

void DistroUpgrade::checkDistroFinished(Transaction::Exit status, uint enlapsed)
{
    Q_UNUSED(status)
    Q_UNUSED(enlapsed)
    m_transaction = 0;
}

void DistroUpgrade::handleDistroUpgradeAction(uint action)
{
    // get the sender cause there might be more than one
    KNotification *notify = qobject_cast<KNotification*>(sender());
    switch(action) {
        case 1:
            // Check to see if there isn't another process running
            if (m_distroUpgradeProcess) {
                // if so we BREAK otherwise our running count gets
                // lost, and we leak as we don't close the caller.
                break;
            }
            m_distroUpgradeProcess = new QProcess;
            connect (m_distroUpgradeProcess, SIGNAL(error(QProcess::ProcessError)),
                    this, SLOT(distroUpgradeError(QProcess::ProcessError)));
            connect (m_distroUpgradeProcess, SIGNAL(finished(int,QProcess::ExitStatus)),
                    this, SLOT(distroUpgradeFinished(int,QProcess::ExitStatus)));
            QStringList env = QProcess::systemEnvironment();
            env << "DESKTOP=kde";
            m_distroUpgradeProcess->setEnvironment(env);
            m_distroUpgradeProcess->start("/usr/share/PackageKit/pk-upgrade-distro.sh");
            // TODO
//             suppressSleep(true);
            break;
        // perhaps more actions needed in the future
    }
    // in persistent mode we need to manually close it
    notify->close();
}

void DistroUpgrade::distroUpgradeFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    KNotification *notify = new KNotification("DistroUpgradeFinished");
    notify->setComponentData(KComponentData("apperd"));
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        notify->setPixmap(KIcon("security-high").pixmap(64, 64));
        notify->setText(i18n("Distribution upgrade finished. "));
    } else if (exitStatus == QProcess::NormalExit) {
        notify->setPixmap(KIcon("dialog-warning").pixmap(64, 64));
        notify->setText(i18n("Distribution upgrade process exited with code %1.", exitCode));
    }/* else {
        notify->setText(i18n("Distribution upgrade didn't exit normally, the process probably crashed. "));
    }*/
    notify->sendEvent();
    m_distroUpgradeProcess->deleteLater();
    m_distroUpgradeProcess = 0;
//     suppressSleep(false);
}

void DistroUpgrade::distroUpgradeError(QProcess::ProcessError error)
{
    QString text;

    KNotification *notify = new KNotification("DistroUpgradeError");
    notify->setComponentData(KComponentData("apperd"));
    switch(error) {
        case QProcess::FailedToStart:
            text = i18n("The distribution upgrade process failed to start.");
            break;
        case QProcess::Crashed:
            text = i18n("The distribution upgrade process crashed some time after starting successfully.") ;
            break;
        default:
            text = i18n("The distribution upgrade process failed with an unknown error.");
            break;
    }
    notify->setPixmap(KIcon("dialog-error").pixmap(64,64));
    notify->setText(text);
    notify->sendEvent();
}

#include "DistroUpgrade.moc"
