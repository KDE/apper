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

#include "LicensePage.h"
#include "ui_LicensePage.h"

#include <QGroupBox>
#include <QMouseEvent>

#include <KTextBrowser>
#include <KDebug>

LicensePage::LicensePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LicensePage)
{
    ui->setupUi(this);

    connect(ui->acceptedRB, SIGNAL(toggled(bool)), this, SLOT(licenseStateChanged(bool)));
}

LicensePage::~LicensePage()
{
    delete ui;
}

void LicensePage::setTitle(const QString& title)
{
    setWindowTitle(title);
}


void LicensePage::setDescription(const QString &description)
{
    ui->descriptionL->setText(description);
}

void LicensePage::setLicenseText(const QString &licenseText)
{
    ui->licenseTextB->setHtml(licenseText);
}

void LicensePage::reset()
{
    setTitle("");
    setDescription("");
    setLicenseText("");
    ui->acceptedRB->setChecked(false);
}

void LicensePage::licenseStateChanged(bool accepted)
{
    emit licenseAccepted(accepted);
}

void LicensePage::showEvent(QShowEvent *event)
{
    licenseStateChanged(ui->acceptedRB->isChecked());

    QWidget::showEvent(event);
}
