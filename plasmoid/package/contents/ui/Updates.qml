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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.apper 0.1 as Apper
import org.packagekit 0.1 as PackageKit

Item {
    id: updates

    anchors.fill: parent
    clip: true

    property alias sortModel: appModel

    function modelChanged() {
        updateAllCB.checked = updatesModel.allSelected();
        // Enable the update button if there are packages to install
        updateBT.enabled = updatesModel.selectedPackagesToInstall().length;
    }

    PlasmaCore.FrameSvgItem {
        id: padding
        imagePath: "widgets/viewitem"
        prefix: "hover"
        visible: false
    }

    Row {
        id: headerRow
        spacing: 4
        height: updateAllCB.height
        anchors.leftMargin: padding.margins.left
        anchors.topMargin: padding.margins.top
        anchors.rightMargin: padding.margins.right
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        PlasmaComponents.CheckBox {
            id: updateAllCB
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            onClicked: updatesModel.setAllChecked(checked);
        }
        Text {
            height: parent.height
            width: parent.width - updateAllCB.width - parent.spacing
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            text: updatesModel.selectionStateText
        }
    }

    Apper.ApplicationSortFilterModel {
        id: appModel
        sourcePkgModel: updatesModel
    }

    PlasmaCore.SvgItem {
        id: headerSeparator
        anchors.top: headerRow.bottom
        svg: PlasmaCore.Svg {
            id: lineSvg
            imagePath: "widgets/line"
        }
        elementId: "horizontal-line"
        height: lineSvg.elementSize("horizontal-line").height
        width: parent.width
    }

    ScrollableListView {
        height: parent.height - headerRow.height - padding.margins.top * 2
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        delegate: UpdateItemDelegate {
        }
        view.currentIndex: -1
        model: appModel
    }

    Component.onCompleted: {
        updatesModel.changed.connect(modelChanged);
    }
}