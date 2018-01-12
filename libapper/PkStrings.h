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

//#include <kdemacros.h>

#include <Transaction>

class Q_DECL_EXPORT PkStrings : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    static QString infoPresent(PackageKit::Transaction::Info info);
    static QString infoPast(PackageKit::Transaction::Info info);
    static QString error(PackageKit::Transaction::Error error);
    static QString errorMessage(PackageKit::Transaction::Error error);
//    static QString message(PackageKit::Transaction::Message type);
    static QString status(PackageKit::Transaction::Status status, uint speed = 0, qulonglong downloadRemaining = 0);
    static QString statusPast(PackageKit::Transaction::Status status);
    static QString groups(PackageKit::Transaction::Group group);
    static QString info(int state);
    static QString packageQuantity(bool updates, int packages, int selected);
    static QString updateState(PackageKit::Transaction::UpdateState value);
    static QString restartType(PackageKit::Transaction::Restart value);
    static QString restartTypeFuture(PackageKit::Transaction::Restart value);
    static QString action(PackageKit::Transaction::Role role, PackageKit::Transaction::TransactionFlags flags);
    static QString actionPast(PackageKit::Transaction::Role action);
    static QString mediaMessage(PackageKit::Transaction::MediaType value, const QString &text);
    static QString daemonError(int value);
    static QString prettyFormatDuration(unsigned long mSec);
    static QString lastCacheRefreshTitle(uint lastTime);
    static QString lastCacheRefreshSubTitle(uint lastTime);
// the following are needed to be able to use the functions from QML (bug#344639)
    static QString status(int status, uint speed = 0, qulonglong downloadRemaining = 0);
    static QString action(int role, int flags);
};

#endif
