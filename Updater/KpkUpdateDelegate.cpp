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

#include "KpkUpdateDelegate.h"

#include <KDebug>
#include <KIconLoader>
#include <KLocale>

#include "KpkUpdatePackageModel.h"
#include "KpkIcons.h"

#define FAV_ICON_SIZE 24
#define EMBLEM_ICON_SIZE 8
#define UNIVERSAL_PADDING 4
#define FADE_LENGTH 16
#define MAIN_ICON_SIZE 32

KpkUpdateDelegate::KpkUpdateDelegate(QAbstractItemView *parent)
  : KExtendableItemDelegate(parent),
    // loads it here to be faster when displaying items
    m_packageIcon("package"),
    m_installIcon("go-down"),
    m_removeIcon("edit-delete"),
    m_undoIcon("edit-undo")
{
    // maybe rename or copy it to package-available
    if (QApplication::isRightToLeft()) {
        setExtendPixmap(SmallIcon("arrow-left"));
    } else {
        setExtendPixmap(SmallIcon("arrow-right"));
    }
    setContractPixmap(SmallIcon("arrow-down"));
    // store the size of the extend pixmap to know how much we should move
    m_extendPixmapWidth = SmallIcon("arrow-right").size().width();

    QPushButton button, button2;
    button.setText(i18n("Install"));
    button.setIcon(m_installIcon);
    button2.setText(i18n("Remove"));
    button2.setIcon(m_removeIcon);
    m_buttonSize = button.sizeHint();
    int width = qMax(button.sizeHint().width(), button2.sizeHint().width());
    button.setText(i18n("Unmark"));
    width = qMax(width, button2.sizeHint().width());
    m_buttonSize.setWidth(width);
    m_buttonIconSize = button.iconSize();
}

void KpkUpdateDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    if (!index.isValid()) {
        return;
    }
    bool leftToRight = (painter->layoutDirection() == Qt::LeftToRight);

    QStyleOptionViewItemV4 opt(option);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    painter->save();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
    painter->restore();

    //grab the package from the index pointer
    QString pkgName      = index.data(KpkUpdatePackageModel::NameRole).toString();
    QString pkgSummary   = index.data(KpkUpdatePackageModel::SummaryRole).toString();
    QString pkgVersion   = index.data(KpkUpdatePackageModel::VersionRole).toString();
    QString pkgArch      = index.data(KpkUpdatePackageModel::ArchRole).toString();
    QString pkgIconPath  = index.data(KpkUpdatePackageModel::IconPathRole).toString();
    bool    pkgInstalled = index.data(KpkUpdatePackageModel::InstalledRole).toBool();
    bool    pkgChecked   = index.data(KpkUpdatePackageModel::CheckStateRole).toBool();
    bool    pkgCheckable = !index.data(Qt::CheckStateRole).isNull();

    // update kind icon
    QIcon updateIcon = index.data(KpkUpdatePackageModel::IconRole).value<QIcon>();

    // pain the background (checkbox and the extender)
    KExtendableItemDelegate::paint(painter, opt, index);

    int leftCount;
    if (leftToRight) {
        opt.rect.setLeft(option.rect.left() + m_extendPixmapWidth + UNIVERSAL_PADDING);
        leftCount = opt.rect.left();
    } else {
        opt.rect.setRight(option.rect.right() - m_extendPixmapWidth - UNIVERSAL_PADDING);
        leftCount = opt.rect.width();
    }

    int left = opt.rect.left();
    int top = opt.rect.top();
    int width = opt.rect.width();

    QStyleOptionButton optBt;
    optBt.rect = opt.rect;
    if (pkgCheckable) {
        optBt.rect = style->subElementRect(QStyle::SE_CheckBoxIndicator, &optBt);
    } else {
        if (!leftToRight) {
            optBt.rect.setLeft(leftCount - m_buttonSize.width());
        }
        // Calculate the top of the button which is the item height - the button height size divided by 2
        // this give us a little value which is the top and bottom margin
        optBt.rect.setTop(optBt.rect.top() + ((calcItemHeight(option) - m_buttonSize.height()) / 2));
        optBt.rect.setSize(m_buttonSize); // the width and height sizes of the button
        if (option.state & QStyle::State_MouseOver) {
//                 if ()
            QAbstractItemView *view = qobject_cast<QAbstractItemView*>(parent());
            QPoint pos = view->viewport()->mapFromGlobal(QCursor::pos());
            kDebug() << optBt.rect << pos << insideButton(optBt.rect, pos);
//             insideCheckBox(rect, pos);
            optBt.state |= QStyle::State_MouseOver;
        }
        optBt.state |= QStyle::State_Raised | QStyle::State_Active | QStyle::State_Enabled;
        optBt.iconSize = m_buttonIconSize;
        if (pkgChecked) {
            optBt.text = i18n("Unmark");
            optBt.icon = m_undoIcon;
        } else {
            optBt.icon = pkgInstalled ? m_installIcon : m_removeIcon;
            optBt.text = pkgInstalled ? i18n("Install") : i18n("Remove");
        }
        style->drawControl(QStyle::CE_PushButton, &optBt, painter);
    }

    // Count the button size
    if (leftToRight) {
        leftCount += optBt.rect.width() + UNIVERSAL_PADDING;
    } else {
        leftCount -= optBt.rect.width() + UNIVERSAL_PADDING + MAIN_ICON_SIZE;
    }

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
        icon = m_packageIcon;
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
    if (option.state & QStyle::State_MouseOver) {
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
    updateIcon.paint(&p,
                     leftToRight ? leftCount : textWidth - iconSize,
                     top + topTextHeight + UNIVERSAL_PADDING,
                     iconSize,
                     iconSize,
                     Qt::AlignVCenter | Qt::AlignHCenter,
                     iconMode);

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
    // Counting the number of emblems for this item
    int emblemCount = 1;

//     left = option.rect.left();
    // Gradient part of the background - fading of the text at the end
    if (leftToRight) {
        gradient = QLinearGradient(left + width - UNIVERSAL_PADDING - FADE_LENGTH, 0,
                left + width - UNIVERSAL_PADDING, 0);
        gradient.setColorAt(0, Qt::white);
        gradient.setColorAt(1, Qt::transparent);
    } else {
        gradient = QLinearGradient(left + UNIVERSAL_PADDING, 0,
                left + UNIVERSAL_PADDING + FADE_LENGTH, 0);
        gradient.setColorAt(0, Qt::transparent);
        gradient.setColorAt(1, Qt::white);
    }

    QRect paintRect = option.rect;
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(paintRect, gradient);

    if (leftToRight) {
        gradient.setStart(left + width
                - emblemCount * (UNIVERSAL_PADDING + EMBLEM_ICON_SIZE) - FADE_LENGTH, 0);
        gradient.setFinalStop(left + width
                - emblemCount * (UNIVERSAL_PADDING + EMBLEM_ICON_SIZE), 0);
    } else {
        gradient.setStart(left + UNIVERSAL_PADDING
                + emblemCount * (UNIVERSAL_PADDING + EMBLEM_ICON_SIZE), 0);
        gradient.setFinalStop(left + UNIVERSAL_PADDING
                + emblemCount * (UNIVERSAL_PADDING + EMBLEM_ICON_SIZE) + FADE_LENGTH, 0);
    }
    paintRect.setHeight(UNIVERSAL_PADDING + MAIN_ICON_SIZE / 2);
    p.fillRect(paintRect, gradient);

    // Emblems icons
    p.setCompositionMode(QPainter::CompositionMode_SourceOver);
    p.end();

    painter->drawPixmap(option.rect.topLeft(), pixmap);
}

