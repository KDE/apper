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

#include "CustomProgressBar.h"

#include <KLocalizedString>
#include <KFormat>

CustomProgressBar::CustomProgressBar(QWidget *parent)
 : QProgressBar(parent), m_remaining(0)
{
}

CustomProgressBar::~CustomProgressBar()
{
}

QString CustomProgressBar::text() const
{
    if (m_remaining) {
        return i18n("%1 remaining", KFormat().formatSpelloutDuration(m_remaining * 1000));
    } else {
        return QProgressBar::text();
    }
}

void CustomProgressBar::setRemaining(uint remaining)
{
    m_remaining = remaining;
}

#include "moc_CustomProgressBar.cpp"
