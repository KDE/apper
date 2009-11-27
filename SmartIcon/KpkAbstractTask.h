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

#ifndef KPK_ABSTRACT_TASK_H
#define KPK_ABSTRACT_TASK_H

#include <kdemacros.h>
#include <KpkReviewChanges.h>
#include <QDBusMessage>
#include <QWidget>

class KpkAbstractTask : public QWidget
{
Q_OBJECT
Q_ENUMS(Errors)
public:
    KpkAbstractTask(uint xid, const QString &interaction, const QDBusMessage &message, QWidget *parent = 0);
    ~KpkAbstractTask();

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

    Interactions interactions() const;
    uint timeout() const;

    KpkReviewChanges::OperationModes operationModes() const;

    void setParentWindow(QWidget *widget);
    void run();

signals:
    void finished();

private:
    uint m_xid;
    uint m_pid;
    QDBusMessage m_message;
    Interactions m_interactions;
    uint m_timeout;

    void parseInteraction(const QString &interaction);
    uint getPidSystem();
    uint getPidSession();
    QString getCmdLine(uint pid);
    bool pathIsTrusted(const QString &exec);
    void setExec(const QString &exec);

private slots:
    virtual void start();

protected:
    void finishTaskOk();
    void sendErrorFinished(DBusError error, const QString &msg);
    bool sendMessageFinished(const QDBusMessage &message);
    QString parentTitle;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KpkAbstractTask::Interactions)

#endif
