/*
 *   Copyright 2012 Daniel Nicoletti <dantti12@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

Item {
    id: delegateItem
    width: ListView.view.width
    height: line.height
        
    Row {
        id: line
        spacing: 4
        anchors.centerIn: parent
        width: parent.width
        PlasmaComponents.ProgressBar {
            id: itemProgress
            width: progressWidth
            height: itemStatusLabel.paitedHeight + 4
            minimumValue: 0
            maximumValue: 100
            value: rProgress
            PlasmaComponents.Label {
                // 12 = 3 * spacing
                id: itemStatusLabel
                anchors.centerIn: parent
                text: display
                onPaintedWidthChanged: {
                    if (progressWidth < paintedWidth) {
                        progressWidth = paintedWidth + 8;
                    }
                }
            }
        }
        PlasmaComponents.Label {
            // 12 = 3 * spacing
            id: itemNameLabel
            width: paintedWidth
            height: parent.height
            text: rPkgName
        }
        PlasmaComponents.Label {
            id: itemSummaryLabel
            height: parent.height
            width: line.width - itemProgress.width - itemNameLabel.width - 4
            font.pointSize: theme.smallestFont.pointSize
            color: "#99"+(theme.textColor.toString().substr(1))
            elide: Text.ElideRight
            text: rPkgSummary
        }
    }

ListView.onAdd: {
    console.debug("ListView.onAdd.atYEnd: " + ListView.view.atYEnd + "   - " + progressView.followBottom);


//    if (ListView.view.atYEnd === false) {
//        ListView.view.positionViewAtEnd();
//    }
}

//    ListView.onAdd: SequentialAnimation {
//        PropertyAction { target: delegateItem; property: "height"; value: 0 }
//        NumberAnimation { target: delegateItem; property: "height"; to: 55; duration: 250; easing.type: Easing.InOutQuad }
//    }

    ListView.onRemove: SequentialAnimation {
        PropertyAction { target: delegateItem; property: "ListView.delayRemove"; value: true }
        NumberAnimation { target: delegateItem; property: "height"; to: 0; duration: 250; easing.type: Easing.InOutQuad }

        // Make sure delayRemove is set back to false so that the item can be destroyed
        PropertyAction { target: delegateItem; property: "ListView.delayRemove"; value: false }
    }
}
