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

#ifndef KPK_STRINGS_H
#define KPK_STRINGS_H

#include <kdemacros.h>

#include <QPackageKit>

using namespace PackageKit;

namespace KpkStrings
{
    KDE_EXPORT QString finished(Enum::Exit status);
    KDE_EXPORT QString error(Enum::Error error);
    KDE_EXPORT QString errorMessage(Enum::Error error);
    KDE_EXPORT QString message(Enum::Message type);
    KDE_EXPORT QString status(Enum::Status status);
    KDE_EXPORT QString statusPast(Enum::Status status);
    KDE_EXPORT QString groups(Enum::Group group);
    KDE_EXPORT QString info(Enum::Info state);
    KDE_EXPORT QString infoUpdate(Enum::Info state, int number);
    KDE_EXPORT QString infoUpdate(Enum::Info state, int updates, int selected);
    KDE_EXPORT QString updateState(Enum::UpdateState value);
    KDE_EXPORT QString restartType(Enum::Restart value);
    KDE_EXPORT QString restartTypeFuture(Enum::Restart value);
    KDE_EXPORT QString action(Enum::Role action);
    KDE_EXPORT QString actionPast(Enum::Role action);
    KDE_EXPORT QString mediaMessage(Enum::MediaType value, const QString &text);
    KDE_EXPORT QString daemonError(Client::DaemonError value);
};

#endif
