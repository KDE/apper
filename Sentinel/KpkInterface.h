/***************************************************************************
 *   Copyright (C) 2008 Daniel Nicoletti <dantti85-pk@yahoo.com.br>        *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef KPK_INTERFACE_H
#define KPK_INTERFACE_H

#include <QtDBus/QDBusContext>

#include <config.h>

#ifdef HAVE_DEBCONFKDE
#include <DebconfGui.h>
using namespace DebconfKde;
#endif //HAVE_DEBCONFKDE

class KpkInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ApperSentinel")
public:
    KpkInterface(QObject *parent = 0);
    ~KpkInterface();

    void RefreshCache();
    void RefreshAndUpdate();
    void Update();
    void SetupDebconfDialog(const QString &socket_path, uint xid_parent);

signals:
    void refreshAndUpdate(bool refresh);
    void refresh();

#ifdef HAVE_DEBCONFKDE
private slots:
    void debconfActivate();

private:
    QHash<QString, DebconfGui*> m_debconfGuis;
#endif
};


#endif
