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

#include "SessionTask.h"
#include "ui_SessionTask.h"

#include "InfoWidget.h"
#include "ReviewChanges.h"

#include <limits.h>
#include <QtDBus/QDBusConnection>
#include <QTimer>
#include <QStringList>
#include <QFile>

#include <KWindowSystem>
#include <KLocale>
#include <KGlobalSettings>

#include <KDebug>

#include <Daemon>

using namespace PackageKit;

SessionTask::SessionTask(uint xid, const QString &interaction, const QDBusMessage &message, QWidget *parent)
 : KDialog(parent),
   m_xid(xid),
   m_message(message),
   ui(new Ui::SessionTask)
{
    ui->setupUi(KDialog::mainWidget());
    setModal(true);

    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
            this, SLOT(updatePallete()));
    updatePallete();

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

    setMinimumSize(QSize(420,280));
    KConfig config;
    KConfigGroup configGroup(&config, "SessionInstaller");
    restoreDialogSize(configGroup);
}

SessionTask::~SessionTask()
{
    KConfig config;
    KConfigGroup configGroup(&config, "SessionInstaller");
    saveDialogSize(configGroup);

    delete ui;
}

void SessionTask::addPackage(const PackageKit::Package &package)
{
    m_foundPackages.append(package);
}

void SessionTask::searchFinished(PkTransaction::ExitStatus status)
{
    if (status == PkTransaction::Success) {
        if (m_foundPackages.isEmpty()) {
            notFound();
        } else {
            searchSuccess();
        }
    } else {
        searchFailed();
    }
}

void SessionTask::updatePallete()
{
    QPalette pal;
    pal.setColor(QPalette::Window, KGlobalSettings::activeTitleColor());
    pal.setColor(QPalette::WindowText, KGlobalSettings::activeTextColor());
    ui->titleL->setPalette(pal);
}

void SessionTask::setMainWidget(QWidget *widget)
{
    ui->stackedWidget->addWidget(widget);
    ui->stackedWidget->setCurrentWidget(widget);
    if (widget->objectName() == QLatin1String("PkTransaction")) {
        PkTransaction *trans = qobject_cast<PkTransaction*>(widget);
        trans->hideCancelButton();
        connect(trans, SIGNAL(titleChanged(QString)),
                this, SLOT(setTitle(QString)));
        setTitle(trans->title());
    }
}

QWidget* SessionTask::mainWidget()
{
    return ui->stackedWidget->currentWidget();
}

void SessionTask::setInfo(const QString &title, const QString &text)
{
    InfoWidget *info = new InfoWidget(this);
    setTitle(title);
    info->setDescription(text);
    setMainWidget(info);
    setButtons(KDialog::Close);
}

void SessionTask::setError(const QString &title, const QString &text)
{
    InfoWidget *info = new InfoWidget(this);
    setTitle(title);
    info->setDescription(text);
    info->setIcon(KIcon("dialog-error"));
    setMainWidget(info);
    setButtons(KDialog::Close);
}

void SessionTask::setTitle(const QString &title)
{
    ui->titleL->setText(title);
}

void SessionTask::setExec(const QString &exec)
{
    if (pathIsTrusted(exec)) {
        // Get from X11 the window title
        KWindowInfo info = KWindowSystem::windowInfo(m_xid, NET::WMVisibleName);
        parentTitle = info.visibleName();
    } else {
        parentTitle = exec;
    }
}

bool SessionTask::pathIsTrusted(const QString &exec)
{
    // special case the plugin helper -- it's trusted
    return exec == "/usr/libexec/gst-install-plugins-helper" ||
           exec == "/usr/libexec/pk-gstreamer-install" ||
           exec == "/usr/bin/gstreamer-codec-install" ||
           exec == "/usr/lib/packagekit/pk-gstreamer-install" ||
           exec == "/usr/bin/plasma-desktop" ||
           exec == "/usr/bin/apper";
}

QString SessionTask::getCmdLine(uint pid)
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

