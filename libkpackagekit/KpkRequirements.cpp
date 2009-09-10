/***************************************************************************
 *   Copyright (C) 2008-2009 by Daniel Nicoletti                           *
 *   dantti85-pk@yahoo.com.br                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/
// The delegate was written by
// Copyright (C) 2007 Kevin Ottens <ervin@kde.org>
// Copyright (C) 2008 Rafael Fernández López <ereslibre@kde.org>
// Under LGPL
/*
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "KpkRequirements.h"
#include "ui_KpkRequirements.h"

#include <KpkIcons.h>

#include <KDebug>

#include <QPalette>
#include <QStandardItemModel>

Q_DECLARE_METATYPE(Package::State)

#include <QPainter>
#define LATERAL_MARGIN 4

class KActionsViewDelegate : public QAbstractItemDelegate
{
public:
    KActionsViewDelegate(QObject *parent = 0);
    virtual ~KActionsViewDelegate();
    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const;
    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;

    int iconSize() const;
    void setIconSize(int newSize);

    void setShowHoverIndication(bool show);

    void addFadeAnimation(const QModelIndex &index, QTimeLine *timeLine);
    void removeFadeAnimation(const QModelIndex &index);
    QModelIndex indexForFadeAnimation(QTimeLine *timeLine) const;
    QTimeLine *fadeAnimationForIndex(const QModelIndex &index) const;

    qreal contentsOpacity(const QModelIndex &index) const;

private:
    int m_iconSize;

    QList<QPersistentModelIndex> m_appearingItems;
    int m_appearingIconSize;
    qreal m_appearingOpacity;

    QList<QPersistentModelIndex> m_disappearingItems;
    int m_disappearingIconSize;
    qreal m_disappearingOpacity;

    bool m_showHoverIndication;

    QMap<QPersistentModelIndex, QTimeLine*> m_timeLineMap;
    QMap<QTimeLine*, QPersistentModelIndex> m_timeLineInverseMap;
};

KActionsViewDelegate::KActionsViewDelegate(QObject *parent) :
    QAbstractItemDelegate(parent),
    m_iconSize(48),
    m_appearingIconSize(0),
    m_appearingOpacity(0.0),
    m_disappearingIconSize(0),
    m_disappearingOpacity(0.0),
    m_showHoverIndication(true)
{
}

KActionsViewDelegate::~KActionsViewDelegate()
{
}

QSize KActionsViewDelegate::sizeHint(const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    int iconSize = m_iconSize;
    if (m_appearingItems.contains(index)) {
        iconSize = m_appearingIconSize;
    } else if (m_disappearingItems.contains(index)) {
        iconSize = m_disappearingIconSize;
    }

    return QSize(option.rect.width(), option.fontMetrics.height() / 2 + qMax(iconSize, option.fontMetrics.height()));
}

void KActionsViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    if (m_appearingItems.contains(index)) {
        painter->setOpacity(m_appearingOpacity);
    } else if (m_disappearingItems.contains(index)) {
        painter->setOpacity(m_disappearingOpacity);
    }

    QStyleOptionViewItemV4 opt = option;
    if (!m_showHoverIndication) {
        opt.state &= ~QStyle::State_MouseOver;
    }
    QApplication::style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter);

    bool isLTR = option.direction == Qt::LeftToRight;

    QIcon icon = index.model()->data(index, Qt::DecorationRole).value<QIcon>();
    QPixmap pm = icon.pixmap(m_iconSize, m_iconSize);
    QPoint point(isLTR ? option.rect.left() + LATERAL_MARGIN
                       : option.rect.right() - LATERAL_MARGIN - m_iconSize, option.rect.top() + (option.rect.height() - m_iconSize) / 2);
    painter->drawPixmap(point, pm);

    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.highlightedText().color());
    }

    QRect rectText;
    rectText = QRect(isLTR ? m_iconSize + LATERAL_MARGIN * 2 + option.rect.left()
                           : 0, option.rect.top(), option.rect.width() - m_iconSize - LATERAL_MARGIN * 2, option.rect.height());
    painter->drawText(rectText, Qt::AlignLeft | Qt::AlignVCenter, option.fontMetrics.elidedText(index.model()->data(index).toString(), Qt::ElideRight, rectText.width()));

    painter->restore();
}

int KActionsViewDelegate::iconSize() const
{
    return m_iconSize;
}

void KActionsViewDelegate::setIconSize(int newSize)
{
    m_iconSize = newSize;
}

void KActionsViewDelegate::setShowHoverIndication(bool show)
{
    m_showHoverIndication = show;
}

void KActionsViewDelegate::addFadeAnimation(const QModelIndex &index, QTimeLine *timeLine)
{
    m_timeLineMap.insert(index, timeLine);
    m_timeLineInverseMap.insert(timeLine, index);
}

void KActionsViewDelegate::removeFadeAnimation(const QModelIndex &index)
{
    QTimeLine *timeLine = m_timeLineMap.value(index, 0);
    m_timeLineMap.remove(index);
    m_timeLineInverseMap.remove(timeLine);
}

QModelIndex KActionsViewDelegate::indexForFadeAnimation(QTimeLine *timeLine) const
{
    return m_timeLineInverseMap.value(timeLine, QModelIndex());
}

QTimeLine *KActionsViewDelegate::fadeAnimationForIndex(const QModelIndex &index) const
{
    return m_timeLineMap.value(index, 0);
}

qreal KActionsViewDelegate::contentsOpacity(const QModelIndex &index) const
{
    QTimeLine *timeLine = fadeAnimationForIndex(index);
    if (timeLine) {
        return timeLine->currentValue();
    }
    return 0;
}


class KpkRequirementsPrivate
{
public:
    QStandardItemModel *actionsModel;
    Ui::KpkRequirements ui;
};

KpkRequirements::KpkRequirements(KpkSimulateModel *model, QWidget *parent)
 : KDialog(parent), d(new KpkRequirementsPrivate)
{
    d->ui.setupUi(mainWidget());
    setCaption(i18n("Additional changes"));
    setButtons(KDialog::Ok | KDialog::Cancel);
    setButtonText(KDialog::Ok, i18n("Continue"));
    setModal(true);
    // restore size
    setMinimumSize(QSize(450,200));
    setInitialSize(QSize(450,200));
    KConfig config("KPackageKit");
    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    restoreDialogSize(requirementsDialog);

    d->ui.packageView->setModel(model);
    d->actionsModel = new QStandardItemModel(this);
    d->ui.actionsView->setModel(d->actionsModel);
    d->ui.actionsView->setItemDelegate(new KActionsViewDelegate(d->ui.actionsView));

    connect(d->ui.actionsView->selectionModel(),
                SIGNAL(currentChanged(const QModelIndex &,
                                      const QModelIndex &)),
            this,
                SLOT(actionClicked(const QModelIndex &)));

    // Sets a transparent background
    QWidget *actionsViewport = d->ui.actionsView->viewport();
    QPalette palette = actionsViewport->palette();
    palette.setColor(actionsViewport->backgroundRole(), Qt::transparent);
    palette.setColor(actionsViewport->foregroundRole(), palette.color(QPalette::WindowText));
    actionsViewport->setPalette(palette);

    // Populate the actionsModel
    QStandardItem *item;
    if (int c = model->countState(Package::StateRemoving)) {
        item = new QStandardItem;
        item->setText(i18np("1 package to remove", "%1 packages to remove", c));
        item->setIcon(KpkIcons::actionIcon(Client::ActionRemovePackages));
        item->setData(QVariant::fromValue(Package::StateRemoving));
        d->actionsModel->appendRow(item);
        model->setCurrentState(Package::StateRemoving);
    }

    if (int c = model->countState(Package::StateDowngrading)) {
        item = new QStandardItem;
        item->setText(i18np("1 package to downgrade", "%1 packages to downgrade", c));
        item->setIcon(KpkIcons::actionIcon(Client::ActionRemovePackages));
        item->setData(QVariant::fromValue(Package::StateDowngrading));
        if (model->currentState() == Package::UnknownState) {
            model->setCurrentState(Package::StateDowngrading);
        }
        d->actionsModel->appendRow(item);
    }

    if (int c = model->countState(Package::StateReinstalling)) {
        item = new QStandardItem;
        item->setText(i18np("1 package to reinstall", "%1 packages to reinstall", c));
        item->setIcon(KpkIcons::actionIcon(Client::ActionRemovePackages));
        item->setData(QVariant::fromValue(Package::StateReinstalling));
        if (model->currentState() == Package::UnknownState) {
            model->setCurrentState(Package::StateReinstalling);
        }
        d->actionsModel->appendRow(item);
    }

    if (int c = model->countState(Package::StateInstalling)) {
        item = new QStandardItem;
        item->setText(i18np("1 package to install", "%1 packages to install", c));
        item->setIcon(KpkIcons::actionIcon(Client::ActionInstallPackages));
        item->setData(QVariant::fromValue(Package::StateInstalling));
        if (model->currentState() == Package::UnknownState) {
            model->setCurrentState(Package::StateInstalling);
        }
        d->actionsModel->appendRow(item);
    }

    if (int c = model->countState(Package::StateUpdating)) {
        item = new QStandardItem;
        item->setText(i18np("1 package to update", "%1 packages to update", c));
        item->setIcon(KpkIcons::actionIcon(Client::ActionUpdatePackages));
        item->setData(QVariant::fromValue(Package::StateUpdating));
        if (model->currentState() == Package::UnknownState) {
            model->setCurrentState(Package::StateUpdating);
        }
        d->actionsModel->appendRow(item);
    }

    if (d->actionsModel->rowCount()) {
        d->ui.actionsView->setCurrentIndex(d->actionsModel->index(0, 0));
    }
    d->ui.packageView->resizeColumnToContents(0);
    d->ui.packageView->resizeColumnToContents(1);
}

KpkRequirements::~KpkRequirements()
{
    // save size
    KConfig config("KPackageKit");
    KConfigGroup requirementsDialog(&config, "requirementsDialog");
    saveDialogSize(requirementsDialog);
}

void KpkRequirements::actionClicked(const QModelIndex &index)
{
    Package::State state = index.data(Qt::UserRole+1).value<Package::State>();
    static_cast<KpkSimulateModel*>(d->ui.packageView->model())->setCurrentState(state);
    d->ui.packageView->resizeColumnToContents(0);
    d->ui.packageView->resizeColumnToContents(1);
}

#include "KpkRequirements.moc"
