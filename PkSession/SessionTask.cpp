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

#include "SessionTask.h"
#include "ui_SessionTask.h"

#include "IntroDialog.h"
#include "InfoWidget.h"
#include "ReviewChanges.h"
#include "ApplicationLauncher.h"

#include <PkStrings.h>
#include <PackageModel.h>
#include <PkTransactionWidget.h>

#include <limits.h>
#include <QtDBus/QDBusConnection>
#include <QTimer>
#include <QStringList>
#include <QFile>
#include <QSignalMapper>
#include <QStringBuilder>
#include <QDialogButtonBox>

//#include <KWindowSystem>
#include <KLocalizedString>
//#include <KGlobalSettings>
#include <QPushButton>
//#include <KGlobal>

#include <QLoggingCategory>

#include <Daemon>

Q_DECLARE_LOGGING_CATEGORY(APPER_SESSION)

using namespace PackageKit;

SessionTask::SessionTask(uint xid, const QString &interaction, const QDBusMessage &message, QWidget *parent) :
    QDialog(parent),
    m_xid(xid),
    m_message(message),
    m_reviewChanges(0),
    m_pkTransaction(0),
    ui(new Ui::SessionTask)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    m_model = new PackageModel(this);

//    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
//            this, SLOT(updatePallete()));
    updatePallete();

    setWindowIcon(QIcon::fromTheme("system-software-install"));
//    setButtons(KDialog::Ok | KDialog::Cancel);
//    setButtonText(KDialog::Ok, i18n("Continue"));
//    setButtonIcon(KDialog::Ok, QIcon::fromTheme("go-next"));
    enableButtonOk(false);

    //TODO FIXME
//    QString locale(KLocale::global()->language() % QLatin1Char('.') % KLocale::global()->encoding());
//    Daemon::global()->setHints(QLatin1String("locale=") % locale);

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

    setMinimumSize(QSize(430,280));
//    KConfig config("apper");
//    KConfigGroup configGroup(&config, "SessionInstaller");
//    restoreDialogSize(configGroup);
}

SessionTask::~SessionTask()
{
//    KConfig config("apper");
//    KConfigGroup configGroup(&config, "SessionInstaller");
//    saveDialogSize(configGroup);

    delete ui;
}

void SessionTask::addPackage(Transaction::Info info, const QString &packageID, const QString &summary)
{
    m_model->addSelectedPackage(info, packageID, summary);
}

void SessionTask::searchFinished(PkTransaction::ExitStatus status)
{
    if (m_pkTransaction) {
        // Disconnect so it can be connected to commitFinished latter
        disconnect(m_pkTransaction->transaction(), &PkTransaction::finished, this, &SessionTask::searchFinished);
    }

    if (status == PkTransaction::Success) {
        m_model->finished();
        if (m_model->rowCount() == 0) {
            notFound();
        } else {
            searchSuccess();
        }
    } else if (status == PkTransaction::Cancelled) {
        // TODO PORT
//        slotButtonClicked(KDialog::Cancel);
    } else {
        searchFailed();
    }
}

void SessionTask::commitFinished(PkTransaction::ExitStatus status)
{
    if (m_pkTransaction) {
        // Disconnect so it can be connected to something else latter
        disconnect(m_pkTransaction->transaction(), &PkTransaction::finished, this, &SessionTask::searchFinished);
    }

    if (status == PkTransaction::Success) {
        if (!m_removePackages.isEmpty()) {
            removePackages();
        } else {
            commitSuccess();
        }
    } else if (status == PkTransaction::Cancelled) {
        // TODO PORT
//        slotButtonClicked(KDialog::Cancel);
    } else {
        commitFailed();
    }
}

void SessionTask::updatePallete()
{
    QPalette pal;
//    pal.setColor(QPalette::Window, KGlobalSettings::activeTitleColor());
//    pal.setColor(QPalette::WindowText, KGlobalSettings::activeTextColor());
    ui->backgroundFrame->setPalette(pal);
}

