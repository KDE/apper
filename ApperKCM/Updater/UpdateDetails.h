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

#ifndef UPDATE_DETAILS_H
#define UPDATE_DETAILS_H

#include <Package>

#include <KPixmapSequenceOverlayPainter>

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

#include "ui_UpdateDetails.h"

using namespace PackageKit;

namespace PackageKit {
    class Transaction;
}

class UpdateDetails : public QWidget, Ui::UpdateDetails
{
Q_OBJECT
public:
    explicit UpdateDetails(QWidget *parent = 0);
    ~UpdateDetails();

    void setPackage(const QString &packageId, Package::Info updateInfo);

public slots:
    void hide();

private slots:
    void updateDetail(const PackageKit::Package &package);
    void updateDetailFinished();
    void display();

private:
    QString getLinkList(const QString &links) const;

    bool m_show;
    QString m_packageId;
    Transaction *m_transaction;
    QString m_currentDescription;
    Package::Info m_updateInfo;
    KPixmapSequenceOverlayPainter *m_busySeq;
    QPropertyAnimation *m_fadeDetails;
    QParallelAnimationGroup *m_expandPanel;
};

#endif
