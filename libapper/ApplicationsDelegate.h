/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2008-2011 Daniel Nicoletti <dantti12@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef APPLICATIONS_DELEGATE_H
#define APPLICATIONS_DELEGATE_H

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QStyledItemDelegate>

#include <KIcon>

/**
 * Delegate for displaying the packages
 */
class KDE_EXPORT ApplicationsDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ApplicationsDelegate(QAbstractItemView *parent);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index);
    void setCheckable(bool checkable);

private:
    bool insideButton(const QRect &rect, const QPoint &pos) const;

    QWidget *m_viewport;
    KIcon    m_packageIcon;
    KIcon    m_installIcon;
    QString  m_installString;
    KIcon    m_removeIcon;
    QString  m_removeString;
    KIcon    m_undoIcon;
    QString  m_undoString;
    KIcon    m_checkedIcon;
    QSize    m_buttonSize;
    QSize    m_buttonIconSize;
    bool     m_checkable;
};

#endif
