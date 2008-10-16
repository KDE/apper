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

#ifndef KPKSMARTUPDATE_H
#define KPKSMARTUPDATE_H

#include <QPackageKit>
#include <KNotification>

using namespace PackageKit;

class KpkSmartUpdate : public QObject
{
Q_OBJECT
public:
    KpkSmartUpdate( QObject *parent=0 );
    ~KpkSmartUpdate();

    bool canClose() { return !(m_running); };
    void smartUpdate();

private slots:
    void getUpdatesFinished(PackageKit::Transaction::ExitStatus status, uint runtime);
    void package(PackageKit::Package *);
    void updatesFinished(PackageKit::Transaction::ExitStatus status, uint runtime);

signals:
    void autoUpdatesBeingInstalled(Transaction *);
    void showUpdates();

private:
    uint m_autoUpdateType;

    // updates notification
    QList<Package *> packages;
    bool m_running;
};

#endif
