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

FocusScope {
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
        UpdaterPlasmoid.reviewUpdates.connect(reviewUpdates);
    }

    function checkForNewUpdates() {
        transactionView.refreshCache();
        root.state = "TRANSACTION";
    }

    function reviewUpdates() {
        if (root.state !== "TRANSACTION") {
            // If we are not checking for updates show
            // the package selection
            if (checkedForUpdates && root.state !== "BUSY") {
                root.state = "SELECTION";
            } else {
                getUpdates();
            }
        }
    }

    function installUpdates() {
        transactionView.update(updatesModel.selectedPackagesToInstall());
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
                root.state = "SELECTION";
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
        opacity: 0
        anchors.fill: parent
        state: "BUSY"
    }

    Column {
        id: statusColumn
        opacity: 0
        spacing: 4
        anchors.fill: parent
        anchors.margins: 4
        StatusView {
            id: statusView
            height: parent.height - refreshBT.height - parent.anchors.margins
            width: parent.width
        }
        PlasmaComponents.Button {
            id: refreshBT
            anchors.right: parent.right
            text:  i18n("Check for new updates")
            iconSource: "system-software-update"
            onClicked: checkForNewUpdates()
        }
    }

    Updates {
        id: updatesView
        opacity: 0
        anchors.fill: parent
        onUpdateClicked: installUpdates()
    }

    Transaction {
        id: transactionView
        opacity: 0
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
            PropertyChanges { target: updatesView; opacity: 1 }
            PropertyChanges { target: updatesView; focus: true }
        },
        State {
            name: "TRANSACTION"
            PropertyChanges { target: transactionView; opacity: 1 }
            PropertyChanges { target: transactionView; focus: true }
        },
        State {
            name: "BUSY"
            PropertyChanges { target: busyView; opacity: 1 }
        },
        State {
            name: "UPTODATE"
            PropertyChanges { target: statusColumn; opacity: 1 }
            PropertyChanges { target: refreshBT; focus: true }
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "opacity"; easing.type: Easing.InOutQuad }
    }
}
