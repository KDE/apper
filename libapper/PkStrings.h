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

#ifndef PK_STRINGS_H
#define PK_STRINGS_H

#include <kdemacros.h>

#include <Transaction>

using namespace PackageKit;

namespace PkStrings
{
    KDE_EXPORT QString finished(Transaction::Exit status);
    KDE_EXPORT QString infoPresent(Package::Info info);
    KDE_EXPORT QString infoPast(Package::Info info);
    KDE_EXPORT QString error(Transaction::Error error);
    KDE_EXPORT QString errorMessage(Transaction::Error error);
    KDE_EXPORT QString message(Transaction::Message type);
    KDE_EXPORT QString status(Transaction::Status status);
    KDE_EXPORT QString statusPast(Transaction::Status status);
    KDE_EXPORT QString groups(Package::Group group);
    KDE_EXPORT QString info(Package::Info state);
    KDE_EXPORT QString packageQuantity(bool updates, int packages, int selected);
    KDE_EXPORT QString updateState(Package::UpdateState value);
    KDE_EXPORT QString restartType(Package::Restart value);
    KDE_EXPORT QString restartTypeFuture(Package::Restart value);
    KDE_EXPORT QString action(Transaction::Role action);
    KDE_EXPORT QString actionPast(Transaction::Role action);
    KDE_EXPORT QString mediaMessage(Transaction::MediaType value, const QString &text);
    KDE_EXPORT QString daemonError(Transaction::InternalError value);
};

#endif
