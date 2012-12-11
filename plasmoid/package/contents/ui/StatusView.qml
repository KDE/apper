/*
 * Copyright 2012  Daniel Nicoletti <dantti12@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

Item {
    id: statusView

    property string title: ""
    property string subTitle: ""
    property string iconName: ""

    onIconNameChanged: {
        if (iconName === "") {
            state = "BUSY";
        } else {
            state = "ICON";
            statusIcon.icon = QIcon(iconName);
        }
    }

    PlasmaComponents.BusyIndicator {
        id: busy
        width: 64
        height: 64
        anchors.centerIn: parent
        running: true
    }

    QIconItem {
        id: statusIcon
        width: 64
        height: 64
        anchors.centerIn: parent
    }

    Column {
        id: textColumn
        anchors.left: parent.left
        anchors.top: busy.bottom
        anchors.right: parent.right
        spacing: 4
        Text {
            id: titleText
            width: parent.width
            elide: Text.ElideRight
            font.pointSize: font.pointSize * 1.5
            horizontalAlignment: Text.AlignHCenter
            text: title
        }
        Text {
            id: subTitleText
            width: parent.width
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            text: subTitle
        }
    }
}
