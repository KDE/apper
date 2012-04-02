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

#include <PkStrings.h>
#include <PkIcons.h>

#include <KCategorizedSortFilterProxyModel>
#include <KLocale>
#include <KServiceGroup>
#include <KDesktopFile>
#include <KConfigGroup>

#include <KDebug>

#include <Daemon>

#include <config.h>

CategoryModel::CategoryModel(Transaction::Roles roles, QObject *parent) :
    QStandardItemModel(parent),
    m_roles(roles)
{
    QStandardItem *item;
    item = new QStandardItem(i18n("Installed Software"));
    item->setDragEnabled(false);
    item->setData(Transaction::RoleGetPackages, SearchRole);
    item->setData(i18n("Lists"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    item->setData(0, KCategorizedSortFilterProxyModel::CategorySortRole);
    item->setIcon(KIcon("dialog-ok-apply"));
    appendRow(item);

    item = new QStandardItem(i18n("Updates"));
    item->setDragEnabled(false);
    item->setData(Transaction::RoleGetUpdates, SearchRole);
    item->setData(i18n("Lists"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    item->setData(0, KCategorizedSortFilterProxyModel::CategorySortRole);
    item->setIcon(KIcon("system-software-update"));
    appendRow(item);

#ifdef HAVE_APPINSTALL
    // Get the groups
    m_groups = Daemon::groups();
    fillWithServiceGroups();
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
    if (m_roles & Transaction::RoleGetCategories
        && Daemon::getTransactions().isEmpty()) {
        Transaction *trans = new Transaction(this);
        connect(trans, SIGNAL(category(QString,QString,QString,QString,QString)),
                this, SLOT(category(QString,QString,QString,QString,QString)));
        connect(trans, SIGNAL(finished(PackageKit::Transaction::Exit,uint)),
                this, SIGNAL(finished()));
        trans->getCategories();
        if (trans->error()) {
            fillWithStandardGroups();
        }
    } else {
        fillWithStandardGroups();
    }
#endif //HAVE_APPINSTALL
    QTimer::singleShot(0, this, SIGNAL(finished()));
}

CategoryModel::~CategoryModel()
{
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
    item->setIcon(KIcon("/usr/share/pixmaps/comps/" + icon + ".png"));

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
    m_groups = Daemon::groups();
    QStandardItem *item;
    foreach (const Package::Group &group, m_groups) {
        if (group != Package::UnknownGroup) {
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

    emit finished();
}

void CategoryModel::fillWithServiceGroups()
{
#ifdef AI_CATEGORIES_PATH
    KGlobal::locale()->insertCatalog("gnome-menus");
    QFile file(QString(AI_CATEGORIES_PATH) + "/categories.xml");
     if (!file.open(QIODevice::ReadOnly)) {
         kDebug() << "Failed to open file";
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
#endif //AI_CATEGORIES_PATH
}

void CategoryModel::parseMenu(QXmlStreamReader &xml, const QString &parentIcon, QStandardItem *parent)
{
//     kDebug() << 1 << xml.name();
    QString icon = parentIcon;
    QStandardItem *item = 0;
    while(!xml.atEnd() &&
          !(xml.tokenType() == QXmlStreamReader::EndElement &&
            xml.name() == "Menu")) {

        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "Menu") {
                xml.readNext();
//                 kDebug() << "Found Menu";
                parseMenu(xml, icon, item);
            } else if (xml.name() == "Name") {
                QString name = xml.readElementText();
                if (!item) {
                    item = new QStandardItem(i18n(name.toUtf8()));
                    item->setDragEnabled(false);
                } else if (item->text().isEmpty()) {
                    item->setText(i18n(name.toUtf8()));
                }
            } else if (xml.name() == "Icon") {
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
            } else if (xml.name() == "Categories") {
                if (!item) {
                    item = new QStandardItem;
                    item->setDragEnabled(false);
                }
                xml.readNext();
//                 kDebug() << "Found Categories           ";
                QString categories;
                categories = parseCategories(xml, item);
//                 kDebug() << categories;
                item->setData(categories, CategoryRole);
                item->setData(Transaction::RoleResolve, SearchRole);
            } else if (xml.name() == "Directory") {
                if (!item) {
                    item = new QStandardItem;
                    item->setDragEnabled(false);
                }
                QString directory = xml.readElementText();

                const KDesktopFile desktopFile("xdgdata-dirs", directory);
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
            } else if (xml.name() == "PkGroups") {
                if (!item) {
                    item = new QStandardItem;
                    item->setDragEnabled(false);
                }
                QString group = xml.readElementText();
                Package::Group groupEnum = static_cast<Package::Group>(enumFromString<Package>(group, "Group", "Group"));

                if (groupEnum != Package::UnknownGroup &&
                    (m_groups.contains(groupEnum))) {
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
            Package::Group group;
            group = static_cast<Package::Group>(item->data(GroupRole).toUInt());
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

QString CategoryModel::parseCategories(QXmlStreamReader &xml, QStandardItem *item, const QString &join)
{
    QString endElement = join;
    // When the join element is empty means that it was started by
    // 'Categories' keyword
    if (endElement.isEmpty()) {
        endElement = "Categories";
    }
//     kDebug() << "endElement" << endElement;

    QStringList ret;
    while(!xml.atEnd() &&
          !(xml.tokenType() == QXmlStreamReader::EndElement &&
            xml.name() == endElement)) {
        if(xml.tokenType() == QXmlStreamReader::StartElement) {
            // Where the categories where AND or OR
            if (xml.name() == "And" || xml.name() == "Or") {
//                 kDebug() << "Found:" << xml.name();
                QString _endElement = xml.name().toString();
                xml.readNext();
                QString andOr;
                andOr = parseCategories(xml, item, _endElement);
                if (!andOr.isEmpty()) {
                    // The result should be so that we make sure the
                    // precedence is right "( andOr )"
                    andOr.prepend("( ");
                    andOr.append(" )");
                    ret << andOr;
                }
            }

            // USED to negate the categories inside it
            if(xml.name() == "Not") {
//                 kDebug() << "Found:" << xml.name();
                xml.readNext();
                QString _ret;
                _ret = parseCategories(xml, item, "Not");
                if (!_ret.isEmpty()) {
                    ret << _ret;
                }
            }

            // Found the real category, if the join was not means
            // that applications in this category should NOT be displayed
            if(xml.name() == "Category") {
                QString name;
                name = xml.readElementText();
                if (!name.isEmpty()){
                    if (join == "Not") {
                        ret << QString("categories != '%1' AND "
                                       "categories NOT GLOB '*;%1' AND "
                                       "categories NOT GLOB '*;%1;*' AND "
                                       "categories NOT GLOB '%1;*'").arg(name);
                    } else {
                        ret << QString("categories = '%1' OR "
                                       "categories GLOB '*;%1' OR "
                                       "categories GLOB '*;%1;*' OR "
                                       "categories GLOB '%1;*'").arg(name);
                    }
                }
            }
        }

        xml.readNext();
    }

    if (ret.isEmpty()) {
        return QString();
    }
    return ret.join(' ' + join + ' ');
}

template<class T> int CategoryModel::enumFromString(const QString& str, const char* enumName, const QString& prefix)
{
    QString realName;
    bool lastWasDash = false;
    QChar buf;

    for(int i = 0 ; i < str.length() ; ++i) {
        buf = str[i].toLower();
        if(i == 0 || lastWasDash) {
            buf = buf.toUpper();
        }

        lastWasDash = false;
        if(buf.toAscii() == '-') {
            lastWasDash = true;
        } else if(buf.toAscii() == '~') {
            lastWasDash = true;
            realName += "Not";
        } else {
            realName += buf;
        }
    };

    if(!prefix.isNull())
        realName = prefix + realName;

    // Filter quirk
    if(enumName == QString("Filter")) {
        if(realName == QString("FilterNone"))
            realName = "NoFilter";

        if(realName == QString("FilterDevel") || realName == QString("FilterNotDevel"))
            realName += "opment";
    }

    // Action quirk
    if(enumName == QString("Action") && realName == QString("ActionUpdatePackage"))
        realName = "ActionUpdatePackages";


    int id = T::staticMetaObject.indexOfEnumerator(enumName);
    QMetaEnum e = T::staticMetaObject.enumerator(id);
    int enumValue = e.keyToValue(realName.toAscii().data());

    if(enumValue == -1) {
        enumValue = e.keyToValue(QString("Unknown").append(enumName).toAscii().data());
        qDebug() << "enumFromString (" << enumName << ") : converted" << str << "to" << QString("Unknown").append(enumName) << ", enum value" << enumValue;
    }
    return enumValue;
}


#include "CategoryModel.moc"
