/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *   Copyright (C) 2008 Daniel Nicoletti <dantti85-pk@yahoo.com.br>
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

#include "KpkPackageModel.h"
#include "KpkIcons.h"

#define FAV_ICON_SIZE 24
#define EMBLEM_ICON_SIZE 8
#define UNIVERSAL_PADDING 4
#define FADE_LENGTH 16
#define MAIN_ICON_SIZE 32

ApplicationsDelegate::ApplicationsDelegate(QAbstractItemView *parent)
  : QStyledItemDelegate(parent),
    m_viewport(parent->viewport()),
    // loads it here to be faster when displaying items
    m_packageIcon("package"),
    m_collectionIcon("package-orign"),
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

    if (index.column() == 0 || index.column() == 1 || index.column() == 2) {
        QStyleOptionViewItemV4 opt(option);
        if (opt.state & QStyle::State_HasFocus) {
            opt.state ^= QStyle::State_HasFocus;
        }
        QStyledItemDelegate::paint(painter, opt, index);
        return;
    } else if (index.column() == 3) {
        QPixmap pixmap(option.rect.size());
        pixmap.fill(Qt::transparent);
        QPainter p(&pixmap);
        p.translate(-option.rect.topLeft());

        bool leftToRight = (painter->layoutDirection() == Qt::LeftToRight);


        QStyleOptionViewItemV4 opt(option);
        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
        painter->save();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
        painter->restore();

        int left = opt.rect.left();
//         int top = opt.rect.top();
        int width = opt.rect.width();

        Enum::Info info;
        info = static_cast<Enum::Info>(index.data(KpkPackageModel::InfoRole).toUInt());
        QString pkgSummary    = index.data(KpkPackageModel::SummaryRole).toString();
        bool    pkgInstalled  = (info == Enum::InfoInstalled ||
                             info == Enum::InfoCollectionInstalled);

        // store the original opacity
        qreal opa = p.opacity();
        QStyleOptionViewItem local_option_normal(option);
        if (!pkgInstalled) {
            local_option_normal.font.setItalic(true);
        }

        QColor foregroundColor;
        if (option.state.testFlag(QStyle::State_Selected)) {
            foregroundColor = option.palette.color(QPalette::HighlightedText);
        } else {
            if (!pkgInstalled) {
                p.setOpacity(0.75);
            }
            foregroundColor = option.palette.color(QPalette::Text);
        }


        p.setFont(local_option_normal.font);
        p.setPen(foregroundColor);
        p.drawText(opt.rect, Qt::AlignVCenter | (leftToRight ? Qt::AlignLeft : Qt::AlignRight), pkgSummary);
//         p.drawText(leftToRight ? leftCount + iconSize + UNIVERSAL_PADDING : left - UNIVERSAL_PADDING,
//                 top + itemHeight / 2,
//                 textWidth - iconSize,
//                 QFontInfo(local_option_normal.font).pixelSize() + UNIVERSAL_PADDING,
//                 ,
//                 pkgSummary);
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

        QRect paintRect = option.rect;
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(paintRect, gradient);

        if (leftToRight) {
            gradient.setStart(left + width
                    - (UNIVERSAL_PADDING + EMBLEM_ICON_SIZE) - FADE_LENGTH, 0);
            gradient.setFinalStop(left + width
                    - (UNIVERSAL_PADDING + EMBLEM_ICON_SIZE), 0);
        } else {
            gradient.setStart(left + UNIVERSAL_PADDING
                    + (UNIVERSAL_PADDING + EMBLEM_ICON_SIZE), 0);
            gradient.setFinalStop(left + UNIVERSAL_PADDING
                    + (UNIVERSAL_PADDING + EMBLEM_ICON_SIZE) + FADE_LENGTH, 0);
        }
        paintRect.setHeight(UNIVERSAL_PADDING + MAIN_ICON_SIZE / 2);
        p.fillRect(paintRect, gradient);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.end();

        painter->drawPixmap(option.rect.topLeft(), pixmap);
        return;
    } else if (index.column() == 4) {
        bool    pkgChecked    = index.data(KpkPackageModel::CheckStateRole).toBool();

        if (!(option.state & QStyle::State_MouseOver) &&
            !(option.state & QStyle::State_Selected) &&
            !pkgChecked) {
            QStyleOptionViewItemV4 opt(option);
            QStyledItemDelegate::paint(painter, opt, index);
            return;
        }
        QStyleOptionViewItemV4 opt(option);
        QStyledItemDelegate::paint(painter, opt, index);
        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

        QStyleOptionButton optBt;
        optBt.rect = option.rect;

        Enum::Info info;
        info = static_cast<Enum::Info>(index.data(KpkPackageModel::InfoRole).toUInt());
        bool    pkgInstalled  = (info == Enum::InfoInstalled ||
                                info == Enum::InfoCollectionInstalled);

//         if (leftToRight) {
//             optBt.rect.setLeft(left + width - (m_buttonSize.width() + UNIVERSAL_PADDING));
//             width -= m_buttonSize.width() + UNIVERSAL_PADDING;
//         } else {
//             optBt.rect.setLeft(left + UNIVERSAL_PADDING);
//             left += m_buttonSize.width() + UNIVERSAL_PADDING;
//         }
        
        // Button height
        int btHeight = m_buttonSize.height() + 2 * UNIVERSAL_PADDING;
        // Calculate the top of the button which is the item height - the button height size divided by 2
        // this give us a little value which is the top and bottom margin
        optBt.rect.setTop(optBt.rect.top() + ((btHeight - m_buttonSize.height()) / 2));
        optBt.rect.setSize(m_buttonSize); // the width and height sizes of the button

        if (option.state & QStyle::State_Selected) {
            optBt.palette.setBrush(QPalette::ButtonText, optBt.palette.brush(QPalette::HighlightedText));
//                 QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected))?
//     option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);
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
    if (index.column() == 4 &&
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
                       !index.data(KpkPackageModel::CheckStateRole).toBool(),
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
    if (index.column() == 4) {
        size = m_buttonSize;
        size.rheight() += 2 * UNIVERSAL_PADDING;
        size.rwidth()  += 2 * UNIVERSAL_PADDING;
    } else {
        QFontMetrics metric = QFontMetrics(option.font);
        // Button size is always bigger than text (since it has text in it
        size.setHeight(m_buttonSize.height() + 2 * UNIVERSAL_PADDING);
        size.setWidth(metric.width(index.data().toString()));
        if (index.column() == 0) {
            if (m_checkable) {
                const QStyle *style = QApplication::style();
                QRect rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &option);
                // Adds the icon size AND the checkbox size
                // [ x ] (icon) Text
                size.rwidth() += 4 * UNIVERSAL_PADDING + 44 + rect.width();
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
