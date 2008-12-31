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

#include "KpkDelegate.h"

#include <cmath>

#include <QtCore/QtCore>

#include <KDebug>
#include <KIconLoader>
#include "KpkPackageModel.h"

#define FAV_ICON_SIZE 24
#define EMBLEM_ICON_SIZE 16
#define UNIVERSAL_PADDING 6
#define FADE_LENGTH 32
#define MAIN_ICON_SIZE 48
#define DROPDOWN_PADDING 2
#define DROPDOWN_SEPARATOR_HEIGHT 32

KpkDelegate::KpkDelegate(QObject * parent)
 : QItemDelegate(parent), m_addIcon("list-add"), m_removeIcon("list-remove")
{

}

void KpkDelegate::paint(QPainter *painter,
        const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.isValid()) return;

    QStyleOptionViewItemV4 opt(option);
    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    switch (index.column()) {
	case 0:
	    paintColMain(painter, option, index);
	    break;
	case 1:
	    paintColFav(painter, option, index);
	    break;
	default:
	    kDebug() << "unexpected column";
    }
}

int KpkDelegate::calcItemHeight(const QStyleOptionViewItem &option) const
{
    // Painting main column
    QStyleOptionViewItem local_option_title(option);
    QStyleOptionViewItem local_option_normal(option);

    local_option_title.font.setBold(true);
    local_option_title.font.setPointSize(local_option_title.font.pointSize() + 2);

    int textHeight = QFontInfo(local_option_title.font).pixelSize() + QFontInfo(local_option_normal.font).pixelSize();
    return qMax(textHeight, MAIN_ICON_SIZE) + 2 * UNIVERSAL_PADDING;
}

void KpkDelegate::paintColMain(QPainter *painter,
        const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int left = option.rect.left();
    int top = option.rect.top();
    int width = option.rect.width();

    bool leftToRight = (painter->layoutDirection() == Qt::LeftToRight);

    //grab the info from the model
    QString title = index.model()->data(index, Qt::DisplayRole).toString();
    QString description = index.model()->data(index, SummaryRole).toString();
    bool group = index.model()->data(index, GroupRole).toBool();

    // selects the mode to paint the icon based on the info field
    QIcon::Mode iconMode;
    if (group)
	iconMode = QIcon::Normal;
    else {
	if ( index.model()->data(index, InstalledRole).toBool() )
	    iconMode = index.model()->data(index, CheckedRole).toInt() == Qt::Checked ? QIcon::Disabled : QIcon::Normal;
	else
	    iconMode = index.model()->data(index, CheckedRole).toInt() == Qt::Checked ? QIcon::Selected : QIcon::Disabled;
    }
	
// 	    QIcon::Mode iconMode = QIcon::Normal;
//     switch ( index.model()->data(index, CheckedRole).toInt() ) {
// 	case Qt::Unchecked :
// 	    iconMode = QIcon::Disabled;
// 	    break;
// 	case Qt::PartiallyChecked :
// 	    iconMode = QIcon::Selected;
// 	    break;
// 	case Qt::Checked :
// 	    iconMode = QIcon::Active;
// 	    break;
//     }


    QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected))?
        option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);

    // Painting main column
    QStyleOptionViewItem local_option_title(option);
    QStyleOptionViewItem local_option_normal(option);

    local_option_title.font.setBold(true);
    local_option_title.font.setPointSize(local_option_title.font.pointSize() + 2);

    QPixmap pixmap(option.rect.size());
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.translate(-option.rect.topLeft());

    QLinearGradient gradient;

    // Painting

    // Text
    int textInner = 2 * UNIVERSAL_PADDING + MAIN_ICON_SIZE;
    const int itemHeight = calcItemHeight(option);

    p.setPen(foregroundColor);
    if (group) {
	p.setFont(local_option_title.font);
	p.drawText(
		left + (leftToRight ? textInner : 0),
		top,
		width - textInner, itemHeight,
		Qt::AlignVCenter | Qt::AlignLeft, title);
    }
    else {
	p.setFont(local_option_title.font);
	p.drawText(
		left + (leftToRight ? textInner : 0),
		top,
		width - textInner, itemHeight / 2,
		Qt::AlignBottom | Qt::AlignLeft, title);
	p.setFont(local_option_normal.font);
	p.drawText(
		left + (leftToRight ? textInner : 0),
		top + itemHeight / 2,
		width - textInner, itemHeight / 2,
		Qt::AlignTop | Qt::AlignLeft, description);
    }

    // Main icon
    index.model()->data(index, Qt::DecorationRole).value<QIcon>().paint(&p, 
            leftToRight ? left + UNIVERSAL_PADDING : left + width - UNIVERSAL_PADDING - MAIN_ICON_SIZE,
            top + UNIVERSAL_PADDING, 
            MAIN_ICON_SIZE, MAIN_ICON_SIZE, Qt::AlignCenter, iconMode);

    // Counting the number of emblems for this item
    int emblemCount = 1;
//     QPair < Filter, QIcon * > emblem;
//     foreach (emblem, m_parent->m_emblems) {
//         if (item->passesFiltering(emblem.first)) ++emblemCount;
//     }

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
    int emblemLeft = leftToRight ? (left + width - EMBLEM_ICON_SIZE) : left; // - FAV_ICON_SIZE - 2 * UNIVERSAL_PADDING
