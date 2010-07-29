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

// #include <cmath>

// #include <QtCore/QtCore>

#include <KDebug>
#include <KIconLoader>
#include "KpkUpdatePackageModel.h"
#include "KpkIcons.h"

#define FAV_ICON_SIZE 24
#define EMBLEM_ICON_SIZE 8
#define UNIVERSAL_PADDING 4
#define FADE_LENGTH 16
#define MAIN_ICON_SIZE 32

KpkUpdateDelegate::KpkUpdateDelegate(QAbstractItemView *parent)
  : KExtendableItemDelegate(parent)
{
    // loads it here to be faster when displaying items
    m_packageIcon = KIcon("package");
    // maybe rename or copy it to package-available
    if (QApplication::isRightToLeft()) {
        setExtendPixmap(SmallIcon("arrow-left"));
    } else {
        setExtendPixmap(SmallIcon("arrow-right"));
    }
    setContractPixmap(SmallIcon("arrow-down"));
    // store the size of the extend pixmap to know how much we should move
    m_extendPixmapWidth = SmallIcon("arrow-right").size().width();
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
    QString pkgName     = index.data(KpkUpdatePackageModel::NameRole).toString();
    QString pkgSummary  = index.data(KpkUpdatePackageModel::SummaryRole).toString();
    QString pkgVersion  = index.data(KpkUpdatePackageModel::VersionRole).toString();
    QString pkgArch     = index.data(KpkUpdatePackageModel::ArchRole).toString();
    QString pkgIconPath = index.data(KpkUpdatePackageModel::IconPathRole).toString();

    // update kind icon
    QIcon updateIcon = index.data(KpkUpdatePackageModel::IconRole).value<QIcon>();

    // pain the background (checkbox and the extender)
    KExtendableItemDelegate::paint(painter, opt, index);

    if (!leftToRight) {
        opt.rect.setRight(option.rect.right() - m_extendPixmapWidth);
    } else {
        opt.rect.setLeft(option.rect.left() + m_extendPixmapWidth);
    }

    int left = opt.rect.left();
    int leftCount;
    int top = opt.rect.top();
    int width = opt.rect.width();

    QStyleOptionButton optBt;
    optBt.rect = option.rect;
    if (leftToRight) {
        // Fisrt the left is increased by the universal pading
        leftCount = left + UNIVERSAL_PADDING;
        optBt.rect.setLeft(leftCount);
        // add the checkbox element to the left count
        leftCount += style->subElementRect(QStyle::SE_CheckBoxIndicator, &opt).width() + UNIVERSAL_PADDING;
    } else {
        leftCount = left + (width - UNIVERSAL_PADDING);
        optBt.rect.setRight(leftCount);
        // now remove the element from the left count
        leftCount -= style->subElementRect(QStyle::SE_CheckBoxIndicator, &opt).width() + UNIVERSAL_PADDING + MAIN_ICON_SIZE;
    }

    optBt.rect.setHeight(calcItemHeight(option));

    // draw item data as CheckBox
//     style->drawControl(QStyle::CE_CheckBox, &opt, painter);


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
    p.setFont(local_option_title.font);
    p.drawText(leftCount,
                top + UNIVERSAL_PADDING,
                textWidth,
                itemHeight / 2,
                Qt::AlignBottom | Qt::AlignLeft,
                pkgName);

    // draw the bottom line
    int topTextHeight = QFontInfo(local_option_title.font).pixelSize();

    iconSize = (calcItemHeight(option) - 2 * UNIVERSAL_PADDING)/2;
    updateIcon.paint(&p,
                     leftToRight ? leftCount : textWidth - iconSize,
                     top + topTextHeight + 2 * UNIVERSAL_PADDING,
                     iconSize,
                     iconSize,
                     Qt::AlignCenter,
                     iconMode);

    // store the original opacity
    qreal opa = p.opacity();
    if (!(option.state & QStyle::State_MouseOver) && !(option.state & QStyle::State_Selected)) {
        p.setOpacity(opa / 2.5);
    }

    p.setFont(local_option_normal.font);
    p.drawText(leftToRight ? leftCount + iconSize : left,
               top + topTextHeight + 2 * UNIVERSAL_PADDING,
               textWidth - iconSize,
               itemHeight / 2,
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

bool KpkUpdateDelegate::editorEvent(QEvent *event,
                                    QAbstractItemModel *model,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
{
    Q_UNUSED(option)
    if (event->type() == QEvent::MouseButtonPress) {
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
