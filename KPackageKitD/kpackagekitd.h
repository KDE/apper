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
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifndef KPACKAGEKITD_H
#define KPACKAGEKITD_H

#include <KDEDModule>

#include <QTimer>

class KPackageKitD : public KDEDModule
{
Q_OBJECT

public:
    KPackageKitD(QObject *parent, const QList<QVariant>&);
    ~KPackageKitD();

private slots:
    void init();
    void read();

    void transactionListChanged(const QStringList &tids);

private:
    void update();
    void refreshAndUpdate();
    bool systemIsReady();
    uint getTimeSinceRefreshCache() const;
    bool canRefreshCache();

    bool m_actRefreshCacheChecked;
    bool m_canRefreshCache;
    QTimer *m_qtimer;
};

#endif
