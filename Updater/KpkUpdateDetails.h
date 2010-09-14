/***************************************************************************
 *   Copyright (C) 2009 by Daniel Nicoletti                                *
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

#ifndef KPK_UPDATE_DETAILS_H
#define KPK_UPDATE_DETAILS_H

#include <QPackageKit>
#include <KPixmapSequenceOverlayPainter>

#include "ui_KpkUpdateDetails.h"

using namespace PackageKit;

class KpkUpdateDetails : public QWidget, Ui::KpkUpdateDetails
{
Q_OBJECT
public:
    explicit KpkUpdateDetails(QWidget *parent = 0);
    ~KpkUpdateDetails();

    void setPackage(const QString &packageId, Enum::Info updateInfo);
    void hide();

private slots:
    void updateDetail(PackageKit::Client::UpdateInfo info);
    void updateDetailFinished();
    void display();

private:
    QString getLinkList(const QString &links) const;

    bool m_show;
    QString m_packageId;
    QString m_currentDescription;
    Enum::Info m_updateInfo;
    KPixmapSequenceOverlayPainter *m_busySeq;
    QPropertyAnimation *m_fadeDetails;
    QParallelAnimationGroup *m_expandPanel;
};

#endif
