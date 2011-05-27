/***************************************************************************
 *   Copyright (C) 2008-2010 by Daniel Nicoletti                           *
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

#include "IntroDialog.h"

#include "ui_IntroDialog.h"

#include <KpkSimulateModel.h>
#include <KpkRequirements.h>
#include <KpkStrings.h>
#include <KpkMacros.h>

#include <KLocale>
#include <KMessageBox>
#include <KMimeType>

#include <KDebug>

#include <QWeakPointer>
#include <QFileInfo>
#include <QCoreApplication>

#include <Daemon>

#include "FilesModel.h"

IntroDialog::IntroDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IntroDialog)
{
    ui->setupUi(this);
    ui->arrowL->setPixmap(KIcon("go-next-view").pixmap(32, 32));
    ui->iconL->setPixmap(KIcon("computer").pixmap(128, 128));
}

IntroDialog::~IntroDialog()
{
    delete ui;
}

void IntroDialog::setModel(QAbstractItemModel *model)
{
    ui->listView->setModel(model);
}

void IntroDialog::setTitle(const QString &title)
{
    if (title.isNull()) {
        ui->titleL->hide();
    } else {
        ui->titleL->show();
        ui->titleL->setText(title);
    }
}

void IntroDialog::setDescription(const QString &description)
{
    ui->descriptionL->setText(description);
}

#include "IntroDialog.moc"
