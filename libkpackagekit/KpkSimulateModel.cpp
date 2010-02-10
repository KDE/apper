/***************************************************************************
 *   Copyright (C) 2009-2010 by Daniel Nicoletti                           *
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

#include "KpkSimulateModel.h"

#include <KDebug>
#include <KpkIcons.h>
#include <KLocale>

using namespace PackageKit;

KpkSimulateModel::KpkSimulateModel(QObject *parent)
: QAbstractTableModel(parent),
  m_currentInfo(Enum::UnknownInfo)
{
//     setSortRole(Qt::DisplayRole);
}

QVariant KpkSimulateModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() ||
        m_currentInfo == Enum::UnknownInfo ||
        index.row() >= m_packages[m_currentInfo].size()) {
        return QVariant();
    }

    Package *p = m_packages[m_currentInfo].at(index.row());
    switch(index.column()) {
    case 0:
        switch (role) {
        case Qt::DisplayRole:
            return p->name();
        case Qt::DecorationRole:
            return KpkIcons::getIcon("package");
        case Qt::ToolTipRole:
            return p->summary();
        default:
            return QVariant();
        }
        break;
    case 1:
        if (role == Qt::DisplayRole) {
            return p->version();
        }
        break;
    }
    return QVariant();
}

Enum::Info KpkSimulateModel::currentInfo() const
{
    return m_currentInfo;
}

void KpkSimulateModel::setCurrentInfo(Enum::Info currentInfo)
{
    m_currentInfo = currentInfo;
    reset();
}

int KpkSimulateModel::countInfo(Enum::Info info)
{
    if (m_packages.contains(info)) {
        return m_packages[info].size();
    } else {
        return 0;
    }
}

void KpkSimulateModel::addPackage(PackageKit::Package *p)
{
    if (p->info() == Enum::InfoFinished) {
        return;
    }
    if (m_currentInfo == Enum::UnknownInfo) {
        m_currentInfo = p->info();
    }
    m_packages[p->info()].append(p);
}

int KpkSimulateModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() && m_currentInfo == Enum::UnknownInfo) {
        return 0;
    } else {
        return m_packages[m_currentInfo].size();
    }
}

int KpkSimulateModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid() && m_currentInfo == Enum::UnknownInfo) {
        return 0;
    } else {
        return 2;
    }
}

QVariant KpkSimulateModel::headerData(int section, Qt::Orientation orientation, int role) const
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

void KpkSimulateModel::clear()
{
    m_packages.clear();
    m_currentInfo = Enum::UnknownInfo;
    reset();
}
