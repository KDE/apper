/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
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

#ifndef APPLICATION_LAUNCHER_H
#define APPLICATION_LAUNCHER_H

#include <KDialog>
#include <KService>
#include <Package>
#include <QModelIndex>

namespace Ui {
    class ApplicationLauncher;
}

class KDE_EXPORT ApplicationLauncher : public KDialog
{
    Q_OBJECT
    Q_PROPERTY(bool embedded READ embedded WRITE setEmbedded USER true)
public:
    explicit ApplicationLauncher(QWidget *parent = 0);
    ~ApplicationLauncher();

    bool embedded() const;
    void setEmbedded(bool embedded);

    QList<PackageKit::Package> packages() const;
    bool hasApplications();

public slots:
    void addPackage(const PackageKit::Package &package);
    void files(const PackageKit::Package &package, const QStringList &files);

private slots:
    void itemClicked(const QModelIndex &index);
    void on_showCB_toggled(bool checked);

private:
    bool m_embed;
    QStringList m_files;
    QList<PackageKit::Package> m_packages;
    Ui::ApplicationLauncher *ui;
};

#endif
