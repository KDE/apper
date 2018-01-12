/***************************************************************************
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

#ifndef LICENSE_AGREEMENT_H
#define LICENSE_AGREEMENT_H

#include <QDialog>

namespace Ui {
    class LicenseAgreement;
}

class LicenseAgreement : public QDialog
{
    Q_OBJECT
public:
    explicit LicenseAgreement(const QString &eulaID, const QString &packageID, const QString &vendor, const QString &licenseAgreement, QWidget *parent = 0);
    ~LicenseAgreement();

    QString id() const;

private:
    QString m_id;
    Ui::LicenseAgreement *ui;
};

#endif
