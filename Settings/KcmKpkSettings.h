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

#ifndef KCMKPKSETTINGS_H
#define KCMKPKSETTINGS_H

#include <KCModule>
#include <QGridLayout>
#include "KpkSettings.h"

class KcmKpkSettings : public KCModule
{
Q_OBJECT

public:
    KcmKpkSettings(QWidget *parent, const QVariantList &args);

public slots:
    virtual void load() { emit(s_load()); };
    virtual void save() { emit(s_save()); };
    virtual void defaults() { emit(s_defaults()); };

signals:
    void s_load();
    void s_save();
    void s_defaults();

private:
    KpkSettings *view;
    QGridLayout *m_grid;
};

#endif
