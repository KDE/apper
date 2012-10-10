/***************************************************************************
 *   Copyright (C) 2008-2012 by Daniel Nicoletti                           *
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

#ifndef PK_STRINGS_H
#define PK_STRINGS_H

#include <kdemacros.h>

#include <Transaction>

namespace PkStrings
{
    KDE_EXPORT QString finished(PackageKit::Transaction::Exit status);
    KDE_EXPORT QString infoPresent(PackageKit::Transaction::Info info);
    KDE_EXPORT QString infoPast(PackageKit::Transaction::Info info);
    KDE_EXPORT QString error(PackageKit::Transaction::Error error);
    KDE_EXPORT QString errorMessage(PackageKit::Transaction::Error error);
    KDE_EXPORT QString message(PackageKit::Transaction::Message type);
    KDE_EXPORT QString status(PackageKit::Transaction::Status status);
    KDE_EXPORT QString statusPast(PackageKit::Transaction::Status status);
    KDE_EXPORT QString groups(PackageKit::Transaction::Group group);
    KDE_EXPORT QString info(PackageKit::Transaction::Info state);
    KDE_EXPORT QString packageQuantity(bool updates, int packages, int selected);
    KDE_EXPORT QString updateState(PackageKit::Transaction::UpdateState value);
    KDE_EXPORT QString restartType(PackageKit::Transaction::Restart value);
    KDE_EXPORT QString restartTypeFuture(PackageKit::Transaction::Restart value);
    KDE_EXPORT QString action(PackageKit::Transaction::Role action);
    KDE_EXPORT QString actionPast(PackageKit::Transaction::Role action);
    KDE_EXPORT QString mediaMessage(PackageKit::Transaction::MediaType value, const QString &text);
    KDE_EXPORT QString daemonError(PackageKit::Transaction::InternalError value);
};

#endif
