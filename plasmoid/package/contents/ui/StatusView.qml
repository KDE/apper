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

    state: "ICON"

    property alias title: titleText.text
    property alias subTitle: subTitleText.text
    property string iconName: ""
    property int iconSize: 128
    property alias preferedHeight: column.height

    Column {
        id: column
        width: parent.width
        anchors.centerIn: parent
        spacing: 4
        Item {
            id: image
            width: iconSize
            height: iconSize
            anchors.horizontalCenter: parent.horizontalCenter

            PlasmaComponents.BusyIndicator {
                id: busy
                opacity: 0
                anchors.fill: parent
                running: statusView.opacity !== 0 && opacity !== 0
            }
            QIconItem {
                id: statusIcon
                opacity: 0
                anchors.fill: parent
                icon: QIcon(iconName)
            }
        }
        Column {
            id: textColumn
            width: parent.width
            spacing: 4
            PlasmaComponents.Label {
                id: titleText
                width: parent.width
                height: paintedHeight
                elide: Text.ElideRight
                font.pointSize: subTitleText.font.pointSize * 1.5
                horizontalAlignment: Text.AlignHCenter
            }
            PlasmaComponents.Label {
                id: subTitleText
                width: parent.width
                height: paintedHeight
                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    states: [
        State {
            name: "BUSY"
            PropertyChanges { target: busy; opacity: 1 }
        },
        State {
            name: "ICON"
            PropertyChanges { target: statusIcon; opacity: 1 }
        }
    ]
}
