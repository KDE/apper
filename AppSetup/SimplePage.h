/* This file is part of Apper
 *
 * Copyright (C) 2012 Matthias Klumpp <matthias@tenstral.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef SIMPLE_PAGE_H
#define SIMPLE_PAGE_H

#include <QWidget>
#include <QAbstractItemModel>
#include <KIcon>

namespace Ui {
    class SimplePage;
}

class SimplePage : public QWidget
{
    Q_OBJECT
public:
    explicit SimplePage(QWidget *parent = 0);
    ~SimplePage();

    void setTitle(const QString& title);
    void setDescription(const QString &description);
    void setDetails(const QString &details, bool isHtml = true);
    void reset();

    void addWidget(QWidget *widget);

private:
    int m_pageId;

    Ui::SimplePage *ui;
};

#endif
