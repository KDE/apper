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

#include <kdemacros.h>
#include <PkTransaction.h>
//#include "ReviewChanges.h"

#include <QDBusMessage>
#include <KDialog>

namespace Ui {
    class SessionTask;
}

class SessionTask : public KDialog
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

    bool showConfirmSearch() const;
    bool showConfirmDeps() const;
    bool showConfirmInstall() const;
    bool showProgress() const;
    bool showFinished() const;
    bool showWarning() const;

    virtual void slotButtonClicked(int button);

    Interactions interactions() const;
    uint timeout() const;

//    ReviewChanges::OperationModes operationModes() const;
    void setMainWidget(QWidget *widget);
    QWidget* mainWidget();

    void setInfo(const QString &title, const QString &text);
    void setError(const QString &title, const QString &text);

    uint parentWId() const;

protected:
    // Virtual methods to easy subclasses
    virtual void search();
    virtual void commit();
    virtual void notFound();
    virtual void searchFailed();
    virtual void searchSuccess();

    bool foundPackages() const;
    int  foundPackagesSize() const;
    QList<Package> foundPackagesList() const;
    void finishTaskOk();
    void sendErrorFinished(DBusError error, const QString &msg);
    bool sendMessageFinished(const QDBusMessage &message);
    QString parentTitle;

protected slots:
    void setTitle(const QString &title);
    virtual void addPackage(const PackageKit::Package &package);
    virtual void searchFinished(PkTransaction::ExitStatus status);

private slots:
    void updatePallete();

private:
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
    QList<Package> m_foundPackages;
    Ui::SessionTask *ui;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SessionTask::Interactions)

#endif
