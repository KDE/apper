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

#include "KpkAbstractTask.h"

#include "InfoWidget.h"

#include <limits.h>
#include <QtDBus/QDBusConnection>
#include <QTimer>
#include <QStringList>
#include <QFile>

#include <KWindowSystem>
#include <KLocale>

#include <KDebug>

#include <Daemon>

using namespace PackageKit;

KpkAbstractTask::KpkAbstractTask(uint xid, const QString &interaction, const QDBusMessage &message, QWidget *parent)
 : KDialog(parent),
   m_xid(xid),
   m_message(message)
{
    setWindowIcon(KIcon("applications-other"));
    setButtons(KDialog::Ok | KDialog::Cancel);
    setButtonText(KDialog::Ok, i18n("Continue"));
    setButtonIcon(KDialog::Ok, KIcon("go-next"));

    QString locale(KGlobal::locale()->language() + '.' + KGlobal::locale()->encoding());
    Daemon::setHints("locale=" + locale);

    // Defaults to always
    m_interactions = ConfirmSearch
                   | ConfirmDeps
                   | ConfirmInstall
                   | Progress
                   | Finished
                   | Warning;
    m_timeout = 0;
    parseInteraction(interaction);

    QString cmdline;
    uint pid;
    // TODO as we are running on the session it might
    // be useless to check the PID on the system
    if ((pid = getPidSession()) != UINT_MAX) {
        cmdline = getCmdLine(pid);
    } else if ((pid = getPidSystem()) != UINT_MAX) {
        cmdline = getCmdLine(pid);
    }

    if (!cmdline.isNull()) {
        setExec(cmdline);
    }

    m_stackedWidget = new QStackedWidget(this);
    KDialog::setMainWidget(m_stackedWidget);

    KWindowSystem::setMainWindow(this, m_xid);
}

KpkAbstractTask::~KpkAbstractTask()
{
}

void KpkAbstractTask::setMainWidget(QWidget *widget)
{
    m_stackedWidget->addWidget(widget);
    m_stackedWidget->setCurrentWidget(widget);
}

QWidget* KpkAbstractTask::mainWidget()
{
    return m_stackedWidget->currentWidget();
}

void KpkAbstractTask::setInfo(const QString &title, const QString &text)
{
    InfoWidget *info = new InfoWidget(this);
    info->setTitle(title);
    info->setDescription(text);
    setMainWidget(info);
    setButtons(KDialog::Close);
}

void KpkAbstractTask::setExec(const QString &exec)
{
    if (pathIsTrusted(exec)) {
        // Get from X11 the window title
        KWindowInfo info = KWindowSystem::windowInfo(m_xid, NET::WMVisibleName);
        parentTitle = info.visibleName();
    } else {
        parentTitle = exec;
    }
}

bool KpkAbstractTask::pathIsTrusted(const QString &exec)
{
    // special case the plugin helper -- it's trusted
    return exec == "/usr/libexec/gst-install-plugins-helper" ||
           exec == "/usr/libexec/pk-gstreamer-install" ||
           exec == "/usr/bin/gstreamer-codec-install" ||
           exec == "/usr/lib/packagekit/pk-gstreamer-install" ||
           exec == "/usr/bin/plasma-desktop" ||
           exec == "/usr/bin/apper";
}

QString KpkAbstractTask::getCmdLine(uint pid)
{
    QFile file(QString("/proc/%1/cmdline").arg(pid));
    QString line;
    if (file.open(QFile::ReadOnly)) {
        char buf[1024];
        qint64 lineLength = file.readLine(buf, sizeof(buf));
        if (lineLength != -1) {
            // the line is available in buf
            line = QString::fromLocal8Bit(buf);
            if (!line.contains("(deleted)")) {
                return line;
            }
        }
    }
    return QString();
}

uint KpkAbstractTask::getPidSystem()
{
    QDBusMessage msg;
    msg = QDBusMessage::createMethodCall("org.freedesktop.DBus",
                                         "/org/freedesktop/DBus/Bus",
                                         "org.freedesktop.DBus",
                                         QLatin1String("GetConnectionUnixProcessID"));
    msg << m_message.service();
    QDBusMessage reply = QDBusConnection::systemBus().call(msg);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        kWarning() << "Message did not receive a reply";
    }

    if (reply.arguments().size() == 1) {
        return reply.arguments().at(0).toUInt();
    }

    return UINT_MAX;
}

uint KpkAbstractTask::getPidSession()
{
    QDBusMessage msg;
    msg = QDBusMessage::createMethodCall("org.freedesktop.DBus",
                                         "/org/freedesktop/DBus/Bus",
                                         "org.freedesktop.DBus",
                                         QLatin1String("GetConnectionUnixProcessID"));
    msg << m_message.service();
    QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
    if (reply.type() != QDBusMessage::ReplyMessage) {
        kWarning() << "Message did not receive a reply";
    }

    if (reply.arguments().size() == 1) {
        return reply.arguments().at(0).toUInt();
    }

    return UINT_MAX;
}

