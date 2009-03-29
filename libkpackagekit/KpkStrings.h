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

#ifndef KPKSTRINGS_H
#define KPKSTRINGS_H

#include <QObject>
#include <KIcon>

#include <QPackageKit>

using namespace PackageKit;

class KDE_EXPORT KpkStrings : public QObject
{
    Q_OBJECT
public:
    KpkStrings( QObject *parent=0);
    ~KpkStrings();

    static QString finished(PackageKit::Transaction::ExitStatus status);
    static QString error(PackageKit::Client::ErrorType error);
    static QString errorMessage(PackageKit::Client::ErrorType error);
    static QString status(PackageKit::Transaction::Status status);
    static QString statusPast(PackageKit::Transaction::Status status);
    static QString groups(Client::Group group);
    static QString info(Package::State state);
    static QString infoUpdate(Package::State state, int number);
    static QString updateState(Client::UpgradeType value);
    static QString restartType(Client::RestartType value);
    static QString restartTypeFuture(Client::RestartType value);
    static QString action(Client::Action action);
    static QString actionPast(Client::Action action);
};

#endif
