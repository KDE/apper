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
#define UNIVERSAL_PADDING 2
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
    m_checkedIcon("dialog-ok")
{
    // maybe rename or copy it to package-available
//     if (QApplication::isRightToLeft()) {
//         setExtendPixmap(SmallIcon("arrow-left"));
//     } else {
//         setExtendPixmap(SmallIcon("arrow-right"));
//     }
//     setContractPixmap(SmallIcon("arrow-down"));
    // store the size of the extend pixmap to know how much we should move
    m_extendPixmapWidth = SmallIcon("arrow-right").size().width();

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

    if (index.column() == 0 || index.column() == 1) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    } else if (index.column() == 2) {
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
                p.setOpacity(opa / 2.5);
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
    } else if (index.column() == 3) {
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
        // Calculate the top of the button which is the item height - the button height size divided by 2
        // this give us a little value which is the top and bottom margin
        optBt.rect.setTop(optBt.rect.top() + ((calcItemHeight(option) - m_buttonSize.height()) / 2));
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



    bool leftToRight = (painter->layoutDirection() == Qt::LeftToRight);

    QStyleOptionViewItemV4 opt(option);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    painter->save();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
    painter->restore();

    //grab the package from the index pointer
    QString pkgName       = index.data(KpkPackageModel::NameRole).toString();
    QString pkgSummary    = index.data(KpkPackageModel::SummaryRole).toString();
    QString pkgVersion    = index.data(KpkPackageModel::VersionRole).toString();
    QString pkgArch       = index.data(KpkPackageModel::ArchRole).toString();
    QString pkgIconPath   = index.data(KpkPackageModel::IconPathRole).toString();
    bool    pkgChecked    = index.data(KpkPackageModel::CheckStateRole).toBool();
    bool    pkgCheckable  = !index.data(Qt::CheckStateRole).isNull();
    Enum::Info info;
    info = static_cast<Enum::Info>(index.data(KpkPackageModel::InfoRole).toUInt());
    bool    pkgInstalled  = (info == Enum::InfoInstalled ||
                             info == Enum::InfoCollectionInstalled);

    bool    pkgCollection = (info == Enum::InfoCollectionInstalled ||
                             info == Enum::InfoCollectionAvailable);

    QIcon emblemIcon;
    if (pkgCheckable) {
        // update kind icon
        emblemIcon = index.data(KpkPackageModel::IconRole).value<QIcon>();
    } else {
        emblemIcon = m_checkedIcon;
    }

    // pain the background (checkbox and the extender)
//     if (m_extendPixmapWidth) {
//         KExtendableItemDelegate::paint(painter, opt, index);
//     }

    int leftCount;
    if (leftToRight) {
        opt.rect.setLeft(option.rect.left() + m_extendPixmapWidth + UNIVERSAL_PADDING);
        leftCount = opt.rect.left() + UNIVERSAL_PADDING;
    } else {
        opt.rect.setRight(option.rect.right() - m_extendPixmapWidth - UNIVERSAL_PADDING);
        leftCount = opt.rect.width() - (UNIVERSAL_PADDING + MAIN_ICON_SIZE);
    }

    int left = opt.rect.left();
    int top = opt.rect.top();
    int width = opt.rect.width();

    QStyleOptionButton optBt;
    optBt.rect = opt.rect;
    if (pkgCheckable) {
        optBt.rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &optBt);
        // Count the checkbox size
        if (leftToRight) {
            leftCount += optBt.rect.width();
        } else {
            leftCount -= optBt.rect.width();
        }
    } else  if ((option.state & QStyle::State_MouseOver) ||
                (option.state & QStyle::State_Selected)) {
        if (leftToRight) {
            optBt.rect.setLeft(left + width - (m_buttonSize.width() + UNIVERSAL_PADDING));
            width -= m_buttonSize.width() + UNIVERSAL_PADDING;
        } else {
            optBt.rect.setLeft(left + UNIVERSAL_PADDING);
            left += m_buttonSize.width() + UNIVERSAL_PADDING;
        }
        // Calculate the top of the button which is the item height - the button height size divided by 2
        // this give us a little value which is the top and bottom margin
        optBt.rect.setTop(optBt.rect.top() + ((calcItemHeight(option) - m_buttonSize.height()) / 2));
        optBt.rect.setSize(m_buttonSize); // the width and height sizes of the button
        if (option.state & QStyle::State_MouseOver) {
//                 if ()
//             QAbstractItemView *view = qobject_cast<QAbstractItemView*>(parent());
//             QPoint pos = view->viewport()->mapFromGlobal(QCursor::pos());
//             kDebug() << optBt.rect << pos << insideButton(optBt.rect, pos);
//             insideCheckBox(rect, pos);
            optBt.state |= QStyle::State_MouseOver;
        }

        optBt.state |= QStyle::State_Raised | QStyle::State_Active | QStyle::State_Enabled;
        optBt.iconSize = m_buttonIconSize;
        if (pkgChecked) {
            optBt.text = m_undoString;
            optBt.icon = m_undoIcon;
        } else {
            optBt.icon = pkgInstalled ? m_removeIcon   : m_installIcon;
            optBt.text = pkgInstalled ? m_removeString : m_installString;
        }
        qreal opa = painter->opacity();
        if ((option.state & QStyle::State_MouseOver) && !(option.state & QStyle::State_Selected)) {
            painter->setOpacity(opa / 2);
        }
        style->drawControl(QStyle::CE_PushButton, &optBt, painter);
        painter->setOpacity(opa);
    }

