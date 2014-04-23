/*
 * Copyright 2012  Luís Gabriel Lima <lampih@gmail.com>
 * Copyright 2012-2013 Daniel Nicoletti <dantti12@gmail.com>
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
    id: root

    state: ""

    property int minimumHeight: 320
    property int minimumWidth: 370
    property int maximumHeight: 0
    property int maximumWidth: 0
    property int preferredHeight: 0
    property int preferredWidth: 0
    property int implicitHeight: 0
    property int implicitWidth: 0

    property bool checkedForUpdates: false
    property string tooltipText: i18n("Software Updates")
    property string tooltipIcon: "kpackagekit-updates"

    property Component compactRepresentation: CompactRepresentation {
        text: tooltipText
        icon: tooltipIcon
    }

    anchors.fill: parent
    clip: true

    PlasmaCore.Theme {
        id: theme
    }

    Component.onCompleted: {
        getUpdatesTransaction.package.connect(updatesModel.addSelectedPackage);
        getUpdatesTransaction.errorCode.connect(errorCode);
        getUpdatesTransaction.finished.connect(getUpdatesFinished);

        Daemon.updatesChanged.connect(updatesChanged);

        // This allows the plasmoid to shrink when the layout changes
        plasmoid.status = "PassiveStatus"
        plasmoid.aspectRatioMode = IgnoreAspectRatio
        plasmoid.popupEvent.connect(popupEventSlot)
        plasmoid.setAction('checkForNewUpdates', i18n("Check for new updates"), 'view-refresh')
        dbusInterface.registerService()
        dbusInterface.reviewUpdates.connect(reviewUpdates)
        dbusInterface.reviewUpdates.connect(plasmoid.showPopup)
    }

    function popupEventSlot(popped) {
        if (popped) {
            root.forceActiveFocus()
            getUpdates()
        }
    }

    function action_checkForNewUpdates() {
        transactionView.refreshCache();
        var error = transactionView.transaction.internalError;
        if (error) {
            statusView.title = PkStrings.daemonError(error);
            statusView.subTitle = transactionView.transaction.internalErrorMessage;
            statusView.iconName = "dialog-error";
            root.state = "MESSAGE";
        } else {
            root.state = "TRANSACTION";
        }
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
            state = "BUSY"
            getUpdatesTransaction.cancel()
            updatesModel.clear()
            getUpdatesTransaction.getUpdates()
            checkedForUpdates = true
        }
    }

    function errorCode() {
        statusView.title = i18n("Failed to get updates");
        statusView.subTitle = PkStrings.daemonError(error);
        statusView.iconName = "dialog-error";
        state = "MESSAGE";
        plasmoid.status = "PassiveStatus"
    }

    function getUpdatesFinished() {
        updatesModel.finished();
        updatesModel.clearSelectedNotPresent();
        updatesView.sortModel.sortNow();
        updateIcon();
        decideState(false);
    }

    function decideState(force) {
        if (force || state !== "TRANSACTION") {
            if (updatesModel.rowCount() === 0) {
                var lastTime = DaemonHelper.getTimeSinceLastRefresh();
                statusView.title = PkStrings.lastCacheRefreshTitle(lastTime);
                statusView.subTitle = PkStrings.lastCacheRefreshSubTitle(lastTime);
                statusView.iconName = PkIcons.lastCacheRefreshIconName(lastTime);
                state = "MESSAGE";
                plasmoid.status = "PassiveStatus"
            } else {
                plasmoid.status = "ActiveStatus"
                root.state = "SELECTION";
            }
        }
    }

    function updatesChanged() {
        checkedForUpdates = false;
        getUpdates();
    }

    function updateIcon() {
        if (updatesModel.rowCount()) {
            if (updatesModel.countInfo(Apper.Transaction.InfoSecurity)) {
                tooltipIcon = "kpackagekit-security"
            } else if (updatesModel.countInfo(Apper.Transaction.InfoImportant)) {
                tooltipIcon = "kpackagekit-important"
            } else {
                tooltipIcon = "kpackagekit-updates"
            }
            tooltipText = i18np("You have one update",
                                "You have %1 updates",
                                updatesModel.rowCount())
        } else {
            tooltipIcon = "kpackagekit-updates"
            tooltipText = i18n("Your system is up to date")
        }
    }

    Apper.PackageModel {
        id: updatesModel
        checkable: true
    }

    Apper.DBusUpdaterInterface {
        id: dbusInterface
    }

    Timer {
        // Grab updates in five minutes
        interval: 360000
        running: true
        repeat: false
        onTriggered: getUpdates()
    }

    Apper.PkTransaction {
        id: getUpdatesTransaction
    }

    StatusView {
        id: busyView
        opacity: 0
        anchors.fill: parent
        anchors.margins: 4
        state: "BUSY"
        title: PkStrings.action(getUpdatesTransaction.role, getUpdatesTransaction.transactionFlags)
        subTitle: PkStrings.status(getUpdatesTransaction.status)
    }

    Column {
        id: statusColumn
        opacity: 0
        spacing: 4
        anchors.fill: parent
        anchors.margins: 4
        StatusView {
            id: statusView
            height: parent.height - refreshBT.height - parent.anchors.margins - parent.spacing
            width: parent.width
        }
        PlasmaComponents.Button {
            id: refreshBT
            anchors.right: parent.right
            text:  i18n("Check for new updates")
            iconSource: "system-software-update"
            onClicked: action_checkForNewUpdates()
        }
    }

    Updates {
        id: updatesView
        opacity: 0
        anchors.fill: parent
        anchors.margins: 4
        onUpdateClicked: installUpdates()
    }

    Transaction {
        id: transactionView
        opacity: 0
        anchors.fill: parent
        anchors.margins: 4
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
            name: "MESSAGE"
            PropertyChanges { target: statusColumn; opacity: 1 }
            PropertyChanges { target: refreshBT; focus: true }
        }
    ]

    transitions: Transition {
        NumberAnimation { properties: "opacity"; easing.type: Easing.InOutQuad }
    }
}
