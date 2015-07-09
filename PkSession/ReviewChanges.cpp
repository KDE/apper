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

#include "ReviewChanges.h"
#include "ui_ReviewChanges.h"

#include <PackageModel.h>
#include <ChangesDelegate.h>
#include <KLocalizedString>

#include <KCategorizedSortFilterProxyModel>

#include <KDebug>

ReviewChanges::ReviewChanges(PackageModel *model, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReviewChanges),
    m_model(model)
{
    ui->setupUi(this);


    //initialize the model, delegate, client and  connect it's signals
    ui->packageView->viewport()->setAttribute(Qt::WA_Hover);
    KCategorizedSortFilterProxyModel *changedProxy = new KCategorizedSortFilterProxyModel(this);
    changedProxy->setSourceModel(m_model);
    changedProxy->setCategorizedModel(true);
    changedProxy->sort(0);
    changedProxy->setDynamicSortFilter(true);
    changedProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    changedProxy->setSortRole(PackageModel::SortRole);
    ui->packageView->setModel(changedProxy);

    setWindowTitle(i18np("The following package was found",
                         "The following packages were found",
                         m_model->rowCount()));

    ChangesDelegate *delegate = new ChangesDelegate(ui->packageView);
    delegate->setExtendPixmapWidth(0);
    ui->packageView->setItemDelegate(delegate);

    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(selectionChanged()));
}

ReviewChanges::~ReviewChanges()
{
    delete ui;
}

PackageModel *ReviewChanges::model() const
{
    return m_model;
}

void ReviewChanges::selectionChanged()
{
    emit hasSelectedPackages(!m_model->selectedPackagesToInstall().isEmpty() ||
                             !m_model->selectedPackagesToRemove().isEmpty());
}

#include "ReviewChanges.moc"
