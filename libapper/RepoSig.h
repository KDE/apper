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

#ifndef REPO_SIG_H
#define REPO_SIG_H

#include <KDialog>

#include <Transaction>

namespace Ui {
    class RepoSig;
}

class RepoSig : public KDialog
{
    Q_OBJECT
public:
    explicit RepoSig(const QString &packageID,
                     const QString &repoName,
                     const QString &keyUrl,
                     const QString &keyUserid,
                     const QString &keyId,
                     const QString &keyFingerprint,
                     const QString &keyTimestamp,
                     PackageKit::Transaction::SigType type,
                     QWidget *parent = 0);
    ~RepoSig();

    PackageKit::Transaction::SigType sigType() const;
    QString keyID() const;
    QString packageID() const;

private:
    PackageKit::Transaction::SigType m_sigType;
    QString m_keyID;
    QString m_packageID;
    Ui::RepoSig *ui;
};

#endif
