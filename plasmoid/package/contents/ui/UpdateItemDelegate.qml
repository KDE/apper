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
import org.packagekit 0.1 as PackageKit

Item {
    id: updateItem
    width: ListView.view.width
    property bool expanded: ListView.view.currentIndex === index
    height: (expanded ? actionRow.height + updateRow.height : updateRow.height) + padding.margins.top + padding.margins.bottom
    Behavior on height { PropertyAnimation {} }

    PlasmaCore.FrameSvgItem {
        id: padding
        imagePath: "widgets/viewitem"
        prefix: "hover"
        opacity: 0
        Behavior on opacity { PropertyAnimation {} }
        anchors.fill: parent
    }

    onExpandedChanged: {
        console.debug("expanded changed: " + index + " rPackageId " + rId);
        if (expanded) {
            var component = Qt.createComponent("ChangelogView.qml");
            console.debug("component changed: " + component.status);
            if (component.status === Component.Ready) {
                var details = component.createObject(actionRow, {"packageID" : rId});
                console.debug("createObject " + details);
            } else {
                console.debug("component changed: " + component.errorString());
            }
        }
    }
    
    MouseArea {
        id: container
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            padding.opacity = 0.7;
        }
        onClicked: {
            if (updateItem.ListView.view.currentIndex === index) {
                updateItem.ListView.view.currentIndex = -1;
            } else {
                updateItem.ListView.view.currentIndex = index;
            }
        }
        onExited: {
            padding.opacity = 0;
        }
    }
        
    Column {
        id: items
        spacing: 8
        anchors {
            fill: parent
            topMargin: padding.margins.top
            leftMargin: padding.margins.left
            rightMargin: padding.margins.right
            bottomMargin: expanded ? padding.margins.bottom : 0
        }
        Row {
            id: updateRow
            spacing: 4
            width: parent.width - padding.margins.left - padding.margins.right
            height: updateNameLabel.paintedHeight
            PlasmaComponents.CheckBox {
                id: updateCB
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                checked: rChecked
            }
            QIconItem {
                id: updateIcon
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 24
                height: 24
                icon: QIcon(rIcon)
            }
            QIconItem {
                id: infoIcon
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 24
                height: 24
                icon: rInfoIcon
            }
            PlasmaComponents.Label {
                // 12 = 3 * spacing
                id: updateNameLabel
                width: paintedWidth
                height: parent.height
                text: rName
            }
            PlasmaComponents.Label {
                id: pagesLabel
                height: parent.height
                width: updateRow.width - updateIcon.width - infoIcon.width - updateNameLabel.width - updateCB.width
                font.pointSize: theme.smallestFont.pointSize
                color: "#99"+(theme.textColor.toString().substr(1))
                elide: Text.ElideRight
                text: rSummary
            }
        }
        
        Item {
            id: actionRow
            opacity: expanded ? 1 : 0
            width: parent.width
//            height: cancelButton.height + holdButton.height + padding.margins.bottom
            Behavior on opacity { PropertyAnimation {} }
        }
    }
}

