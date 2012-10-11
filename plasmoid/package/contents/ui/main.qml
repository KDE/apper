/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
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
    id: root

    property int minimumWidth: 373
    property int minimumHeight: 272

    anchors.fill: parent
    clip: true

    PlasmaCore.Theme {
        id: theme
    }

    Row {
        id: actionRow
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        Text {
            width: parent.width - updateBT.width
            text: "You have X updates"
        }

        PackageKit.Transaction {
            id: updateTransaction
        }

        PlasmaComponents.ToolButton {
            id: updateBT
            flat: true
            iconSource: "system-software-update"
            text:  i18n("Update")
            onClicked: {
                updateTransaction.updatePackages(updatesModel.selectedPackagesToInstall(), PackageKit.Transaction.TransactionFlagOnlyTrusted | PackageKit.Transaction.TransactionFlagOnlyDownload)
            }
        }
    }

    ScrollableListView {
        height: parent.height - actionRow.height
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        delegate: UpdateItemDelegate {

        }

        model: Apper.PackageModel {
            id: updatesModel
        }
    }

    Component.onCompleted: {
        updateTransaction.package.connect(updatesModel.addPackage);
        updateTransaction.finished.connect(updatesModel.finished);
        updateTransaction.getUpdates();
//        updatesModel.getUpdates(false, true);
    }
}
