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
    state: rRepo ? "REPO" : "PACKAGE"

    Row {
        id: line
        spacing: 4
        anchors.centerIn: parent
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        width: parent.width
        PlasmaComponents.ProgressBar {
            id: itemProgress
            width: progressWidth
            height: itemStatusLabel.paitedHeight + 4
            minimumValue: 0
            maximumValue: 100
            PlasmaComponents.Label {
                // 12 = 3 * spacing
                id: itemStatusLabel
                anchors.centerIn: parent
                onPaintedWidthChanged: {
                    if (progressWidth < paintedWidth) {
                        progressWidth = paintedWidth + 8;
                    }
                }
            }
        }
        PlasmaComponents.Label {
            id: itemNameLabel
            anchors.verticalCenter: parent.verticalCenter
            height: paintedHeight
            width: paintedWidth
        }
        PlasmaComponents.Label {
            id: itemSummaryLabel
            anchors.verticalCenter: parent.verticalCenter
            height: paintedHeight
            width: line.width - itemProgress.width - itemNameLabel.width - 8
            color: "#99"+(theme.textColor.toString().substr(1))
            elide: Text.ElideRight
        }
    }

    PlasmaComponents.Label {
        id: repoLabel
        anchors.centerIn: parent
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        width: parent.width
        height: parent.height
        elide: Text.ElideRight
    }

    states: [
        State {
            name: "REPO"
            PropertyChanges { target: line; opacity: 0 }
            PropertyChanges { target: repoLabel; text: display }
        },
        State {
            name: "PACKAGE"
            PropertyChanges { target: repoLabel; opacity: 0 }
            PropertyChanges { target: itemProgress; value: rProgress }
            PropertyChanges { target: itemStatusLabel; text: display }
            PropertyChanges { target: itemNameLabel; text: rPkgName }
            PropertyChanges { target: itemSummaryLabel; text: rPkgSummary }
        }
    ]
}
