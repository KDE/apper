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

#include "SimulateModel.h"

#include <KDebug>
#include <PkIcons.h>
#include <KLocale>

using namespace PackageKit;

SimulateModel::SimulateModel(QObject *parent, QList<Package> skipPackages)
: QAbstractTableModel(parent),
  m_skipPackages(skipPackages),
  m_currentInfo(Package::UnknownInfo)
{
//     setSortRole(Qt::DisplayRole);
}

QVariant SimulateModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() ||
        m_currentInfo == Package::UnknownInfo ||
        index.row() >= m_packages[m_currentInfo].size()) {
        return QVariant();
    }

    const Package &p = m_packages[m_currentInfo].at(index.row());
    switch(index.column()) {
    case 0:
        switch (role) {
        case Qt::DisplayRole:
            return p.name();
        case Qt::DecorationRole:
            return PkIcons::getIcon("package");
        case Qt::ToolTipRole:
            return p.summary();
        default:
            return QVariant();
        }
        break;
    case 1:
        if (role == Qt::DisplayRole) {
            return p.version();
        }
        break;
    }
    return QVariant();
}

Package::Info SimulateModel::currentInfo() const
{
    return m_currentInfo;
}

void SimulateModel::setCurrentInfo(Package::Info currentInfo)
{
    m_currentInfo = currentInfo;
    reset();
}

int SimulateModel::countInfo(Package::Info info)
{
    if (m_packages.contains(info)) {
        return m_packages[info].size();
    } else {
        return 0;
    }
}

void SimulateModel::addPackage(const PackageKit::Package &p)
{
    if (p.info() == Package::InfoFinished ||
        p.info() == Package::InfoCleanup) {
        return;
    }

    // These are packages that are going to be installed
    // store them to be resolved later to get the desktop files
    if (p.info() == Package::InfoInstalling &&
        !m_newPackages.contains(p.name())) {
        m_newPackages.append(p.name());
    }

    foreach (const Package &pkg, m_skipPackages) {
        if (pkg.id() == p.id()) {
            // found a package to skip
            return;
        }
    }

    if (m_currentInfo == Package::UnknownInfo) {
        m_currentInfo = p.info();
    }
    m_packages[p.info()].append(p);
}

int SimulateModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() && m_currentInfo == Package::UnknownInfo) {
        return 0;
    } else {
        return m_packages[m_currentInfo].size();
    }
}

int SimulateModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid() && m_currentInfo == Package::UnknownInfo) {
        return 0;
    } else {
        return 2;
    }
}

QVariant SimulateModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation);
    if (role == Qt::DisplayRole) {
        switch(section) {
            case 0:
                return QVariant(i18n("Package"));
            case 1:
                return QVariant(i18n("Version"));
        }
    }
    return QVariant();
}

QStringList SimulateModel::newPackages() const
{
    return m_newPackages;
}

void SimulateModel::clear()
{
    m_packages.clear();
    m_currentInfo = Package::UnknownInfo;
    reset();
}
