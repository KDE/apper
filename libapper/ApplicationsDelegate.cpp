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

#include "ApplicationsDelegate.h"

#include <KDebug>
#include <KIconLoader>
#include <KLocale>

#include <Package>

#include "PackageModel.h"
#include "PkIcons.h"

#define UNIVERSAL_PADDING 4
#define FADE_LENGTH 16

ApplicationsDelegate::ApplicationsDelegate(QAbstractItemView *parent)
  : QStyledItemDelegate(parent),
    m_viewport(parent->viewport()),
    // loads it here to be faster when displaying items
    m_installIcon("go-down"),
    m_installString(i18n("Install")),
    m_removeIcon("edit-delete"),
    m_removeString(i18n("Remove")),
    m_undoIcon("edit-undo"),
    m_undoString(i18n("Deselect")),
    m_checkedIcon("dialog-ok"),
    m_checkable(false)
{
    QPushButton button, button2;
    button.setText(m_installString);
    button.setIcon(m_installIcon);
    button2.setText(m_removeString);
    button2.setIcon(m_removeIcon);
    m_buttonSize = button.sizeHint();
    int width = qMax(button.sizeHint().width(), button2.sizeHint().width());
    button.setText(m_undoString);
    width = qMax(width, button2.sizeHint().width());
    m_buttonSize.setWidth(width);
    m_buttonIconSize = button.iconSize();
}

void ApplicationsDelegate::paint(QPainter *painter,
                        const QStyleOptionViewItem &option,
                        const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }

    // Button height
    int btHeight = m_buttonSize.height() + UNIVERSAL_PADDING;
    if (index.column() == PackageModel::VersionCol ||
        index.column() == PackageModel::ArchCol ||
        index.column() == PackageModel::SizeCol) {
        QStyleOptionViewItemV4 opt(option);
        if (opt.state & QStyle::State_HasFocus) {
            opt.state ^= QStyle::State_HasFocus;
        }
        painter->save();
        QStyledItemDelegate::paint(painter, opt, index);
        painter->restore();
        return;
    } else if (index.column() == PackageModel::NameCol) {
        bool leftToRight = (painter->layoutDirection() == Qt::LeftToRight);
        QStyleOptionViewItemV4 opt1(option);
        if (opt1.state & QStyle::State_HasFocus) {
            opt1.state ^= QStyle::State_HasFocus;
        }
        QSize size = QStyledItemDelegate::sizeHint(opt1, index);
        if (leftToRight) {
            opt1.rect.setRight(size.width());
        } else {
            opt1.rect.setLeft(option.rect.left() + (option.rect.width() - size.width()));
        }
        painter->save();
        QStyledItemDelegate::paint(painter, opt1, index);
        painter->restore();

        // a new option for the summary
        QStyleOptionViewItemV4 opt(option);
        if (leftToRight) {
            opt.rect.setLeft(size.width() + 1);
        } else {
            opt.rect.setRight(option.rect.left() + (option.rect.width() - size.width()) - 1);
        }

        QPixmap pixmap(opt.rect.size());
        pixmap.fill(Qt::transparent);
        QPainter p(&pixmap);
        p.translate(-opt.rect.topLeft());

        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
        opt.viewItemPosition = QStyleOptionViewItemV4::Middle;
        painter->save();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
        painter->restore();

        int left = opt.rect.left();
        int width = opt.rect.width();

        Package::Info info;
        info = static_cast<Package::Info>(index.data(PackageModel::InfoRole).toUInt());
        QString pkgSummary = index.data(PackageModel::SummaryRole).toString();

        if (!pkgSummary.isEmpty()) {
            if (leftToRight) {
                pkgSummary.prepend("- ");
            } else {
                pkgSummary.append(" -");
            }
        }

        // store the original opacity
        qreal opa = p.opacity();
        QStyleOptionViewItem local_option_normal(opt);
        QColor foregroundColor;

        p.setOpacity(0.75);
        if (opt.state.testFlag(QStyle::State_Selected)) {
            foregroundColor = opt.palette.color(QPalette::HighlightedText);
        } else {
            foregroundColor = opt.palette.color(QPalette::Text);
        }

        p.setFont(local_option_normal.font);
        p.setPen(foregroundColor);
        p.drawText(opt.rect, Qt::AlignVCenter | (leftToRight ? Qt::AlignLeft : Qt::AlignRight), pkgSummary);
        p.setOpacity(opa);

        QLinearGradient gradient;
        // Gradient part of the background - fading of the text at the end
        if (leftToRight) {
            gradient = QLinearGradient(left + width - UNIVERSAL_PADDING - FADE_LENGTH,
                                    0,
                                    left + width - UNIVERSAL_PADDING,
                                    0);
            gradient.setColorAt(0, Qt::white);
            gradient.setColorAt(1, Qt::transparent);
        } else {
            gradient = QLinearGradient(left + UNIVERSAL_PADDING,
                                    0,
                                    left + UNIVERSAL_PADDING + FADE_LENGTH,
                                    0);
            gradient.setColorAt(0, Qt::transparent);
            gradient.setColorAt(1, Qt::white);
        }

        QRect paintRect = opt.rect;
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(paintRect, gradient);

        if (leftToRight) {
            gradient.setStart(left + width
                    - (UNIVERSAL_PADDING) - FADE_LENGTH, 0);
            gradient.setFinalStop(left + width
                    - (UNIVERSAL_PADDING), 0);
        } else {
            gradient.setStart(left + UNIVERSAL_PADDING
                    + (UNIVERSAL_PADDING ), 0);
            gradient.setFinalStop(left + UNIVERSAL_PADDING
                    + (UNIVERSAL_PADDING) + FADE_LENGTH, 0);
        }
        paintRect.setHeight(btHeight);
        p.fillRect(paintRect, gradient);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.end();

        painter->drawPixmap(opt.rect.topLeft(), pixmap);
        return;
    } else if (index.column() == PackageModel::ActionCol) {
        bool pkgChecked = index.data(PackageModel::CheckStateRole).toBool();
        QStyleOptionViewItemV4 opt(option);
        if (opt.state & QStyle::State_HasFocus) {
            opt.state ^= QStyle::State_HasFocus;
        }

        // Do not draw if the line is not selected or the mouse is over it
        if (!(option.state & QStyle::State_MouseOver) &&
            !(option.state & QStyle::State_Selected) &&
            !pkgChecked) {
            QStyledItemDelegate::paint(painter, opt, index);
            return;
        }

        QStyledItemDelegate::paint(painter, opt, index);
        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

        QStyleOptionButton optBt;
        optBt.rect = option.rect;

        Package::Info info;
        info = static_cast<Package::Info>(index.data(PackageModel::InfoRole).toUInt());
        bool    pkgInstalled  = (info == Package::InfoInstalled ||
                                 info == Package::InfoCollectionInstalled);

        // Calculate the top of the button which is the item height - the button height size divided by 2
        // this give us a little value which is the top and bottom margin
        optBt.rect.setLeft(optBt.rect.left() + UNIVERSAL_PADDING / 2);
        optBt.rect.setTop(optBt.rect.top() + ((btHeight - m_buttonSize.height()) / 2));
        optBt.rect.setSize(m_buttonSize); // the width and height sizes of the button

        if (option.state & QStyle::State_Selected) {
            optBt.palette.setColor(QPalette::WindowText, option.palette.color(QPalette::HighlightedText));
        }

        optBt.features = QStyleOptionButton::Flat;
        optBt.iconSize = m_buttonIconSize;
        if (pkgChecked) {
            optBt.state |= QStyle::State_Sunken | QStyle::State_Active | QStyle::State_Enabled;;
        } else {
            if (option.state & QStyle::State_MouseOver) {
                optBt.state |= QStyle::State_MouseOver;
            }
            optBt.state |= QStyle::State_Raised | QStyle::State_Active | QStyle::State_Enabled;
        }
        optBt.icon = pkgInstalled ? m_removeIcon   : m_installIcon;
        optBt.text = pkgInstalled ? m_removeString : m_installString;

        style->drawControl(QStyle::CE_PushButton, &optBt, painter);
        return;
    }
}

