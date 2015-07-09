/***************************************************************************
 *   Copyright (C) 2008 by Trever Fischer                                  *
 *   wm161@wm161.net                                                       *
 *   Copyright (C) 2008-2011 by Daniel Nicoletti                           *
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

#ifndef UPDATER_H
#define UPDATER_H

#include <PkTransaction.h>

#include <QStringList>

using namespace PackageKit;

class Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater(QObject *parent = 0);
    ~Updater();

    void setConfig(const QVariantHash &configs);
    void setSystemReady();

public Q_SLOTS:
    void checkForUpdates(bool systemReady);

private Q_SLOTS:
    void packageToUpdate(PackageKit::Transaction::Info info, const QString &packageID, const QString &summary);
    void getUpdateFinished();
    void autoUpdatesFinished(PkTransaction::ExitStatus exit);
    void reviewUpdates();
    void installUpdates();
    void serviceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner);

private:
    void showUpdatesPopup();
    bool updatePackages(const QStringList &packages, bool downloadOnly, const QString &icon = QString(), const QString &msg = QString());

    bool m_hasAppletIconified;
    bool m_systemReady;
    Transaction *m_getUpdatesT;
    QStringList m_oldUpdateList;
    QStringList m_updateList;
    QStringList m_importantList;
    QStringList m_securityList;
    QVariantHash m_configs;
};

#endif
