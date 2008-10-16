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

#ifndef KPKUI_H
#define KPKUI_H

#include <KCMultiDialog>
#include <KSystemTrayIcon>

#include <QPackageKit>

using namespace PackageKit;

namespace kpackagekit {

class KpkUi : public QObject
{
Q_OBJECT
public:
    KpkUi( QObject *parent=0 );
    ~KpkUi();

    bool canClose() { return !m_showingUi; };

signals:
    void appClose();

public slots:
    void showUi();
    void showUpdatesUi();
    void showUpdatesTrayIcon();

private slots:
    void closeUi();

private:
    KCMultiDialog *m_kcmMD;
    KPageWidgetItem *m_updatePWI;
    bool m_showingUi;
    bool m_showingOnlyUpdates;
    KSystemTrayIcon *m_updatesTI;
};

}

#endif
