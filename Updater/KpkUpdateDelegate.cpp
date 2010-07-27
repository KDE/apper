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

#include <cmath>

#include <QtCore/QtCore>

#include <KDebug>
#include <KIconLoader>
#include "KpkPackageModel.h"
#include "KpkIcons.h"

#define FAV_ICON_SIZE 24
#define EMBLEM_ICON_SIZE 8
#define UNIVERSAL_PADDING 4
#define FADE_LENGTH 16
#define MAIN_ICON_SIZE 32
#define DROPDOWN_PADDING 2
// #define DROPDOWN_SEPARATOR_HEIGHT 32

KpkUpdateDelegate::KpkUpdateDelegate(QAbstractItemView *parent)
: KExtendableItemDelegate(parent), m_addIcon("go-down"), m_removeIcon("edit-delete")
{
    m_pkgRemove = KpkIcons::getIcon("package-installed");
    // maybe rename or copy it to package-available
    m_pkgDownload = KpkIcons::getIcon("kpk-refresh-cache");
//     setExtendPixmap(SmallIcon("arrow-right"));
//     setContractPixmap(SmallIcon("arrow-down"));
}

void KpkUpdateDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    if (!index.isValid()) return;

    QStyleOptionViewItemV4 opt(option);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
    KExtendableItemDelegate::paint(painter, opt, index);

    int left = option.rect.left();
    int leftCount;
    int top = option.rect.top();
    int width = option.rect.width();

    bool leftToRight = (painter->layoutDirection() == Qt::LeftToRight);

    //grab the package from the index pointer
    PackageKit::Package *pkg = static_cast<PackageKit::Package*>(index.internalPointer());

    QStyleOptionButton optBt;

    switch (index.data(KpkPackageModel::CheckedRole).toInt()) {
    case Qt::Unchecked :
        optBt.state |= QStyle::State_Off;
        break;
    case Qt::PartiallyChecked :
        optBt.state |= QStyle::State_NoChange;
        break;
    case Qt::Checked :
        optBt.state |= QStyle::State_On;
        break;
    }

    if (option.state & QStyle::State_MouseOver) {
        optBt.state |= QStyle::State_MouseOver;
    }

    optBt.state |= QStyle::State_Enabled;
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
    QIcon::Mode iconMode = QIcon::Normal;;
//     if (index.data(KpkPackageModel::CheckedRole).toInt() == Qt::Checked) {
//         iconMode = QIcon::Selected;
// //     } else {
//         iconMode = QIcon::Active;
//     }

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
    // if we have a package that mean we
    // can try to get a better icon
    if (pkg) {
        if (pkg->iconPath().isEmpty()) {
            if (pkg->info() == Enum::InfoInstalled) {
                icon = m_pkgRemove;
            } else {
                icon = m_pkgDownload;
            }
        } else {
            if (pkg->info() == Enum::InfoInstalled) {
                icon = KpkIcons::getIcon(pkg->iconPath(), "package-installed");
            } else {
                icon = KpkIcons::getIcon(pkg->iconPath(), "kpk-refresh-cache");
            }
        }
    } else {
        icon = index.data(KpkPackageModel::IconRole).value<QIcon>();
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
    if (pkg) {
        QString pkgname;
        // compose the top line
        if (option.state & QStyle::State_MouseOver) {
            pkgname = pkg->name() + " - " + pkg->version() + (pkg->arch().isNull() ? NULL : " (" + pkg->arch() + ')');
        } else {
            pkgname = pkg->name();
        }
        // draw the top line
        p.setFont(local_option_title.font);
        p.drawText(leftCount,
                   top + UNIVERSAL_PADDING,
                   textWidth,
                   itemHeight / 2,
                   Qt::AlignBottom | Qt::AlignLeft,
                   pkgname);

        // store the original opacity
        qreal opa = p.opacity();
        if (!(option.state & QStyle::State_MouseOver) && !(option.state & QStyle::State_Selected)) {
            p.setOpacity(opa / 2.5);
        }

        int topTextHeight = QFontInfo(local_option_title.font).pixelSize();
        p.setFont(local_option_normal.font);
        p.drawText(leftToRight ? leftCount + 10 : left,
                   top + topTextHeight + 2 * UNIVERSAL_PADDING,
                   textWidth - 10,
                   itemHeight / 2,
                   Qt::AlignTop | Qt::AlignLeft,
                   pkg->summary());
        p.setOpacity(opa);
    }

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
    kDebug() << event->type();
    Q_UNUSED(option)
    if (!(index.flags() & Qt::ItemIsUserCheckable)) {
        return false;
    }

    if (event->type() == QEvent::MouseButtonPress && index.column() == 0) {
        kDebug() << QCursor::pos() << option.rect.left();
    }

//     if (event->type() == QEvent::MouseButtonPress && index.column() == 1) {
//         return model->setData(index,
//                               !index.data(KpkPackageModel::CheckedRole).toBool(), KpkPackageModel::CheckedRole);
//     else if ( event->type() == QEvent::KeyPress ) {
//
//     }  else {
//         return false;
        return KExtendableItemDelegate::editorEvent(event, model, option, index);
//     }
}

QSize KpkUpdateDelegate::sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index ) const
{
//     int width;
//     if (index.column() == 0) {
//         QStyleOptionViewItem local_option_title(option);
//         QStyleOptionViewItem local_option_normal(option);
//
//         local_option_title.font.setBold(true);
//         local_option_title.font.setPointSize(local_option_title.font.pointSize() + 2);
//         QFontMetrics title(local_option_title.font);
//         QFontMetrics normal(local_option_normal.font);
//         width = qMax(title.width(index.data(KpkPackageModel::NameRole).toString()), normal.width(index.data(KpkPackageModel::SummaryRole).toString())) + MAIN_ICON_SIZE + FADE_LENGTH;
//     } else {
//         width = FAV_ICON_SIZE;
//     }
    int width = (index.column() == 0) ? index.data(Qt::SizeHintRole).toSize().width() : FAV_ICON_SIZE + 2 * UNIVERSAL_PADDING;
    QSize ret(KExtendableItemDelegate::sizeHint(option, index));
    // remove the default size of the index
    ret -= QStyledItemDelegate::sizeHint(option, index);

    ret.rheight() += calcItemHeight(option);
    ret.rwidth()  += width;

    return ret;
}

int KpkUpdateDelegate::columnWidth (int column, int viewWidth) const
{
    if (column != 0) {
        return FAV_ICON_SIZE + 2 * UNIVERSAL_PADDING;
    } else return viewWidth - /*2 **/ columnWidth(1, viewWidth);
}

// void KpkUpdateDelegate::itemActivated(const QModelIndex &index)
// {
//     const TransferTreeModel * transferTreeModel = static_cast <const TransferTreeModel *> (index.model());
//
//     if(!transferTreeModel->isTransferGroup(index) && Settings::showExpandableTransferDetails() && index.column() == 0) {
//         if(!isExtended(index)) {
//             TransferHandler *handler = static_cast <TransferHandler *> (index.internalPointer());
//             QWidget *widget = getDetailsWidgetForTransfer(handler);
//
//             m_editingIndexes.append(index);
//             extendItem(widget, index);
//         }
//         else {
//             removeTransferObserver(index);
//
//             m_editingIndexes.removeAll(index);
//             contractItem(index);
//         }
//     }
// }

#include "KpkUpdateDelegate.moc"