// void KpkAbstractTask::setParentWindow(QWidget *widget)
// {
//     if (m_xid != 0) {
//         KWindowSystem::setMainWindow(widget, m_xid);
//     } else {
// //         updateUserTimestamp(); // make it get focus unconditionally :-/
//     }
// }

void KpkAbstractTask::run()
{
    // Call the start slot
    QTimer::singleShot(0, this, SLOT(start()));
}

void KpkAbstractTask::start()
{
    // kDebug() << "dummy start slot";
}

void KpkAbstractTask::sendErrorFinished(DBusError error, const QString &msg)
{
    QString dbusError;
    switch (error) {
    case Failed:
        dbusError = "org.freedesktop.PackageKit.Failed";
        break;
    case InternalError:
        dbusError = "org.freedesktop.PackageKit.InternalError";
        break;
    case NoPackagesFound:
        dbusError = "org.freedesktop.PackageKit.NoPackagesFound";
        break;
    case Forbidden:
        dbusError = "org.freedesktop.PackageKit.Forbidden";
        break;
    case Cancelled:
        dbusError = "org.freedesktop.PackageKit.Cancelled";
        break;
    }
    QDBusMessage reply;
    reply = m_message.createErrorReply(dbusError, msg);
    QDBusConnection::sessionBus().send(reply);
}

bool KpkAbstractTask::sendMessageFinished(const QDBusMessage &message)
{
    emit finished();
    return QDBusConnection::sessionBus().send(message);
}

uint KpkAbstractTask::parentWId() const
{
    return m_xid;
}

void KpkAbstractTask::parseInteraction(const QString &interaction)
{
    QStringList interactions = interaction.split(',');

    // Enable or disable all options
    if (interactions.contains("always")) {
        m_interactions = ConfirmSearch
                       | ConfirmDeps
                       | ConfirmInstall
                       | Progress
                       | Finished
                       | Warning;
    } else if (interactions.contains("never")) {
        m_interactions = 0;
    }

    // show custom options
    if (interactions.contains("show-confirm-search")) {
        m_interactions |= ConfirmSearch;
    }
    if (interactions.contains("show-confirm-deps")) {
        m_interactions |= ConfirmDeps;
    }
    if (interactions.contains("show-confirm-install")) {
        m_interactions |= ConfirmInstall;
    }
    if (interactions.contains("show-progress")) {
        m_interactions |= Progress;
    }
    if (interactions.contains("show-finished")) {
        m_interactions |= Finished;
    }
    if (interactions.contains("show-warning")) {
        m_interactions |= Warning;
    }

    // hide custom options
    if (interactions.contains("hide-confirm-search")) {
        m_interactions &= ~ConfirmSearch;
    }
    if (interactions.contains("hide-confirm-deps")) {
        m_interactions &= ~ConfirmDeps;
    }
    if (interactions.contains("hide-confirm-install")) {
        m_interactions &= ~ConfirmInstall;
    }
    if (interactions.contains("hide-progress")) {
        m_interactions &= ~Progress;
    }
    if (interactions.contains("hide-finished")) {
        m_interactions &= ~Finished;
    }
    if (interactions.contains("hide-warning")) {
        m_interactions &= ~Warning;
    }

    int index;
    QRegExp rx("^timeout=(\\d+)$");
    index = interactions.indexOf(rx);
    if (index != -1) {
        if (rx.indexIn(interactions.at(index)) != -1) {
            m_timeout = rx.cap(1).toUInt();
        }
    }
}

void KpkAbstractTask::finishTaskOk()
{
    sendMessageFinished(m_message.createReply());
}

KpkAbstractTask::Interactions KpkAbstractTask::interactions() const
{
    return m_interactions;
}

uint KpkAbstractTask::timeout() const
{
    return m_timeout;
}

bool KpkAbstractTask::showConfirmSearch() const
{
    return m_interactions & ConfirmSearch;
}

bool KpkAbstractTask::showConfirmDeps() const
{
    return m_interactions & ConfirmDeps;
}

bool KpkAbstractTask::showConfirmInstall() const
{
    return m_interactions & ConfirmInstall;
}

bool KpkAbstractTask::showProgress() const
{
    return m_interactions & Progress;
}

bool KpkAbstractTask::showFinished() const
{
    return m_interactions & Finished;
}

bool KpkAbstractTask::showWarning() const
{
    return m_interactions & Warning;
}

KpkReviewChanges::OperationModes KpkAbstractTask::operationModes() const
{
    KpkReviewChanges::OperationModes opt;
    opt = KpkReviewChanges::ReturnOnlyWhenFinished;
    if (showConfirmInstall()) {
        opt |= KpkReviewChanges::ShowConfirmation;
    }

    if (!showProgress()) {
        opt |= KpkReviewChanges::HideProgress;
    }

    if (!showConfirmDeps()) {
        opt |= KpkReviewChanges::HideConfirmDeps;
    }

    return opt;
}

#include "KpkAbstractTask.moc"