void SessionTask::setDialog(QDialog *dialog)
{
    // Store the current values
    QWidget *widget = ui->stackedWidget->currentWidget();

    if (qobject_cast<ApplicationLauncher*>(dialog)) {
        // TODO if there is a removal after instalation
        // this will break it, but we don't have
        // this case yet...
        commitSuccess(dialog);
    } else {
        // Set the new ones
//        setMainWidget(dialog->mainWidget());
//        setTitle(dialog->windowTitle()); // must come after
//        connect(this, SIGNAL(okClicked()),
//                dialog, SLOT(accept()));
//        connect(this, SIGNAL(okClicked()),
//                dialog->mainWidget(), SLOT(deleteLater()));
//        connect(this, SIGNAL(okClicked()),
//                dialog, SLOT(deleteLater()));

        // Make sure we see the last widget and title
        auto mapper = new QSignalMapper(this);
        mapper->setMapping(this, widget);
        connect(this, SIGNAL(okClicked()),
                mapper, SLOT(map()));
        connect(mapper, SIGNAL(mapped(QWidget*)),
                this, SLOT(setMainWidget(QWidget*)));
        enableButtonOk(true);
    }
}

void SessionTask::setMainWidget(QWidget *widget)
{
    if (widget != mainWidget()) {
        ui->stackedWidget->addWidget(widget);
        ui->stackedWidget->setCurrentWidget(widget);
        setTitle(widget->windowTitle());
    }
}

QWidget* SessionTask::mainWidget()
{
    return ui->stackedWidget->currentWidget();
}

void SessionTask::setInfo(const QString &title, const QString &text, const QString &details)
{
    auto info = new InfoWidget(this);
    info->setWindowTitle(title);
    info->setDescription(text);
    info->setDetails(details);
    setMainWidget(info);
//    setButtons(KDialog::Close);
//    button(KDialog::Close)->setFocus();

    if (qobject_cast<PkTransaction*>(sender())) {
        // if we have a sender this method was caller by PkTransaction
        // be carefull because QSignalMapper from KDialog also calls this method
        sender()->disconnect();
        sendErrorFinished(Failed, text);
    }
}

void SessionTask::setError(const QString &title, const QString &text, const QString &details)
{
    auto info = new InfoWidget(this);
    info->setWindowTitle(title);
    info->setDescription(text);
    info->setIcon(QIcon::fromTheme("dialog-error"));
    info->setDetails(details);
    setMainWidget(info);
//    setButtons(KDialog::Close);
//    button(KDialog::Close)->setFocus();

    if (qobject_cast<PkTransaction*>(sender())) {
        // if we have a sender this method was caller by PkTransaction
        // be carefull because QSignalMapper from KDialog also calls this method
        sender()->disconnect();
        sendErrorFinished(Failed, text);
    }
}

void SessionTask::setFinish(const QString &title, const QString &text, QWidget *widget)
{
    auto info = new InfoWidget(this);
    info->setWindowTitle(title);
    info->setDescription(text);
    info->setIcon(QIcon::fromTheme("dialog-ok-apply"));
    info->addWidget(widget);
    setMainWidget(info);
//    setButtons(KDialog::Close);
//    button(KDialog::Close)->setFocus();
}

void SessionTask::setTitle(const QString &title)
{
    ui->titleL->setText(title);
}

void SessionTask::setExec(const QString &exec)
{
    if (pathIsTrusted(exec)) {
        // Get from X11 the window title
//        KWindowInfo info = KWindowSystem::windowInfo(m_xid, NET::WMVisibleName);
//        parentTitle = info.visibleName();
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
        qCWarning(APPER_SESSION) << "Message did not receive a reply";
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
        qCWarning(APPER_SESSION) << "Message did not receive a reply";
    }

    if (reply.arguments().size() == 1) {
        return reply.arguments().at(0).toUInt();
    }

    return UINT_MAX;
}

void SessionTask::search()
{
    qCDebug(APPER_SESSION) << "virtual method called, falling back to commit()";
    commit();
}

void SessionTask::commit()
{
    qCDebug(APPER_SESSION) << "virtual method called";
    if (m_reviewChanges) {
        QStringList installPackages = m_reviewChanges->model()->selectedPackagesToInstall();
        m_removePackages = m_reviewChanges->model()->selectedPackagesToRemove();

        if (installPackages.isEmpty() && m_removePackages.isEmpty()) {
            setInfo(i18n("There are no packages to Install or Remove"),
                    i18n("This action should not happen"));
            sendErrorFinished(Failed, "to install or remove due to empty lists");
        } else if (!installPackages.isEmpty()) {
            // Install Packages
            auto transaction = new PkTransaction(this);
            setTransaction(Transaction::RoleInstallPackages, transaction);
            connect(transaction, &PkTransaction::finished, this, &SessionTask::commitFinished, Qt::UniqueConnection);
            transaction->installPackages(installPackages);
        } else {
            // Remove them
            removePackages();
        }
    }
}

