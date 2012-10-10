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
    id: root

    function deleteLater() {
        root.destroy();
    }

    width: parent.width
    height: busy.running ? busy.height : changelogText.paintedHeight

    property string packageID: ""

    PackageKit.Transaction {
        id: transaction
        onUpdateDetail: {
            busy.running = false;
            busy.visible = false;
            console.debug("onPackageUpdateDetails: ");
            console.debug(packageID);
            console.debug(updateText);
            console.debug(restart);
            console.debug(state);
            console.debug(issued);
            if (restart === PackageKit.Transaction.RestartApplication) {
                console.debug("RestartApplication: ");
            } else {
                console.debug("RestartNone not this: ");
            }

            changelogText.text = changelog;
        }
    }

    PlasmaComponents.BusyIndicator {
        id: busy
        anchors.centerIn: parent
        running: true
    }

    Text {
        id: changelogText
        width: parent.width
        wrapMode: Text.WordWrap
    }

    Component.onCompleted: {
        transaction.getUpdateDetail(packageID);
    }
}