int KpkUpdateDelegate::calcItemHeight(const QStyleOptionViewItem &option) const
{
    // Painting main column
    QStyleOptionViewItem local_option_title(option);
    QStyleOptionViewItem local_option_normal(option);

    local_option_normal.font.setPointSize(local_option_normal.font.pointSize() - 1);

    int textHeight = QFontInfo(local_option_title.font).pixelSize() + QFontInfo(local_option_normal.font).pixelSize();
    return textHeight + 3 * UNIVERSAL_PADDING;
}

bool KpkUpdateDelegate::insideButton(const QRect &rect, const QPoint &pos) const
{
//     kDebug() << rect << pos;
    if ((pos.x() >= rect.x() && (pos.x() <= rect.x() + rect.width())) &&
        (pos.y() >= rect.y() && (pos.y() <= rect.y() + rect.height()))) {
        return true;
    }
    return false;
}

bool KpkUpdateDelegate::editorEvent(QEvent *event,
                                    QAbstractItemModel *model,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
{
    Q_UNUSED(option)
    if (event->type() == QEvent::MouseButtonRelease) {
        QAbstractItemView *view = qobject_cast<QAbstractItemView*>(parent());
        QPoint point = view->viewport()->mapFromGlobal(QCursor::pos());
//         kDebug() << point << option.rect.left() << option;
//         kDebug() << view->visualRect(index);
        QRect rect = view->visualRect(index);
        if (QApplication::isRightToLeft()) {
            if ((rect.width() - point.x()) <= m_extendPixmapWidth) {
                emit showExtendItem(index);
            }
        } else {
            if (point.x() <= m_extendPixmapWidth) {
                emit showExtendItem(index);
            }
        }
    }

    // We need move the option rect left because KExtendableItemDelegate
    // drew the extendPixmap
    QStyleOptionViewItemV4 opt(option);
    if (QApplication::isRightToLeft()) {
        opt.rect.setRight(option.rect.right() - m_extendPixmapWidth);
    } else {
        opt.rect.setLeft(option.rect.left() + m_extendPixmapWidth);
    }
    // When the exterder is shown the height get compromised,
    // this makes sure the check box is always known
    opt.rect.setHeight(calcItemHeight(option));
    return KExtendableItemDelegate::editorEvent(event, model, opt, index);
}

QSize KpkUpdateDelegate::sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index ) const
{
    int width = (index.column() == 0) ? index.data(Qt::SizeHintRole).toSize().width() : FAV_ICON_SIZE + 2 * UNIVERSAL_PADDING;
    QSize ret(KExtendableItemDelegate::sizeHint(option, index));
    // remove the default size of the index
    ret -= QStyledItemDelegate::sizeHint(option, index);

    ret.rheight() += calcItemHeight(option);
    ret.rwidth()  += width;

    return ret;
}

#include "KpkUpdateDelegate.moc"
