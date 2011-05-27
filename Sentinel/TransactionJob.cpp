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

#include "TransactionJob.h"

#include <KpkStrings.h>
#include <KpkIcons.h>
#include <KpkEnum.h>

#include <KLocale>
#include <KGlobal>
#include <KNotification>

#include <KDebug>

Q_DECLARE_METATYPE(PackageKit::Transaction::Error)

TransactionJob::TransactionJob(Transaction *transaction, QObject *parent)
 : KJob(parent),
   m_transaction(transaction),
   m_status(Transaction::UnknownStatus),
   m_percentage(0)
{
    setCapabilities(Killable);
    m_title = KpkStrings::action(transaction->role());
    connect(transaction, SIGNAL(changed()), this, SLOT(updateJob()));
    connect(transaction, SIGNAL(finished(PackageKit::Transaction::Exit, uint)),
            this, SLOT(finished(PackageKit::Transaction::Exit)));
    connect(transaction, SIGNAL(destroyed()),
            this, SLOT(finished(PackageKit::Transaction::Exit)));
    connect(transaction, SIGNAL(package(const PackageKit::Package &)),
            this, SLOT(package(const PackageKit::Package &)));
    connect(transaction, SIGNAL(repoDetail(const QString &, const QString &, bool)),
            this, SLOT(repoDetail(const QString &, const QString &)));
    kDebug();
}

TransactionJob::~TransactionJob()
{
}

void TransactionJob::finished(PackageKit::Transaction::Exit exit)
{
    // emit the description so the Speed: xxx KiB/s
    // don't get confused to a destination URL
    emit description(this, m_title);
    if (exit == Transaction::ExitCancelled) {
        setError(KilledJobError);
    }
    emitResult();
}

void TransactionJob::package(const PackageKit::Package &package)
{
    if (!package.id().isEmpty()) {
        bool changed = false;
        if (package.info() == Package::InfoFinished) {
            changed = m_packages.removeOne(package.name());
        } else if (!m_packages.contains(package.name())) {
            m_packages << package.name();
            changed = true;
        }

        if (changed) {
            m_details = m_packages.join(QLatin1String(", "));
            emitDescription();
        }
    }
}

void TransactionJob::repoDetail(const QString &repoId, const QString &repoDescription)
{
    Q_UNUSED(repoId)
    QString first = KpkStrings::status(m_status);
    emit description(this, m_title, qMakePair(first, repoDescription));
}

void TransactionJob::emitDescription()
{
    QString details = m_details;
    if (details.isEmpty()) {
        details = "...";
    }

    QString first = KpkStrings::status(m_status);
    emit description(this, m_title, qMakePair(first, details));
}

void TransactionJob::updateJob()
{
    uint percentage = m_transaction->percentage();
    if (percentage <= 100) {
        emitPercent(percentage, 100);
    } else if (m_percentage != 0) {
        percentage = 0;
        emitPercent(0, 0);
    }
    m_percentage = percentage;

//     if (m_speed == 0) {
//         m_speed = m_transaction->speed();
//         emitSpeed(m_speed);
//     }

    // Status & Speed
    Transaction::Status status = m_transaction->status();
    if (m_status != status) {
        m_status = status;
        emit description(this, m_title);
        emitDescription();
    }
}

void TransactionJob::start()
{
    kDebug();
    m_details = m_transaction->lastPackage().name();
    updateJob();
}

bool TransactionJob::doKill()
{
    // emit the description so the Speed: xxx KiB/s
    // don't get confused to a destination URL
    emit description(this, m_title);
    if (m_transaction->allowCancel()) {
        m_transaction->cancel();
    }
    return false;
}

#include "TransactionJob.moc"
