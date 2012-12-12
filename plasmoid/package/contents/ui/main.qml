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
import org.packagekit 0.1 as PackageKit

Item {
    id: root

    state: "STATUS"

    property int minimumWidth: 373
    property int minimumHeight: 272
    property int maximumWidth: 0
    property int maximumHeight: 0
    property int preferredWidth: 0
    property int preferredHeight: 0

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
    }

    function getUpdates() {
        if (!checkedForUpdates) {
            state = "STATUS";
            getUpdatesTransaction.cancel();
            getUpdatesTransaction.reset();
            updatesModel.clear();
            getUpdatesTransaction.getUpdates();
        }
    }

    function getUpdatesFinished() {
        checkedForUpdates = true;
        updatesModel.finished();
        updatesView.sortModel.sortNow();
        updatesModel.clearSelectedNotPresent();
        decideState(false);

    }

    function decideState(force) {
        if (force || state !== "TRANSACTION") {
            if (updatesModel.rowCount() === 0) {
                var lastTime = UpdaterPlasmoid.getTimeSinceLastRefresh();
                statusView.title = PkStrings.lastCacheRefreshTitle(lastTime);
                statusView.subTitle = PkStrings.lastCacheRefreshSubTitle(lastTime);
                statusView.iconName = PkIcons.lastCacheRefreshIconName(lastTime);
                UpdaterPlasmoid.setActive(false);
                state = "STATUS";
            } else {
                UpdaterPlasmoid.setActive(true);
                state = "SELECTION";
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
            statusView.iconName = "";
            statusView.title = PkStrings.action(role);
            statusView.subTitle = PkStrings.status(status);
        }
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
        onFinished: decideState(true)
    }
    StatusView {
        id: statusView
        anchors.fill: parent
    }

    states: [
        State {
            name: "SELECTION"
            PropertyChanges { target: transactionView; opacity: 0}
            PropertyChanges { target: statusView; opacity: 0}
        },
        State {
            name: "TRANSACTION"
            PropertyChanges { target: updatesView; opacity: 0}
            PropertyChanges { target: statusView; opacity: 0}
        },
        State {
            name: "STATUS"
            PropertyChanges { target: transactionView; opacity: 0}
            PropertyChanges { target: updatesView; opacity: 0}
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "opacity"; easing.type: Easing.InOutQuad }
    }
}
