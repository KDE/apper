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

#include "IntroDialog.h"

#include "ui_IntroDialog.h"

#include <PkStrings.h>

#include <KLocalizedString>
#include <KMessageBox>

#include <QLoggingCategory>

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
}

IntroDialog::~IntroDialog()
{
    delete ui;
}

void IntroDialog::setModel(QAbstractItemModel *model)
{
    ui->listView->setModel(model);
    connect(model, &QAbstractItemModel::dataChanged, this, &IntroDialog::selectionChanged);
}

void IntroDialog::acceptDrops(const QString &toolTip)
{
    ui->listView->setDragDropMode(QListView::DropOnly);
    ui->listView->setToolTip(toolTip);
}

bool IntroDialog::canContinue() const
{
    return ui->listView->model()->rowCount();
}

void IntroDialog::selectionChanged()
{
    // if the model has more than one item it can continue
    emit continueChanged(canContinue());
}

void IntroDialog::setDescription(const QString &description)
{
    ui->descriptionL->setText(description);
}

#include "moc_IntroDialog.cpp"
