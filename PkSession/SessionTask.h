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

#ifndef SESSION_TASK_H
#define SESSION_TASK_H

#include <Transaction>
#include <PkTransaction.h>

#include <QDBusMessage>
#include <QDialog>

namespace Ui {
    class SessionTask;
}

class PackageModel;
class ReviewChanges;
class PkTransactionWidget;
class SessionTask : public QDialog
{
    Q_OBJECT
    Q_ENUMS(Errors)
public:
    SessionTask(uint xid, const QString &interaction, const QDBusMessage &message, QWidget *parent = 0);
    ~SessionTask();

    typedef enum{
        Failed,
        InternalError,
        NoPackagesFound,
        Forbidden,
        Cancelled
    } DBusError;

    enum Interaction {
        ConfirmSearch   = 0x001,
        ConfirmDeps     = 0x002,
        ConfirmInstall  = 0x004,
        Progress        = 0x010,
        Finished        = 0x020,
        Warning         = 0x040,
        Unknown         = 0x100
    };
    Q_DECLARE_FLAGS(Interactions, Interaction)

    typedef struct {
        Transaction::Info info;
        QString packageID;
        QString summary;
    } IPackage;

    bool showConfirmSearch() const;
    bool showConfirmDeps() const;
    bool showConfirmInstall() const;
    bool showProgress() const;
    bool showFinished() const;
    bool showWarning() const;

    Interactions interactions() const;
    uint timeout() const;

    QWidget* mainWidget();
    uint parentWId() const;

    void slotContinueClicked();
    void slotCancelClicked();

Q_SIGNALS:
    void continueClicked();
    void cancelClicked();

public Q_SLOTS:
    void enableButtonOk(bool enable);
    void setMainWidget(QWidget *widget);

protected:
    // Virtual methods to easy subclasses
    virtual void search();
    virtual void commit();
    virtual void notFound();
    virtual void searchFailed();
    virtual void searchSuccess();
    virtual void commitFailed();
    virtual void commitSuccess(QWidget *widget = 0);

    void showCloseButton();
    bool foundPackages() const;
    int  foundPackagesSize() const;
    PackageModel* model() const;
    void setTransaction(Transaction::Role role, PkTransaction *transaction = 0);
    void finishTaskOk();
    void sendErrorFinished(DBusError error, const QString &msg);
    bool sendMessageFinished(const QDBusMessage &message);
    QString parentTitle;

protected Q_SLOTS:
    void setTitle(const QString &title);
    void setInfo(const QString &title, const QString &text, const QString &details = QString());
    void setError(const QString &title, const QString &text, const QString &details = QString());
    void setFinish(const QString &title, const QString &text, QWidget *widget = 0);
    virtual void addPackage(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary);
    virtual void searchFinished(PkTransaction::ExitStatus status);
    virtual void commitFinished(PkTransaction::ExitStatus status);

private Q_SLOTS:
    void updatePallete();
    void setDialog(QDialog *dialog);

private:
    void removePackages();
    void parseInteraction(const QString &interaction);
    uint getPidSystem();
    uint getPidSession();
    QString getCmdLine(uint pid);
    bool pathIsTrusted(const QString &exec);
    void setExec(const QString &exec);

    uint m_xid;
    uint m_pid;
    QDBusMessage m_message;
    Interactions m_interactions;
    uint m_timeout;
    PackageModel *m_model;
    QStringList m_removePackages;
    ReviewChanges *m_reviewChanges = nullptr;
    PkTransactionWidget *m_pkTransaction = nullptr;
    Ui::SessionTask *ui;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SessionTask::Interactions)

#endif
