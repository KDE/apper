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
import org.packagekit 0.1 as PackageKit

Item {
    id: root

    state: "BUSY"

    property int minimumHeight: 320
    property int minimumWidth: 370
    property int maximumHeight: 0
    property int maximumWidth: 0
    property int preferredHeight: 0
    property int preferredWidth: 0
    property int implicitHeight: 0
    property int implicitWidth: 0


    property bool checkedForUpdates: false

    anchors.fill: parent
    clip: true

    PlasmaCore.Theme {
        id: theme
    }

    Component.onCompleted: {
        getUpdatesTransaction.package.connect(updatesModel.addSelectedPackage);
        getUpdatesTransaction.finished.connect(getUpdatesFinished);

        Daemon.updatesChanged.connect(updatesChanged);
        UpdaterPlasmoid.getUpdates.connect(getUpdates);
        UpdaterPlasmoid.checkForNewUpdates.connect(checkForNewUpdates);
    }

    function checkForNewUpdates() {
        transactionView.refreshCache();
        root.state = "TRANSACTION";
    }

    function getUpdates() {
        if (!checkedForUpdates) {
            state = "BUSY";
            getUpdatesTransaction.cancel();
            getUpdatesTransaction.reset();
            updatesModel.clear();
            getUpdatesTransaction.getUpdates();
            checkedForUpdates = true;
        }
    }

    function getUpdatesFinished() {
        updatesModel.finished();
        updatesView.sortModel.sortNow();
        updatesModel.clearSelectedNotPresent();
        UpdaterPlasmoid.updateIcon();
        decideState(false);
    }

    function decideState(force) {
        if (force || state !== "TRANSACTION") {
            if (updatesModel.rowCount() === 0) {
                var lastTime = UpdaterPlasmoid.getTimeSinceLastRefresh();
                statusView.title = PkStrings.lastCacheRefreshTitle(lastTime);
                statusView.subTitle = PkStrings.lastCacheRefreshSubTitle(lastTime);
                statusView.iconName = PkIcons.lastCacheRefreshIconName(lastTime);
                state = "UPTODATE";
                UpdaterPlasmoid.setActive(false);
            } else {
                UpdaterPlasmoid.setActive(true);
                statusView.iconName = "system-software-update";
                statusView.title = i18np("There is one update", "There are %1 updates", updatesModel.rowCount(), updatesModel.rowCount());
                statusView.subTitle = "";
                state = "HAVEUPDATES";
                UpdaterPlasmoid.showPopupIfDifferent();
            }
        }
    }

    function updatesChanged() {
        checkedForUpdates = false;
        getUpdates();
    }

    PackageKit.Transaction {
        id: getUpdatesTransaction
        onChanged: {
            busyView.title = PkStrings.action(role);
            busyView.subTitle = PkStrings.status(status);
        }
    }

    StatusView {
        id: busyView
        anchors.fill: parent
        state: "BUSY"
    }

    Column {
        spacing: 4
        anchors.fill: parent
        anchors.margins: 4
        Item {
            height: parent.height - actionRow.height - parent.anchors.margins
            width: parent.width
            StatusView {
                id: statusView
                anchors.fill: parent
            }
            Updates {
                id: updatesView
                anchors.fill: parent
            }
        }
        Row {
            id: actionRow
            spacing: 4
            anchors.right: parent.right
            PlasmaComponents.Button {
                id: refreshBT
                text:  i18n("Check for new updates")
                onClicked: checkForNewUpdates()
            }
            PlasmaComponents.Button {
                id: reviewBT
                text:  i18n("Review")
                onClicked: root.state = "SELECTION"
            }
            PlasmaComponents.Button {
                id: updateBT
                text:  i18n("Install")
                onClicked: {
                    if (root.state === "HAVEUPDATES") {
                        updatesModel.setAllChecked(true);
                    }
                    transactionView.update(updatesModel.selectedPackagesToInstall());
                    root.state = "TRANSACTION";
                }
            }
        }
    }

    Transaction {
        id: transactionView
        anchors.fill: parent
        onFinished: {
            if (success) {
                if (root.state !== "BUSY") {
                    checkedForUpdates = false;
                    getUpdates();
                }
            } else {
                decideState(true);
            }
        }
    }

    states: [
        State {
            name: "SELECTION"
            PropertyChanges { target: transactionView; opacity: 0 }
            PropertyChanges { target: statusView; opacity: 0 }
            PropertyChanges { target: busyView; opacity: 0 }
            PropertyChanges { target: reviewBT; opacity: 0 }
            PropertyChanges { target: refreshBT; opacity: 0 }
        },
        State {
            name: "TRANSACTION"
            PropertyChanges { target: updatesView; opacity: 0 }
            PropertyChanges { target: statusView; opacity: 0 }
            PropertyChanges { target: busyView; opacity: 0 }
            PropertyChanges { target: refreshBT; opacity: 0 }
            PropertyChanges { target: reviewBT; opacity: 0 }
            PropertyChanges { target: updateBT; opacity: 0 }
        },
        State {
            name: "BUSY"
            PropertyChanges { target: transactionView; opacity: 0 }
            PropertyChanges { target: updatesView; opacity: 0 }
            PropertyChanges { target: statusView; opacity: 0 }
            PropertyChanges { target: refreshBT; opacity: 0 }
            PropertyChanges { target: reviewBT; opacity: 0 }
            PropertyChanges { target: updateBT; opacity: 0 }
        },
        State {
            name: "HAVEUPDATES"
            PropertyChanges { target: busyView; opacity: 0 }
            PropertyChanges { target: transactionView; opacity: 0 }
            PropertyChanges { target: updatesView; opacity: 0 }
            PropertyChanges { target: refreshBT; opacity: 0 }
        },
        State {
            name: "UPTODATE"
            PropertyChanges { target: busyView; opacity: 0 }
            PropertyChanges { target: transactionView; opacity: 0 }
            PropertyChanges { target: updatesView; opacity: 0 }
            PropertyChanges { target: reviewBT; opacity: 0 }
            PropertyChanges { target: updateBT; opacity: 0 }
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "opacity"; easing.type: Easing.InOutQuad }
    }
}
