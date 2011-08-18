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

#include "RepoSig.h"
#include "ui_RepoSig.h"

#include <KLocale>

#include <KDebug>

#include <Signature>

#include "PkStrings.h"

RepoSig::RepoSig(const Signature &info, QWidget *parent) :
    KDialog(parent),
    m_info(info),
    ui(new Ui::RepoSig)
{
    ui->setupUi(mainWidget());

    setButtons(KDialog::Cancel | KDialog::Yes);
    setPlainCaption(i18n("Software signature is required"));

    ui->repoNameL->setText(info.repoId);
    ui->sigUrlL->setText(info.keyUrl);
    ui->sigUserIdL->setText(info.keyUserid);
    ui->sigIdL->setText(info.keyId);
}

RepoSig::~RepoSig()
{
    delete ui;
}

PackageKit::Signature RepoSig::signature() const
{
    return m_info;
}

#include "RepoSig.moc"
