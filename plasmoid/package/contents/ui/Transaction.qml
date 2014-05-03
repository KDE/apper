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

FocusScope {
    id: transactionItem

    anchors.fill: parent
    clip: true

    property int progressWidth: 30
    property alias transaction: updateTransaction

    signal finished(bool success);

    function update(updates) {
        updateTransaction.enableJobWatcher(true);
        updateTransaction.updatePackages(updates);
    }

    function refreshCache() {
        updateTransaction.enableJobWatcher(false);
        updateTransaction.refreshCache(false);
    }

    Apper.PkTransaction {
        id: updateTransaction
        onPercentageChanged: updateTransaction.percentage
        onFinished: transactionItem.finished(status === 0)
    }

    Column {
        id: actionRow
        spacing: 4
        anchors.fill: parent
        Row {
            id: labelButtonRow
            spacing: 4
            anchors.left: parent.left
            anchors.right: parent.right
            PlasmaComponents.Label {
                id: statusText
                anchors.verticalCenter: parent.verticalCenter
                height: paintedHeight
                width: parent.width - updateBT.width - parent.spacing
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
                text: PkStrings.status(updateTransaction.status, updateTransaction.speed, updateTransaction.downloadSizeRemaining)
            }
            PlasmaComponents.Button {
                id: updateBT
                anchors.verticalCenter: parent.verticalCenter
                focus: true
                iconSource: "dialog-cancel"
                text:  i18n("Cancel")
                enabled: updateTransaction.allowCancel
                onClicked: updateTransaction.cancel();
            }
        }
        PlasmaComponents.ProgressBar {
            id: transactionProgress
            anchors.left: parent.left
            anchors.right: parent.right
            minimumValue: 0
            maximumValue: 100
            value: updateTransaction.percentage
        }
        ListView {
            id: progressView
            clip: true
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - labelButtonRow.height - transactionProgress.height - parent.spacing * 2
            delegate: TransactionProgressDelegate {
            }
            boundsBehavior: Flickable.StopAtBounds
            currentIndex: -1
            model: updateTransaction.progressModel()
            onCountChanged: positionViewAtEnd()
        }
    }
}
