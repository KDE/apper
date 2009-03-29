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

#ifndef KPKLICENSEAGREEMENT_H
#define KPKLICENSEAGREEMENT_H

#include <KDialog>

#include "ui_KpkLicenseAgreement.h"
#include <QPackageKit>

using namespace PackageKit;

class KDE_EXPORT KpkLicenseAgreement : public KDialog, Ui::KpkLicenseAgreement
{
    Q_OBJECT
public:
    explicit KpkLicenseAgreement( PackageKit::Client::EulaInfo info, bool modal = true, QWidget *parent=0);
    ~KpkLicenseAgreement();
};

#endif
