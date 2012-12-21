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
import org.packagekit 0.1 as PackageKit

Item {
    id: changelogItem
    state: "FETCHING"
    width: parent.width

    property string updatesList: ""

    PackageKit.Transaction {
        id: transaction
        onUpdateDetail: {
            for (var count = 0; count < updates.length; ++count) {
                if (updatesList.length) {
                    updatesList += ", "
                }
                updatesList += Daemon.packageName(updates[count]) + " - " + Daemon.packageVersion(updates[count]);
            }

            if (updateText === "" || updateText === undefined) {
                if (changelog !== "" && changelog !== undefined) {
                    changelogText.text = changelog;
                }
            } else {
                changelogText.text = updateText;
            }
            changelogItem.state = "DETAILS";
        }
    }

    PlasmaComponents.BusyIndicator {
        id: busy
        anchors.centerIn: parent
        running: true
    }

    Column {
        id: detailsColumn
        spacing: 2
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.leftMargin: updateCB.width
        PlasmaComponents.Label {
            id: updateVersion
            width: parent.width
            wrapMode: Text.Wrap
            text: i18n("Version: %1", rVersion)
        }
        PlasmaComponents.Label {
            id: updatesText
            width: parent.width
            wrapMode: Text.Wrap
            text: i18n("Updates: %1", updatesList)
        }
        PlasmaComponents.Label {
            id: changelogText
            width: parent.width
            wrapMode: Text.Wrap
        }
    }

    states: [
        State {
            name: "FETCHING"
            PropertyChanges { target: detailsColumn; opacity: 0 }
            PropertyChanges { target: changelogItem; height: busy.height }
        },
        State {
            name: "DETAILS"
            PropertyChanges { target: busy; opacity: 0 }
            PropertyChanges { target: busy; running: false }
            PropertyChanges { target: changelogItem; height: detailsColumn.height }
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "opacity"; easing.type: Easing.InOutQuad }
    }

    Component.onCompleted: {
        transaction.getUpdateDetail(rId);
    }
}
