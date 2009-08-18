/***************************************************************************
 *   Copyright (C) 2008 by Daniel Nicoletti                                *
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
    KDE_EXPORT QString finished(PackageKit::Transaction::ExitStatus status);
    KDE_EXPORT QString error(PackageKit::Client::ErrorType error);
    KDE_EXPORT QString errorMessage(PackageKit::Client::ErrorType error);
    KDE_EXPORT QString message(PackageKit::Client::MessageType type);
    KDE_EXPORT QString status(PackageKit::Transaction::Status status);
    KDE_EXPORT QString statusPast(PackageKit::Transaction::Status status);
    KDE_EXPORT QString groups(Client::Group group);
    KDE_EXPORT QString info(Package::State state);
    KDE_EXPORT QString infoUpdate(Package::State state, int number);
    KDE_EXPORT QString infoUpdate(Package::State state, int updates, int selected);
    KDE_EXPORT QString updateState(Client::UpdateState value);
    KDE_EXPORT QString restartType(Client::RestartType value);
    KDE_EXPORT QString restartTypeFuture(Client::RestartType value);
    KDE_EXPORT QString action(Client::Action action);
    KDE_EXPORT QString actionPast(Client::Action action);
    KDE_EXPORT QString mediaMessage(Transaction::MediaType value, const QString &text);
    KDE_EXPORT QString daemonError(PackageKit::Client::DaemonError value);
};

#endif