void SessionTask::removePackages()
{
    // Remove Packages
    auto transaction = new PkTransaction(this);
    setTransaction(Transaction::RoleRemovePackages, transaction);
    connect(transaction, &PkTransaction::finished, this, &SessionTask::commitFinished, Qt::UniqueConnection);
    transaction->removePackages(m_removePackages);
    m_removePackages.clear();
}

void SessionTask::notFound()
{
    qCDebug(APPER_SESSION) << "virtual method called";
    if (showWarning()) {
        setInfo(i18n("Could not find"),
                i18n("No packages were found that meet the request"));
    }
    sendErrorFinished(NoPackagesFound, "no package found");
}

void SessionTask::searchFailed()
{
    qCDebug(APPER_SESSION) << "virtual method called";
    setInfo(i18n("Failed to find"),
            i18n("No packages were found that meet the request"));
    sendErrorFinished(Failed, "failed to search");
}

void SessionTask::searchSuccess()
{
    qCDebug(APPER_SESSION) << "virtual method called";
    enableButtonOk(true);
    m_reviewChanges = new ReviewChanges(m_model, this);
    connect(m_reviewChanges, &ReviewChanges::hasSelectedPackages, this, &SessionTask::enableButtonOk);
    setMainWidget(m_reviewChanges);
}

void SessionTask::commitFailed()
{
    qCDebug(APPER_SESSION) << "virtual method called";
    // This should not be used to display stuff as the transaction should
    // emit error() or info()
//    setInfo(i18n("Failed to commit transaction"),
//            PkStrings::errorMessage(m_pkTransaction->error()));
    sendErrorFinished(Failed, i18n("Transaction did not finish with success"));
}

void SessionTask::commitSuccess(QWidget *widget)
{
    qCDebug(APPER_SESSION) << "virtual method called";
    setFinish(i18n("Task completed"), i18n("All operations were committed successfully"), widget);
    finishTaskOk();
}

void SessionTask::slotButtonClicked(int button)
{
    if (button == QDialogButtonBox::Ok) {
//        qCDebug(APPER_SESSION) << mainWidget()->objectName();
        if (qobject_cast<IntroDialog*>(mainWidget())) {
            enableButtonOk(false);
            search();
        } else if (qobject_cast<ReviewChanges*>(mainWidget())) {
            enableButtonOk(false);
            commit();
        } else {
//            emit okClicked();
        }
    } else {
//        KDialog::slotButtonClicked(button);
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
//    emit finished();
    return QDBusConnection::sessionBus().send(message);
}

uint SessionTask::parentWId() const
{
    return m_xid;
}

void SessionTask::enableButtonOk(bool state)
{
//    KDialog::enableButtonOk(state);
    if (state) {
        // When enabling the Continue button put focus on it
//        button(KDialog::Ok)->setFocus();
    }
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
    return m_model->rowCount();
}

int SessionTask::foundPackagesSize() const
{
    return m_model->rowCount();
}

PackageModel *SessionTask::model() const
{
    return m_model;
}

void SessionTask::setTransaction(Transaction::Role role, PkTransaction *t)
{
    if (m_pkTransaction == 0) {
        m_pkTransaction = new PkTransactionWidget(this);
        m_pkTransaction->hideCancelButton();

        ui->stackedWidget->addWidget(m_pkTransaction);
        connect(m_pkTransaction, &PkTransactionWidget::titleChanged, this, &SessionTask::setTitle);
        connect(this, SIGNAL(cancelClicked()),
                m_pkTransaction, SLOT(cancel()));
        connect(m_pkTransaction, SIGNAL(dialog(KDialog*)),
                this, SLOT(setDialog(KDialog*)));
        connect(m_pkTransaction, SIGNAL(sorry(QString,QString,QString)),
                this, SLOT(setInfo(QString,QString,QString)));
        connect(m_pkTransaction, SIGNAL(error(QString,QString,QString)),
                this, SLOT(setError(QString,QString,QString)));
    }

    if (t) {
        m_pkTransaction->setTransaction(t, role);
//        setTitle(m_pkTransaction->title());
    }

    // avoid changing the current widget
    if (mainWidget() != m_pkTransaction) {
        ui->stackedWidget->setCurrentWidget(m_pkTransaction);
    }
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

#include "SessionTask.moc"
