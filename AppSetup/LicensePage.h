/* This file is part of Apper
 *
 * Copyright (C) 2012 Matthias Klumpp <matthias@tenstral.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef LICENSE_PAGE_H
#define LICENSE_PAGE_H

#include <QWidget>
#include <QAbstractItemModel>
#include <KIcon>

#include "SimplePage.h"

namespace Ui {
    class LicensePage;
}

class QShowEvent;

class LicensePage : public QWidget
{
    Q_OBJECT
public:
    explicit LicensePage(QWidget *parent = 0);
    ~LicensePage();

    void setTitle(const QString& title);
    void setDescription(const QString &description);
    void setLicenseText(const QString &licenseText);
    void reset();

signals:
    void licenseAccepted(bool accepted);

protected:
    void showEvent(QShowEvent *event);

private slots:
    void licenseStateChanged(bool accepted);

private:
    Ui::LicensePage *ui;
};

#endif
