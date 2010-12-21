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

#ifndef SCREEN_SHOT_VIEWER_H
#define SCREEN_SHOT_VIEWER_H


#include <KPixmapSequenceOverlayPainter>
#include <KJob>

#include <QScrollArea>
#include <QPixmap>

class ClickableLabel;
class ScreenShotViewer : public QScrollArea
{
Q_OBJECT
public:
    explicit ScreenShotViewer(const QString &url, QWidget *parent = 0);
    ~ScreenShotViewer();

private slots:
    void resultJob(KJob *);
    void fadeIn();

private:
    KPixmapSequenceOverlayPainter *m_busySeq;
    QPixmap         m_screenshot;
    ClickableLabel *m_screenshotL;
};

#endif
