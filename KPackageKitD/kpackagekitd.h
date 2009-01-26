/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef KPACKAGEKITD_H
#define KPACKAGEKITD_H

#include <KDEDModule>
#include <KDirWatch>
#include <QTimer>
#include <QPackageKit>

using namespace PackageKit;

class KPackageKitD : public KDEDModule
{
Q_OBJECT

public:
    KPackageKitD(QObject *parent, const QList<QVariant>&);
    ~KPackageKitD();

private slots:
    void init();
    void read();
    void checkUpdates();
    void finished(PackageKit::Transaction::ExitStatus status, uint);

    void transactionListChanged(const QList<PackageKit::Transaction*> &tids);

private:
    QTimer *m_qtimer;
    KDirWatch *m_confWatch;
    Client *m_client;
    Transaction *m_refreshCacheT;
    bool systemIsReady();
};

#endif