bool ApplicationsDelegate::insideButton(const QRect &rect, const QPoint &pos) const
{
//     kDebug() << rect << pos;
    if ((pos.x() >= rect.x() && (pos.x() <= rect.x() + rect.width())) &&
        (pos.y() >= rect.y() && (pos.y() <= rect.y() + rect.height()))) {
        return true;
    }
    return false;
}

bool ApplicationsDelegate::editorEvent(QEvent *event,
                                       QAbstractItemModel *model,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index)
{
    bool setData = false;
    if (index.column() == PackageModel::ActionCol &&
        event->type() == QEvent::MouseButtonPress) {
        setData = true;
    }

    const QWidget *widget = 0;
    if (const QStyleOptionViewItemV4 *v4 = qstyleoption_cast<const QStyleOptionViewItemV4 *>(&option)) {
        widget = v4->widget;
    }

    QStyle *style = widget ? widget->style() : QApplication::style();

    // make sure that we have the right event type
    if ((event->type() == QEvent::MouseButtonRelease)
        || (event->type() == QEvent::MouseButtonDblClick)) {
        QStyleOptionViewItemV4 viewOpt(option);
        initStyleOption(&viewOpt, index);
        QRect checkRect = style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &viewOpt, widget);
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        if (me->button() != Qt::LeftButton || !checkRect.contains(me->pos()))
            return false;

        // eat the double click events inside the check rect
        if (event->type() == QEvent::MouseButtonDblClick)
            return true;

        setData = true;
    } else if (event->type() == QEvent::KeyPress) {
        if (static_cast<QKeyEvent*>(event)->key() == Qt::Key_Space
         || static_cast<QKeyEvent*>(event)->key() == Qt::Key_Select) {
            setData = true;
        }
    }

    if (setData) {
        return model->setData(index,
                       !index.data(PackageModel::CheckStateRole).toBool(),
                       Qt::CheckStateRole);
    }
    return false;
}

void ApplicationsDelegate::setCheckable(bool checkable)
{
    m_checkable = checkable;
}

QSize ApplicationsDelegate::sizeHint(const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    QSize size;
//     kDebug() << index;
    if (index.column() == PackageModel::ActionCol) {
        size = m_buttonSize;
        size.rheight() += UNIVERSAL_PADDING;
        size.rwidth()  += UNIVERSAL_PADDING;
    } else {
        QFontMetrics metric = QFontMetrics(option.font);
        // Button size is always bigger than text (since it has text in it
        size.setHeight(m_buttonSize.height() + UNIVERSAL_PADDING);
        size.setWidth(metric.width(index.data().toString()));
        if (index.column() == PackageModel::NameCol) {
            if (m_checkable) {
                const QStyle *style = QApplication::style();
                QRect rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option);
                // Adds the icon size AND the checkbox size
                // [ x ] (icon) Text
                size.rwidth() += 4 * UNIVERSAL_PADDING + 46 + rect.width();
//                return QStyledItemDelegate::sizeHint(option, index);
            } else {
                // Adds the icon size
                size.rwidth() += 3 * UNIVERSAL_PADDING + 44;
            }
        } else {
            size.rwidth() += 2 * UNIVERSAL_PADDING;
        }
    }
    return size;
}

#include "ApplicationsDelegate.moc"
