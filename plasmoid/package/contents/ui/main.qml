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

Item {
    id: root

    state: "SELECTION"

    property int minimumWidth: 373
    property int minimumHeight: 272

    anchors.fill: parent
    clip: true

    PlasmaCore.Theme {
        id: theme
    }

    Updates {
        id: updatesView
        anchors.fill: parent
        onUpdate: {
            transactionView.update(packages);
            root.state = "TRANSACTION";
        }
    }

    Transaction {
        id: transactionView
        anchors.fill: parent
    }

    states: [
        State {
            name: "SELECTION"
            PropertyChanges { target: transactionView; opacity: 0}
        },
        State {
            name: "TRANSACTION"
            PropertyChanges { target: updatesView; opacity: 0}
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "opacity"; easing.type: Easing.InOutQuad }
    }
}
