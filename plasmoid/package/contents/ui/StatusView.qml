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

    property string title: ""
    property string subTitle: ""
    property string iconName: ""
    property int iconSize: 128

    PlasmaComponents.BusyIndicator {
        id: busy
        width: iconSize
        height: iconSize
        anchors.centerIn: parent
        running: statusView.opacity !== 0
    }

    QIconItem {
        id: statusIcon
        width: iconSize
        height: iconSize
        anchors.centerIn: parent
        icon: QIcon(iconName)
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
            font.pointSize: subTitleText.font.pointSize * 1.5
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

    states: [
        State {
            name: "BUSY"
            PropertyChanges { target: statusIcon; opacity: 0}
        },
        State {
            name: "ICON"
            PropertyChanges { target: busy; opacity: 0}
            PropertyChanges { target: busy; running: false}
        }
    ]
}
