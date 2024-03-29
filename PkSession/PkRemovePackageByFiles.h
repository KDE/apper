/***************************************************************************
 *   Copyright (C) 2009-2011 by Daniel Nicoletti                           *
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

#ifndef PK_REMOVE_PACKAGE_BY_FILES_H
#define PK_REMOVE_PACKAGE_BY_FILES_H

#include <SessionTask.h>

class IntroDialog;
class FilesModel;
class PkRemovePackageByFiles : public SessionTask
{
Q_OBJECT
public:
    PkRemovePackageByFiles(uint xid,
                           const QStringList &files,
                           const QString &interaction,
                           const QDBusMessage &message,
                           QWidget *parent = nullptr);
    ~PkRemovePackageByFiles() override;

protected:
    void search() override;
    void notFound() override;

protected Q_SLOTS:
    void searchFinished(PkTransaction::ExitStatus status) override;

private Q_SLOTS:
    void modelChanged();

private:
    QStringList m_files;
    IntroDialog *m_introDialog;
    FilesModel *m_model;
};

#endif
