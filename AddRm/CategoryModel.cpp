/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
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

#include "CategoryModel.h"

#include <KpkStrings.h>
#include <KpkIcons.h>

#include <KCategorizedSortFilterProxyModel>
#include <KLocale>
#include <KServiceGroup>
#include <KDesktopFile>
#include <KConfigGroup>

#include <KDebug>

#include <config.h>

CategoryModel::CategoryModel(QObject *parent)
 : QStandardItemModel(parent)
{
    QStandardItem *item;
    item = new QStandardItem(i18n("Installed Software"));
    item->setData(Enum::RoleGetPackages, SearchRole);
    item->setData(i18n("Lists"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    item->setData(0, KCategorizedSortFilterProxyModel::CategorySortRole);
    item->setIcon(KIcon("dialog-ok-apply"));
    appendRow(item);

    item = new QStandardItem(i18n("History"));
    item->setData(Enum::RoleGetOldTransactions, SearchRole);
    item->setData(i18n("Lists"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
    item->setData(0, KCategorizedSortFilterProxyModel::CategorySortRole);
    item->setIcon(KIcon("view-history"));
    appendRow(item);

    // Get the groups
    m_groups = Client::instance()->groups();
    m_roles  = Client::instance()->actions();

#ifdef HAVE_APPINSTALL
    fillWithServiceGroups();
#endif //HAVE_APPINSTALL

    if (rowCount() == 1) {
        fillWithStandardGroups();
    }
}

CategoryModel::~CategoryModel()
{
}

void CategoryModel::fillWithStandardGroups()
{
    QStandardItem *item;
    foreach (const Enum::Group &group, m_groups) {
        if (group != Enum::UnknownGroup) {
            item = new QStandardItem(KpkStrings::groups(group));
            item->setData(Enum::RoleSearchGroup, SearchRole);
            item->setData(group, GroupRole);
            item->setData(i18n("Groups"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
            item->setData(1, KCategorizedSortFilterProxyModel::CategorySortRole);
            item->setIcon(KpkIcons::groupsIcon(group));
            if (!(m_roles & Enum::RoleSearchGroup)) {
                item->setSelectable(false);
            }
            appendRow(item);
        }
    }
}

void CategoryModel::fillWithServiceGroups()
{
#ifdef AI_CATEGORIES_PATH
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
                    item = new QStandardItem(name);
                } else if (item->text().isEmpty()) {
                    item->setText(name);
                }
            } else if (xml.name() == "Icon") {
                if (!item) {
                    item = new QStandardItem;
                }
                // only sets the icon if it wasn't set,
                // the .directory might have a better one
                QString _icon;
                _icon = xml.readElementText();
                if (item->icon().isNull()) {
                    item->setIcon(KpkIcons::getIcon(_icon, icon));
                    icon = _icon;
                }
            } else if (xml.name() == "Categories") {
                if (!item) {
                    item = new QStandardItem;
                }
                xml.readNext();
//                 kDebug() << "Found Categories           ";
                QString categories;
                categories = parseCategories(xml, item);
                kDebug() << categories;
                item->setData(categories, CategoryRole);
                item->setData(Enum::RoleResolve, SearchRole);
            } else if (xml.name() == "Directory") {
                if (!item) {
                    item = new QStandardItem;
                }
                QString directory = xml.readElementText();

                const KDesktopFile desktopFile("xdgdata-dirs", directory);
                const KConfigGroup config = desktopFile.desktopGroup();
                QString _icon = config.readEntry("Icon");
                QString _name = config.readEntry("Name");
                if (!_icon.isEmpty()) {
                    item->setIcon(KpkIcons::getIcon(_icon, icon));
                    icon = _icon;
                }
                if (!_name.isEmpty()) {
                    item->setText(_name);
                }
            } else if (xml.name() == "PkGroups") {
                if (!item) {
                    item = new QStandardItem;
                }
                QString group = xml.readElementText();
                Enum::Group groupEnum = static_cast<Enum::Group>(enumFromString<Enum>(group, "Group", "Group"));

                if (groupEnum != Enum::UnknownGroup &&
                    (m_groups.contains(groupEnum))) {
                    item->setData(Enum::RoleSearchGroup, SearchRole);
                    item->setData(groupEnum, GroupRole);
                }
            }
        }

        // next...
        xml.readNext();
    }

    if (item &&
        (!item->data(GroupRole).isNull() || !item->data(CategoryRole).isNull())) {
        item->setData(i18n("Categories"), KCategorizedSortFilterProxyModel::CategoryDisplayRole);
        item->setData(2, KCategorizedSortFilterProxyModel::CategorySortRole);
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
