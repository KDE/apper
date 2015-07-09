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

#include "ChangesDelegate.h"

#include <KDebug>
#include <KIconLoader>
#include <KLocalizedString>
#include <QApplication>
#include <QPushButton>
#include <QTreeView>
#include <QHeaderView>

#include "PackageModel.h"
#include "PkIcons.h"

#include <Transaction>

#define FAV_ICON_SIZE 24
#define EMBLEM_ICON_SIZE 8
#define UNIVERSAL_PADDING 4
#define FADE_LENGTH 16
#define MAIN_ICON_SIZE 32

using namespace PackageKit;

ChangesDelegate::ChangesDelegate(QAbstractItemView *parent) :
    KExtendableItemDelegate(parent),
    m_viewport(parent->viewport()),
    // loads it here to be faster when displaying items
    m_packageIcon("package"),
    m_collectionIcon("package-orign"),
    m_installIcon("dialog-cancel"),
    m_installString(i18n("Do not Install")),
    m_removeIcon("dialog-cancel"),
    m_removeString(i18n("Do not Remove")),
    m_undoIcon("edit-undo"),
    m_undoString(i18n("Deselect")),
    m_checkedIcon("dialog-ok-apply")
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

void ChangesDelegate::paint(QPainter *painter,
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
    QString pkgName       = index.data(PackageModel::NameRole).toString();
    QString pkgSummary    = index.data(PackageModel::SummaryRole).toString();
    QString pkgVersion    = index.data(PackageModel::VersionRole).toString();
    QString pkgArch       = index.data(PackageModel::ArchRole).toString();
//     QString pkgIconPath   = index.data(PackageModel::IconPathRole).toString();
    bool    pkgChecked    = index.data(PackageModel::CheckStateRole).toBool();
    bool    pkgCheckable  = !index.data(Qt::CheckStateRole).isNull();
    Transaction::Info info;
    info = index.data(PackageModel::InfoRole).value<Transaction::Info>();
    bool    pkgInstalled  = (info == Transaction::InfoInstalled ||
                             info == Transaction::InfoCollectionInstalled);

    bool    pkgCollection = (info == Transaction::InfoCollectionInstalled ||
                             info == Transaction::InfoCollectionAvailable);

    QIcon emblemIcon;
    if (pkgCheckable) {
        // update kind icon
        emblemIcon = index.data(PackageModel::IconRole).value<QIcon>();
    } else {
        emblemIcon = m_checkedIcon;
    }

    // pain the background (checkbox and the extender)
    if (m_extendPixmapWidth) {
        KExtendableItemDelegate::paint(painter, opt, index);
    }

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
                (option.state & QStyle::State_Selected) ||
                !pkgChecked) {
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
        optBt.features = QStyleOptionButton::Flat;
        optBt.iconSize = m_buttonIconSize;
        optBt.icon = pkgInstalled ? m_removeIcon   : m_installIcon;
        optBt.text = pkgInstalled ? m_removeString : m_installString;
        if (pkgChecked) {
            optBt.state |= QStyle::State_Raised | QStyle::State_Active | QStyle::State_Enabled;;
        } else {
            if ((option.state & QStyle::State_MouseOver) &&
                !(option.state & QStyle::State_Selected)) {
                optBt.state |= QStyle::State_MouseOver;
            }
            optBt.state |= QStyle::State_Sunken | QStyle::State_Active | QStyle::State_Enabled;
        }
        style->drawControl(QStyle::CE_PushButton, &optBt, painter);
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
    if (pkgCollection) {
        icon = m_collectionIcon;
    } else {
        icon = PkIcons::getIcon(index.data(PackageModel::IconRole).toString(), QString());
        if (icon.isNull()) {
            icon = m_packageIcon;
        }
    }
//     if (pkgIconPath.isEmpty()) {
//        icon = pkgCollection ? m_collectionIcon : m_packageIcon;
//     } else {
//         icon = PkIcons::getIcon(pkgIconPath, "package");
//     }

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
        //! pkgName = pkgName + " - " + pkgVersion + (pkgArch.isNull() ? NULL : " (" + pkgArch + ')');
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

int ChangesDelegate::calcItemHeight(const QStyleOptionViewItem &option) const
{
    // Painting main column
    QStyleOptionViewItem local_option_title(option);
    QStyleOptionViewItem local_option_normal(option);

    local_option_normal.font.setPointSize(local_option_normal.font.pointSize() - 1);

    int textHeight = QFontInfo(local_option_title.font).pixelSize() + QFontInfo(local_option_normal.font).pixelSize();
    return textHeight + 3 * UNIVERSAL_PADDING;
}

bool ChangesDelegate::insideButton(const QRect &rect, const QPoint &pos) const
{
//     kDebug() << rect << pos;
    if ((pos.x() >= rect.x() && (pos.x() <= rect.x() + rect.width())) &&
        (pos.y() >= rect.y() && (pos.y() <= rect.y() + rect.height()))) {
        return true;
    }
    return false;
}

bool ChangesDelegate::editorEvent(QEvent *event,
                                    QAbstractItemModel *model,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
{
    Q_UNUSED(option)

    if (event->type() == QEvent::MouseButtonRelease) {
        QAbstractItemView *view = qobject_cast<QAbstractItemView*>(parent());
        QPoint point = m_viewport->mapFromGlobal(QCursor::pos());
        QTreeView *tree = qobject_cast<QTreeView*>(parent());
        if (tree) {
            point.ry() -= tree->header()->size().height();
        }

        bool leftToRight = QApplication::isLeftToRight();
        QStyleOptionButton optBt;
        optBt.rect = option.rect;
        if (leftToRight) {
            optBt.rect.setLeft(option.rect.left() + option.rect.width() - (m_buttonSize.width() + UNIVERSAL_PADDING));
        } else {
            optBt.rect.setLeft(option.rect.left() + UNIVERSAL_PADDING);
        }
        // Calculate the top of the button which is the item height - the button height size divided by 2
        // this give us a little value which is the top and bottom margin
        optBt.rect.setTop(optBt.rect.top() + ((calcItemHeight(option) - m_buttonSize.height()) / 2));
        optBt.rect.setSize(m_buttonSize);

        kDebug() << point << option.rect.left() << option << insideButton(optBt.rect, point);
//         kDebug() << view->visualRect(index);
        if (insideButton(optBt.rect, point)) {
            return model->setData(index,
                                  !index.data(PackageModel::CheckStateRole).toBool(),
                                  Qt::CheckStateRole);
        }
        QRect rect = view->visualRect(index);
        if (QApplication::isRightToLeft()) {
            if ((rect.width() - point.x()) <= m_extendPixmapWidth) {
                emit showExtendItem(index);
            }
        } else if (point.x() <= m_extendPixmapWidth) {
            emit showExtendItem(index);
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

void ChangesDelegate::setExtendPixmapWidth(int width)
{
    m_extendPixmapWidth = width;
}

void ChangesDelegate::setViewport(QWidget *viewport)
{
    m_viewport = viewport;
}

QSize ChangesDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    int width = (index.column() == 0) ? index.data(Qt::SizeHintRole).toSize().width() : FAV_ICON_SIZE + 2 * UNIVERSAL_PADDING;
    QSize ret(KExtendableItemDelegate::sizeHint(option, index));
    // remove the default size of the index
    ret -= QStyledItemDelegate::sizeHint(option, index);

    ret.rheight() += calcItemHeight(option);
    ret.rwidth()  += width;

    return ret;
}

#include "ChangesDelegate.moc"
