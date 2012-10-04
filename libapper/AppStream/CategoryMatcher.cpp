/***************************************************************************
 *   Copyright (C) 2012 by Daniel Nicoletti                                *
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

#include "CategoryMatcher.h"

CategoryMatcher::CategoryMatcher(Kind kind, const QString &term) :
    m_kind(kind),
    m_term(term)
{
}

CategoryMatcher::CategoryMatcher(const CategoryMatcher &other) :
    m_kind(other.m_kind),
    m_term(other.m_term),
    m_child(other.m_child)
{
}

CategoryMatcher::~CategoryMatcher()
{
}

CategoryMatcher &CategoryMatcher::operator =(const CategoryMatcher &other)
{
    m_kind = other.m_kind;
    m_term = other.m_term;
    m_child = other.m_child;
    return *this;
}

bool CategoryMatcher::match(const QStringList &categories) const
{
    if (categories.isEmpty()) {
        return false;
    }

    bool ret = false;
    switch (m_kind) {
    case Term:
        ret = categories.contains(m_term);
        break;
    case And:
        foreach (const CategoryMatcher &parser, m_child) {
            if (!(ret = parser.match(categories))) {
                break;
            }
        }
        break;
    case Or:
        foreach (const CategoryMatcher &parser, m_child) {
            if ((ret = parser.match(categories))) {
                break;
            }
        }
        break;
    case Not:
        // We match like And but negating
        foreach (const CategoryMatcher &parser, m_child) {
            if (!(ret = !parser.match(categories))) {
                break;
            }
        }
        break;
    }
    return ret;
}

void CategoryMatcher::setChild(const QList<CategoryMatcher> &child)
{
    m_child = child;
}

QList<CategoryMatcher> CategoryMatcher::child() const
{
    return m_child;
}

QString CategoryMatcher::term() const
{
    return m_term;
}

CategoryMatcher::Kind CategoryMatcher::kind() const
{
    return m_kind;
}