//     foreach (emblem, m_parent->m_emblems) {
//         if (item->passesFiltering(emblem.first)) {
//             emblem.second->paint(&p, 
//                     emblemLeft, top + UNIVERSAL_PADDING, 
//                     EMBLEM_ICON_SIZE, EMBLEM_ICON_SIZE, Qt::AlignCenter, iconMode);
            if (leftToRight) {
                emblemLeft -= UNIVERSAL_PADDING + EMBLEM_ICON_SIZE;
            } else {
                emblemLeft += UNIVERSAL_PADDING + EMBLEM_ICON_SIZE;
            }
//         }
//     }
    p.end();

    painter->drawPixmap(option.rect.topLeft(), pixmap);
}

void KpkDelegate::paintColFav(QPainter *painter,
        const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int left = option.rect.left();
    int top = option.rect.top();
    int width = option.rect.width();

    // Painting favorite icon column

//     if (! (option.state & QStyle::State_MouseOver) && m_onFavoriteIconItem == item)
//         m_onFavoriteIconItem = NULL;

    if ( !(index.flags() & Qt::ItemIsUserCheckable) )
	return;

    QIcon::Mode iconMode = QIcon::Normal;
    switch ( index.model()->data(index, CheckedRole).toInt() ) {
	case Qt::Unchecked :
	    iconMode = QIcon::Disabled;
	    break;
	case Qt::PartiallyChecked :
	    iconMode = QIcon::Selected;
	    break;
	case Qt::Checked :
	    iconMode = QIcon::Active;
	    break;
    }

//     kDebug() << index.column();

//     const KIcon * icon = ( index.model()->data(index, PkAddRmModel::InstalledRole).toBool() )? & m_removeIcon : & m_addIcon;

    if ( index.model()->data(index, InstalledRole).toBool() )
        m_removeIcon.paint(painter, 
            left + width - FAV_ICON_SIZE - UNIVERSAL_PADDING, top + UNIVERSAL_PADDING, 
            FAV_ICON_SIZE, FAV_ICON_SIZE, Qt::AlignCenter, iconMode);
    else
        m_addIcon.paint(painter, 
            left + width - FAV_ICON_SIZE - UNIVERSAL_PADDING, top + UNIVERSAL_PADDING, 
            FAV_ICON_SIZE, FAV_ICON_SIZE, Qt::AlignCenter, iconMode);

    iconMode = QIcon::Active;

    const KIcon * icon = (index.model()->data(index, CheckedRole).toBool() )? & m_removeIcon : & m_addIcon;

    if ( option.state & QStyle::State_MouseOver )
        icon->paint(painter, 
                left + width - EMBLEM_ICON_SIZE - UNIVERSAL_PADDING, top + UNIVERSAL_PADDING, 
                EMBLEM_ICON_SIZE, EMBLEM_ICON_SIZE, Qt::AlignCenter, iconMode);
}

// void PkDelegate::paintColRemove(QPainter *painter,
//         const QStyleOptionViewItem &option, const KCategorizedItemsViewModels::AbstractItem * item) const
// {
// //     // Painting remove icon column
// //     int running = item->running();
// //     if (!running) {
// //         return;
// //     }
// // 
// //     int left = option.rect.left();
// //     int top = option.rect.top();
// //     int width = option.rect.width();
// // 
// //     QIcon::Mode iconMode = QIcon::Normal;
// //     if (option.state & QStyle::State_MouseOver) {
// //         iconMode = QIcon::Active;
// //     } 
// // 
// //     m_removeIcon.paint(painter, 
// //             left + width - FAV_ICON_SIZE - UNIVERSAL_PADDING, top + UNIVERSAL_PADDING, 
// //             FAV_ICON_SIZE, FAV_ICON_SIZE, Qt::AlignCenter, iconMode);
// // 
// //     if (running == 1) {
// //         return;
// //     }
// //     //paint number
// //     QColor foregroundColor = (option.state.testFlag(QStyle::State_Selected))?
// //         option.palette.color(QPalette::HighlightedText):option.palette.color(QPalette::Text);
// //     painter->setPen(foregroundColor);
// //     painter->setFont(option.font);
// //     painter->drawText(
// //             left + UNIVERSAL_PADDING, //FIXME might be wrong
// //             top + UNIVERSAL_PADDING + MAIN_ICON_SIZE / 2,
// //             width - 2 * UNIVERSAL_PADDING, MAIN_ICON_SIZE / 2,
// //             Qt::AlignCenter, QString::number(running));
// }

bool KpkDelegate::editorEvent(QEvent *event,
                               QAbstractItemModel *model,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index)
{
    if ( !(index.flags() & Qt::ItemIsUserCheckable) )
	return false;

    if ( event->type() == QEvent::MouseButtonPress && index.column() == 1 )
        return model->setData(index, !model->data(index, CheckedRole).toBool(), CheckedRole );
//     else if ( event->type() == QEvent::KeyPress ) {
//     
//     }
    else
        return QItemDelegate::editorEvent(event, model, option, index);
}

QSize KpkDelegate::sizeHint(const QStyleOptionViewItem &option,
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
//         width = qMax(title.width(index.data(Qt::DisplayRole).toString()), normal.width(index.data(KpkPackageModel::SummaryRole).toString())) + MAIN_ICON_SIZE + FADE_LENGTH;
//     } else {
//         width = FAV_ICON_SIZE;
//     }
    int width = (index.column() == 0) ? index.model()->data(index, Qt::SizeHintRole).toSize().width() : FAV_ICON_SIZE;
    return QSize(width, calcItemHeight(option));
}

int KpkDelegate::columnWidth (int column, int viewWidth) const
{
    if (column != 0) {
        return FAV_ICON_SIZE + 2 * UNIVERSAL_PADDING;
    } else return viewWidth - /*2 **/ columnWidth(1, viewWidth);
}

#include "KpkDelegate.moc"
