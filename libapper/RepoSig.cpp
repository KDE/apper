/***************************************************************************
 *   Copyright (C) 2008-2018 by Daniel Nicoletti                           *
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

#include <KLocalizedString>

#include <QLoggingCategory>

#include "PkStrings.h"

using namespace PackageKit;

RepoSig::RepoSig(const QString &packageID,
                 const QString &repoName,
                 const QString &keyUrl,
                 const QString &keyUserid,
                 const QString &keyId,
                 const QString &keyFingerprint,
                 const QString &keyTimestamp,
                 PackageKit::Transaction::SigType type,
                 QWidget *parent) :
    QDialog(parent),
    m_sigType(type),
    m_keyID(keyId),
    m_packageID(packageID),
    ui(new Ui::RepoSig)
{
    Q_UNUSED(keyFingerprint)
    Q_UNUSED(keyTimestamp)
    ui->setupUi(this);

    ui->repoNameL->setText(repoName);
    ui->sigUrlL->setText(keyUrl);
    ui->sigUserIdL->setText(keyUserid);
    ui->sigIdL->setText(keyId);
}

RepoSig::~RepoSig()
{
    delete ui;
}

Transaction::SigType RepoSig::sigType() const
{
    return m_sigType;
}

QString RepoSig::keyID() const
{
    return m_keyID;
}

QString RepoSig::packageID() const
{
    return m_packageID;
}

#include "moc_RepoSig.cpp"
