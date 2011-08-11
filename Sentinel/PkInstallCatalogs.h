/***************************************************************************
 *   Copyright (C) 2009-2011 by Daniel Nicoletti                           *
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

#ifndef PK_INSTALL_CATALOGS_H
#define PK_INSTALL_CATALOGS_H

#include "SessionTask.h"

#include <Package>
#include <QDBusMessage>

using namespace PackageKit;

class PkTransaction;
class PkInstallCatalogs : public SessionTask
{
Q_OBJECT
public:
    PkInstallCatalogs(uint xid,
                      const QStringList &files,
                      const QString &interaction,
                      const QDBusMessage &message,
                      QWidget *parent = 0);
    ~PkInstallCatalogs();

public slots:
    void start();

private slots:
    void addPackage(const PackageKit::Package &package);

private:
    bool installPackages(const QStringList &packages);
    bool installProvides(const QStringList &provides);
    bool installFiles(const QStringList &files);
    bool runTransaction(Transaction *trans);

    PkTransaction *m_trans;
    QList<Package> m_foundPackages;
    QStringList  m_files;
    QString      m_interaction;
    QDBusMessage m_message;
    QStringList  m_alreadyInstalled;
    int          m_maxResolve;
};

#endif