uint SessionTask::getPidSystem()
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

uint SessionTask::getPidSession()
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

// void SessionTask::setParentWindow(QWidget *widget)
// {
//     if (m_xid != 0) {
//         KWindowSystem::setMainWindow(widget, m_xid);
//     } else {
// //         updateUserTimestamp(); // make it get focus unconditionally :-/
//     }
// }

void SessionTask::search()
{
    kDebug() << "virtual method called, falling back to commit()";
    commit();
}

void SessionTask::commit()
{
    kDebug() << "virtual method called";
}

void SessionTask::notFound()
{
    kDebug() << "virtual method called";
    if (showWarning()) {
        setInfo(i18n("Could not find"),
                i18n("No packages were found that meet the request"));
    }
    sendErrorFinished(NoPackagesFound, "no package found");
}

void SessionTask::searchFailed()
{
    kDebug() << "virtual method called";
    setInfo(i18n("Failed to find"),
            i18n("No packages were found that meet the request"));
    sendErrorFinished(Failed, "failed to search");
}

void SessionTask::searchSuccess()
{
    kDebug() << "virtual method called";
    ReviewChanges *frm = new ReviewChanges(m_foundPackages, this);
    setTitle(frm->title());
    setMainWidget(frm);
//            if (frm->exec(operationModes()) == 0) {
//                sendErrorFinished(Failed, i18n("Transaction did not finish with success"));
//            } else {
//                finishTaskOk();
//            }
}

void SessionTask::slotButtonClicked(int button)
{
    if (button == KDialog::Ok) {
        kDebug() << mainWidget()->objectName();
        if (mainWidget()->objectName() == "IntroDialog") {
            enableButtonOk(false);
            search();
        } else if (mainWidget()->objectName() == "ReviewChanges") {
            enableButtonOk(false);
            commit();
        }
    } else {
        KDialog::slotButtonClicked(button);
        sendErrorFinished(Cancelled, "Aborted by the user");
    }
}

void SessionTask::sendErrorFinished(DBusError error, const QString &msg)
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

bool SessionTask::sendMessageFinished(const QDBusMessage &message)
{
    emit finished();
    return QDBusConnection::sessionBus().send(message);
}

uint SessionTask::parentWId() const
{
    return m_xid;
}

void SessionTask::parseInteraction(const QString &interaction)
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

bool SessionTask::foundPackages() const
{
    return !m_foundPackages.isEmpty();
}

int SessionTask::foundPackagesSize() const
{
    return m_foundPackages.size();
}

QList<Package> SessionTask::foundPackagesList() const
{
    return m_foundPackages;
}

void SessionTask::finishTaskOk()
{
    sendMessageFinished(m_message.createReply());
}

SessionTask::Interactions SessionTask::interactions() const
{
    return m_interactions;
}

uint SessionTask::timeout() const
{
    return m_timeout;
}

bool SessionTask::showConfirmSearch() const
{
    return m_interactions & ConfirmSearch;
}

bool SessionTask::showConfirmDeps() const
{
    return m_interactions & ConfirmDeps;
}

bool SessionTask::showConfirmInstall() const
{
    return m_interactions & ConfirmInstall;
}

bool SessionTask::showProgress() const
{
    return m_interactions & Progress;
}

bool SessionTask::showFinished() const
{
    return m_interactions & Finished;
}

bool SessionTask::showWarning() const
{
    return m_interactions & Warning;
}

//ReviewChanges::OperationModes SessionTask::operationModes() const
//{
//    ReviewChanges::OperationModes opt;
//    opt = ReviewChanges::ReturnOnlyWhenFinished;
//    if (showConfirmInstall()) {
//        opt |= ReviewChanges::ShowConfirmation;
//    }

//    if (!showProgress()) {
//        opt |= ReviewChanges::HideProgress;
//    }

//    if (!showConfirmDeps()) {
//        opt |= ReviewChanges::HideConfirmDeps;
//    }

//    return opt;
//}

#include "SessionTask.moc"