// QAbstractItemView *view = qobject_cast<QAbstractItemView*>(parent());
//             QPoint pos = view->viewport()->mapFromGlobal(QCursor::pos());
//     kDebug() << pos;


    // selects the mode to paint the icon based on the info field
    QIcon::Mode iconMode = QIcon::Normal;
    if (option.state & QStyle::State_MouseOver) {
        iconMode = QIcon::Active;
    }

    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected))?
    option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);

    // Painting main column
    QStyleOptionViewItem local_option_title(option);
    QStyleOptionViewItem local_option_normal(option);

    local_option_normal.font.setPointSize(local_option_normal.font.pointSize() - 1);

    QPixmap pixmap(option.rect.size());
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.translate(-option.rect.topLeft());

    // Main icon
    QIcon icon;
    if (pkgIconPath.isEmpty()) {
        icon = pkgCollection ? m_collectionIcon : m_packageIcon;
    } else {
        icon = KpkIcons::getIcon(pkgIconPath, "package");
    }

    int iconSize = calcItemHeight(option) - 2 * UNIVERSAL_PADDING;
    icon.paint(&p,
               leftCount,
               top + UNIVERSAL_PADDING,
               iconSize,
               iconSize,
               Qt::AlignCenter,
               iconMode);

    int textWidth;
    if (leftToRight) {
        // add the main icon
        leftCount += iconSize + UNIVERSAL_PADDING;
        textWidth = width - (leftCount - left);
    } else {
        leftCount -= UNIVERSAL_PADDING;
        textWidth = leftCount - left;
        leftCount = left;
    }


    // Painting

    // Text
    const int itemHeight = calcItemHeight(option);

    p.setPen(foregroundColor);
    // compose the top line
    // Collections does not have version and arch
    if (option.state & QStyle::State_MouseOver && !pkgCollection) {
        pkgName = pkgName + " - " + pkgVersion + (pkgArch.isNull() ? NULL : " (" + pkgArch + ')');
    }
    // draw the top line
    int topTextHeight = QFontInfo(local_option_title.font).pixelSize();
    p.setFont(local_option_title.font);
    p.drawText(leftCount,
               top,
               textWidth,
               topTextHeight + UNIVERSAL_PADDING,
               Qt::AlignVCenter | Qt::AlignLeft,
               pkgName);

    // draw the bottom line
    iconSize = topTextHeight + UNIVERSAL_PADDING;
    if (pkgCheckable || pkgInstalled) {
        emblemIcon.paint(&p,
                         leftToRight ? leftCount : (textWidth + left) - iconSize,
                         top + topTextHeight + UNIVERSAL_PADDING,
                         iconSize,
                         iconSize,
                         Qt::AlignVCenter | Qt::AlignHCenter,
                         iconMode);
    }

    // store the original opacity
    qreal opa = p.opacity();
    if (!(option.state & QStyle::State_MouseOver) && !(option.state & QStyle::State_Selected)) {
        p.setOpacity(opa / 2.5);
    }

    p.setFont(local_option_normal.font);
    p.drawText(leftToRight ? leftCount + iconSize + UNIVERSAL_PADDING : left - UNIVERSAL_PADDING,
               top + itemHeight / 2,
               textWidth - iconSize,
               QFontInfo(local_option_normal.font).pixelSize() + UNIVERSAL_PADDING,
               Qt::AlignTop | Qt::AlignLeft,
               pkgSummary);
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
}

int ApplicationsDelegate::calcItemHeight(const QStyleOptionViewItem &option) const
{
//     kDebug() << (m_buttonSize.height() + 2 * UNIVERSAL_PADDING);
    return m_buttonSize.height() + 2 * UNIVERSAL_PADDING;
    // Painting main column
    QStyleOptionViewItem local_option_title(option);
    QStyleOptionViewItem local_option_normal(option);

    local_option_normal.font.setPointSize(local_option_normal.font.pointSize() - 1);

    int textHeight = QFontInfo(local_option_title.font).pixelSize() + QFontInfo(local_option_normal.font).pixelSize();
    return textHeight + 3 * UNIVERSAL_PADDING;
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
    if (index.column() == 2 &&
        event->type() == QEvent::MouseButtonPress) {
        model->setData(index,
                       !index.data(KpkPackageModel::CheckStateRole).toBool(),
                       Qt::CheckStateRole);
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void ApplicationsDelegate::setExtendPixmapWidth(int width)
{
    m_extendPixmapWidth = width;
}

void ApplicationsDelegate::setViewport(QWidget *viewport)
{
    m_viewport = viewport;
}

QSize ApplicationsDelegate::sizeHint(const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    if (index.column() == 2) {
        QSize size = m_buttonSize;
        size.rheight() += 2 * UNIVERSAL_PADDING;
        size.rwidth()  += 5 * UNIVERSAL_PADDING;
//         kDebug() << size;
        return size;
    } else {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        if (index.column() == 0) {
            size.rwidth() += 2 * UNIVERSAL_PADDING;
        }
        return size;
    }
    int width = (index.column() == 0) ? index.data(Qt::SizeHintRole).toSize().width() : FAV_ICON_SIZE + 2 * UNIVERSAL_PADDING;
    QSize ret(QStyledItemDelegate::sizeHint(option, index));
    // remove the default size of the index
    ret -= QStyledItemDelegate::sizeHint(option, index);

    ret.rheight() += calcItemHeight(option);
    if (index.column() == 0) {
//         ret.rwidth() = 300;
// kDebug() << QStyledItemDelegate::sizeHint(option, index);
        return QStyledItemDelegate::sizeHint(option, index);
    } else {
    ret.rwidth()  += width;

    }
    kDebug() << index << ret;

    return ret;
}

#include "ApplicationsDelegate.moc"
