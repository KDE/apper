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
    id: updateItem
    clip: true
    width: ListView.view.width
    height: items.height + padding.margins.top + padding.margins.bottom
    property bool expanded: ListView.isCurrentItem
    property bool updateChecked: rChecked

    property variant changelog: null

    Behavior on height { PropertyAnimation {} }

    // This is a HACK, for some
    // reason when the user manually checks some updates,
    // clicking to check/uncheck all doesn't work on those
    onUpdateCheckedChanged: updateCB.checked = updateChecked;

    PlasmaCore.FrameSvgItem {
        id: padding
        imagePath: "widgets/viewitem"
        prefix: "hover"
        opacity: 0
        Behavior on opacity { PropertyAnimation {} }
        anchors.fill: parent
    }

    onExpandedChanged: {
        if (expanded) {
            var component = Qt.createComponent("ChangelogView.qml");
            if (component.status === Component.Ready) {
                changelog = component.createObject(actionRow);
            } else {
                console.debug("Error creating details view: " + component.errorString());
            }
        } else if (changelog) {
            changelog.destroy();
        }
    }
    
    MouseArea {
        id: container
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            if (expanded) {
                padding.opacity = 1;
            } else {
                padding.opacity = 0.7;
            }
        }
        onClicked: {
            if (expanded) {
                updateItem.ListView.view.currentIndex = -1;
            } else {
                updateItem.ListView.view.currentIndex = index;
            }
        }
        onExited: {
            if (expanded) {
                padding.opacity = 0.9;
            } else {
                padding.opacity = 0;
            }
        }
    }

    Column {
        id: items
        spacing: 4
        anchors {
            right: parent.right
            left: parent.left
            top: parent.top
            topMargin: padding.margins.top
            leftMargin: padding.margins.left
            rightMargin: padding.margins.right
            bottomMargin: padding.margins.bottom
        }
        Row {
            id: updateRow
            spacing: 4
            anchors.left: parent.left
            anchors.right: parent.right
            PlasmaComponents.CheckBox {
                id: updateCB
                anchors.verticalCenter: parent.verticalCenter
                checked: updateChecked
                height: width
                onClicked: updatesModel.toggleSelection(index)
            }
            QIconItem {
                id: updateIcon
                anchors.verticalCenter: parent.verticalCenter
                width: parent.height
                height: parent.height
                icon: QIcon(rIcon)
            }
            QIconItem {
                id: infoIcon
                anchors.verticalCenter: parent.verticalCenter
                width: parent.height
                height: parent.height
                icon: rInfoIcon
            }
            PlasmaComponents.Label {
                id: updateNameLabel
                anchors.verticalCenter: parent.verticalCenter
                height: paintedHeight
                width: paintedWidth
                text: rName
            }
            PlasmaComponents.Label {
                id: pagesLabel
                anchors.verticalCenter: parent.verticalCenter
                height: paintedHeight
                width: updateRow.width - updateCB.width - updateIcon.width - infoIcon.width - updateNameLabel.width - updateRow.spacing * 4
                font.pointSize: theme.smallestFont.pointSize
                color: "#99"+(theme.textColor.toString().substr(1))
                elide: Text.ElideRight
                text: rSummary
            }
        }

        PlasmaCore.SvgItem {
            id: headerSeparator
            visible: expanded
            svg: PlasmaCore.Svg {
                id: lineSvg
                imagePath: "widgets/line"
            }
            elementId: "horizontal-line"
            height: lineSvg.elementSize("horizontal-line").height
            width: parent.width
        }

        Item {
            id: actionRow
            opacity: expanded ? 1 : 0
            width: parent.width
            height: childrenRect.height
            clip: true
            Behavior on opacity { PropertyAnimation {} }
        }
    }
}

