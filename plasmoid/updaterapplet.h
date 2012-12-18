/***************************************************************************
 *   Copyright (C) 2007-2009 by Shawn Starr <shawn.starr@rogers.com>       *
 *   Copyright (C) 2012 by Luís Gabriel Lima <lampih@gmail.com>            *
 *   Copyright (C) 2012 by Daniel Nicoletti <dantti12@gmail.com>           *
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

#ifndef UPDATERAPPLET_H
#define UPDATERAPPLET_H

#include <Plasma/PopupApplet>

namespace Plasma
{
    class DeclarativeWidget;
}

class PackageModel;
class UpdaterApplet : public Plasma::PopupApplet
{
    Q_OBJECT
public:
    typedef enum{
        Normal,
        Important,
        Security
    } UpdateType;
    UpdaterApplet(QObject *parent, const QVariantList &args);
    ~UpdaterApplet();

    void init();
    QGraphicsWidget *graphicsWidget();

signals:
    void getUpdates();

protected Q_SLOTS:
    void updateIcon();
    void toolTipAboutToShow();
    void setActive(bool active);
    uint getTimeSinceLastRefresh();

protected:
    void constraintsEvent(Plasma::Constraints constraints);
    virtual void popupEvent(bool show);

private slots:
    bool registerService();

private:

    Plasma::DeclarativeWidget *m_declarativeWidget;
    PackageModel *m_updatesModel;
    bool m_initted;
};

K_EXPORT_PLASMA_APPLET(updater, UpdaterApplet)

#endif // UPDATERAPPLET_H
