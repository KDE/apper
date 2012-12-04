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

    function getUpdates() {
        getUpdatesTransaction.getUpdates();
    }

    PackageKit.Transaction {
        id: getUpdatesTransaction
    }

    Row {
        id: actionRow
        spacing: 4
        anchors.margins: 2
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        PlasmaComponents.CheckBox {
            id: updateAllCB
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            checked: rChecked
            onClicked: updatesModel.setAllChecked(checked);
        }
        Text {
            height: parent.height
            width: parent.width - updateBT.width - updateAllCB.width - parent.spacing * 2
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            text: updatesModel.selectionStateText
        }
        PlasmaComponents.ToolButton {
            id: updateBT
            flat: true
            iconSource: "system-software-update"
            text:  i18n("Update")
            onClicked: {
                updateTransaction.updatePackages(updatesModel.selectedPackagesToInstall());
            }
        }
    }

    ScrollableListView {
        height: parent.height - actionRow.height - 4
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        delegate: UpdateItemDelegate {
        }

        model: Apper.PackageModel {
            id: updatesModel
            checkable: true
            onChanged: updateAllCB.checked = updatesModel.allSelected();
        }
    }

    Component.onCompleted: {
        getUpdatesTransaction.package.connect(updatesModel.addSelectedPackage);
        getUpdatesTransaction.finished.connect(updatesModel.finished);
        getUpdates();
    }
}
