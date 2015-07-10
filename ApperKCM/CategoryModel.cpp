/***************************************************************************
 *   Copyright (C) 2010-2011 by Daniel Nicoletti                           *
 *   dantti12@gmail.com                                                    *
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

#include "CategoryModel.h"

#include <QMetaEnum>
#include <QFile>
#include <QTimer>
#include <QStringBuilder>

#include <PkStrings.h>
#include <PkIcons.h>

#include <KCategorizedSortFilterProxyModel>
#include <KLocalizedString>
#include <KServiceGroup>
#include <KDesktopFile>
#include <KConfigGroup>

#include <KDebug>

#include <Daemon>

#include <config.h>

using namespace PackageKit;

CategoryModel::CategoryModel(QObject *parent) :
    QStandardItemModel(parent)
{
    QStandardItem *item;
    item = new QStandardItem(i18n("Installed Software"));
    item->setDragEnabled(false);
    item->setData(Transaction::RoleGetPackages, SearchRole);
    item->setData(i18n("Lists"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    item->setData(0, KCategorizedSortFilterProxyModel::CategorySortRole);
    item->setIcon(QIcon::fromTheme("dialog-ok-apply"));
    appendRow(item);

    item = new QStandardItem(i18n("Updates"));
    item->setDragEnabled(false);
    item->setData(Transaction::RoleGetUpdates, SearchRole);
    item->setData(i18n("Lists"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    item->setData(0, KCategorizedSortFilterProxyModel::CategorySortRole);
    item->setIcon(QIcon::fromTheme("system-software-update"));
    appendRow(item);

#ifdef HAVE_APPSTREAM
    // Get the groups
#ifdef AS_CATEGORIES_PATH
    fillWithServiceGroups();
#else
    fillWithStandardGroups();
#endif // AS_CATEGORIES_PATH
//         category("",
//                              "servers",
//                              "Servers",
//                              "const QString &summary",
//                              "computer");
//     category("servers",
//                              "@coomputer",
//                              "Lighttpd",
//                              "const QString &summary",
//                              "emblem-new");
//     category("servers",
//                              "@coomputer2",
//                              "Apache",
//                              "const QString &summary",
//                              "dialog-cancel");
#else

#endif //HAVE_APPSTREAM
    QTimer::singleShot(0, this, SIGNAL(finished()));
}

CategoryModel::~CategoryModel()
{
}

void CategoryModel::setRoles(Transaction::Roles roles)
{
    m_roles = roles;
    removeRows(2, rowCount() - 2);

    QDBusPendingReply<QList<QDBusObjectPath> > transactions = Daemon::getTransactionList();
    transactions.waitForFinished();
    if (m_roles & Transaction::RoleGetCategories
        && transactions.value().isEmpty()) {
        Transaction *trans = Daemon::getCategories();
        connect(trans, SIGNAL(category(QString,QString,QString,QString,QString)),
                this, SLOT(category(QString,QString,QString,QString,QString)));
        connect(trans, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                this, SIGNAL(finished()));
    } else {
        fillWithStandardGroups();
    }
}

QModelIndex CategoryModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return QStandardItemModel::index(row, column, parent);
    }
    return QStandardItemModel::index(row, column, m_rootIndex);
}

int CategoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return QStandardItemModel::rowCount(parent);
    }
    return QStandardItemModel::rowCount(m_rootIndex);
}

void CategoryModel::setRootIndex(const QModelIndex &index)
{
    m_rootIndex = index;
    reset();
    emit finished();
}

bool CategoryModel::setParentIndex()
{
    if (m_rootIndex.isValid()) {
        setRootIndex(m_rootIndex.parent());
        // Return the future parent so that Back button can be disabled
        return m_rootIndex.parent().isValid();
    }
    // if there is no higher level return false
    return false;
}

bool CategoryModel::hasParent() const
{
    return m_rootIndex.isValid();
}

void CategoryModel::category(const QString &parentId,
                             const QString &categoryId,
                             const QString &name,
                             const QString &summary,
                             const QString &icon)
{
    kDebug() << parentId << categoryId << name << summary << icon;
    QStandardItem *item = new QStandardItem(name);
    item->setDragEnabled(false);
    item->setData(Transaction::RoleSearchGroup, SearchRole);
    item->setData(categoryId, GroupRole);
    item->setData(i18n("Categories"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    item->setData(2, KCategorizedSortFilterProxyModel::CategorySortRole);
    item->setToolTip(summary);
    item->setIcon(QIcon("/usr/share/pixmaps/comps/" + icon + ".png"));

    if (parentId.isEmpty()) {
        appendRow(item);
    } else {
        QStandardItem *parent = findCategory(parentId);
        if (parent) {
            item->setData(parent->text(),
                          KCategorizedSortFilterProxyModel::CategoryDisplayRole);
            item->setData(2, KCategorizedSortFilterProxyModel::CategorySortRole);
            parent->appendRow(item);
        } else {
            appendRow(item);
        }
    }

    // This is a MUST since the spacing needs to be fixed
    emit finished();
}

QStandardItem* CategoryModel::findCategory(const QString &categoryId, const QModelIndex &parent) const
{
    QStandardItem *ret = 0;
    for (int i = 0; i < rowCount(parent); i++) {
        QModelIndex group = index(i, 0, parent);
        if (group.data(SearchRole).toUInt() == Transaction::RoleSearchGroup
         && group.data(GroupRole).toString() == categoryId) {
            ret = itemFromIndex(group);
        } else if (hasChildren(group)) {
            ret = findCategory(categoryId, group);
        }

        if (ret) {
            break;
        }
    }
    return ret;
}

void CategoryModel::fillWithStandardGroups()
{
    // Get the groups
    m_groups = Daemon::global()->groups();
    kDebug();
    QStandardItem *item;
    for (int i = 1; i < 64; ++i) {
        if (m_groups & i) {
            Transaction::Group group;
            group = static_cast<Transaction::Group>(i);
            if (group != Transaction::GroupUnknown) {
                item = new QStandardItem(PkStrings::groups(group));
                item->setDragEnabled(false);
                item->setData(Transaction::RoleSearchGroup, SearchRole);
                item->setData(group, GroupRole);
                item->setData(i18n("Groups"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
                item->setData(1, KCategorizedSortFilterProxyModel::CategorySortRole);
                item->setIcon(PkIcons::groupsIcon(group));
                if (!(m_roles & Transaction::RoleSearchGroup)) {
                    item->setSelectable(false);
                }
                appendRow(item);
            }
        }
    }

    emit finished();
}

void CategoryModel::fillWithServiceGroups()
{
#ifdef AS_CATEGORIES_PATH
    KGlobal::locale()->insertCatalog("gnome-menus");
    QFile file(QString(AS_CATEGORIES_PATH) + "/categories.xml");
     if (!file.open(QIODevice::ReadOnly)) {
         kDebug() << "Failed to open file";
         fillWithStandardGroups();
         return;
    }
    QXmlStreamReader xml(&file);

    while(xml.readNextStartElement() && !xml.hasError()) {
        // Read next element.
        if(xml.tokenType() == QXmlStreamReader::StartDocument) {
//             kDebug() << "StartDocument";
            continue;
        }

        if(xml.name() == "Menu") {
            parseMenu(xml, QString());
        }
    }
#endif //AS_CATEGORIES_PATH
}

void CategoryModel::parseMenu(QXmlStreamReader &xml, const QString &parentIcon, QStandardItem *parent)
{
    QString icon = parentIcon;
    QStandardItem *item = 0;
    while(!xml.atEnd() &&
          !(xml.tokenType() == QXmlStreamReader::EndElement &&
            xml.name() == QLatin1String("Menu"))) {

        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == QLatin1String("Menu")) {
                xml.readNext();
                parseMenu(xml, icon, item);
            } else if (xml.name() == QLatin1String("Name")) {
                QString name = xml.readElementText();
                if (!item) {
                    item = new QStandardItem(i18n(name.toUtf8().data()));
                    item->setDragEnabled(false);
                } else if (item->text().isEmpty()) {
                    item->setText(i18n(name.toUtf8().data()));
                }
            } else if (xml.name() == QLatin1String("Icon")) {
                if (!item) {
                    item = new QStandardItem;
                    item->setDragEnabled(false);
                }
                // only sets the icon if it wasn't set,
                // the .directory might have a better one
                QString _icon;
                _icon = xml.readElementText();
                if (item->icon().isNull()) {
                    item->setIcon(PkIcons::getIcon(_icon, icon));
                    icon = _icon;
                }
            } else if (xml.name() == QLatin1String("Categories")) {
                QList<CategoryMatcher> categories;
                categories = parseCategories(xml);
                if (!categories.isEmpty()) {
                    if (!item) {
                        item = new QStandardItem;
                        item->setDragEnabled(false);
                    }

                    // If we only have one category inside get the first item
                    if (categories.size() == 1) {
                        item->setData(qVariantFromValue(categories.first()), CategoryRole);
                    } else {
                        CategoryMatcher parser(CategoryMatcher::And);
                        parser.setChild(categories);
                        item->setData(qVariantFromValue(parser), CategoryRole);
                    }
                    item->setData(Transaction::RoleResolve, SearchRole);
                }
            } else if (xml.name() == QLatin1String("Directory")) {
                if (!item) {
                    item = new QStandardItem;
                    item->setDragEnabled(false);
                }
                QString directory = xml.readElementText();

                const KDesktopFile desktopFile(directory);
                const KConfigGroup config = desktopFile.desktopGroup();
                QString _icon = config.readEntry("Icon");
                QString _name = config.readEntry("Name");
                if (!_icon.isEmpty()) {
                    item->setIcon(PkIcons::getIcon(_icon, icon));
                    icon = _icon;
                }
                if (!_name.isEmpty()) {
                    item->setText(_name);
                }
            } else if (xml.name() == QLatin1String("PkGroups")) {
                if (!item) {
                    item = new QStandardItem;
                    item->setDragEnabled(false);
                }
                QString group = xml.readElementText();
                Transaction::Group groupEnum;
                int groupInt = Daemon::enumFromString<Transaction>(group, "Group");
                groupEnum = static_cast<Transaction::Group>(groupInt);
                if (groupEnum != Transaction::GroupUnknown && m_groups & groupEnum) {
                    item->setData(Transaction::RoleSearchGroup, SearchRole);
                    item->setData(groupEnum, GroupRole);
                }
            }
        }

        // next...
        xml.readNext();
    }

    if (item &&
        (!item->data(GroupRole).isNull() || !item->data(CategoryRole).isNull())) {
        if (item->data(CategoryRole).isNull()) {
            // Set the group name to get it translated
            Transaction::Group group;
            group = item->data(GroupRole).value<Transaction::Group>();
            item->setText(PkStrings::groups(group));
        }
        item->setData(i18n("Categories"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
        item->setData(1, KCategorizedSortFilterProxyModel::CategorySortRole);
        if (parent) {
            parent->appendRow(item);
        } else {
            appendRow(item);
        }
    }
}

QList<CategoryMatcher> CategoryModel::parseCategories(QXmlStreamReader &xml)
{
    QString token = xml.name().toString();

    QList<CategoryMatcher> ret;
    while(!xml.atEnd() && !(xml.readNext() == QXmlStreamReader::EndElement && xml.name() == token)) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            // Where the categories where AND
            if (xml.name() == QLatin1String("And")) {
                // We are going to read the next element to save the token name
                QList<CategoryMatcher> parsers;
                parsers = parseCategories(xml);
                if (!parsers.isEmpty()) {
                    CategoryMatcher opAND(CategoryMatcher::And);
                    opAND.setChild(parsers);
                    ret << opAND;
                }
            } else if (xml.name() == QLatin1String("Or")) {
                // Where the categories where OR
                QList<CategoryMatcher> parsers;
                parsers = parseCategories(xml);
                if (!parsers.isEmpty()) {
                    CategoryMatcher opOR(CategoryMatcher::Or);
                    opOR.setChild(parsers);
                    ret << opOR;
                }
            } else if (xml.name() == QLatin1String("Not")) {
                // USED to negate the categories inside it
                QList<CategoryMatcher> parsers;
                parsers = parseCategories(xml);
                if (!parsers.isEmpty()) {
                    CategoryMatcher opNot(CategoryMatcher::Not);
                    opNot.setChild(parsers);
                    ret << opNot;
                }
            } else if (xml.name() == QLatin1String("Category")) {
                // Found the real category, if the join was not means
                // that applications in this category should NOT be displayed
                QString name = xml.readElementText();
                if (!name.isEmpty()){
                    ret << CategoryMatcher(CategoryMatcher::Term, name);
                }
            }
        }
    }

    return ret;
}
