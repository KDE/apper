import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.0

ApplicationWindow {
    id: mainWindow
    width: 800
    height: 600
    visible: true

    title: "Apper"

    SystemPalette { id: sysPalette }

    color: sysPalette.base

    function addPage(dict) {
        // Remove history forward to the current location
        if (goNext.enabled) {
            historyModel.remove(mainView.depth, historyModel.count - mainView.depth)
        }

        historyModel.append(dict)
        createPage(historyModel.count - 1)
    }

    function createPage(index) {
        var dict = historyModel.get(index)
        mainView.push({"item": Qt.resolvedUrl(dict.page), "properties": dict})
    }

    toolBar: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                iconName: "go-previous"
                enabled: mainView.depth > 1
                onClicked: mainView.pop()
            }

            ToolButton {
                id: goNext
                iconName: "go-next"
                enabled: mainView.depth < historyModel.count
                onClicked: createPage(mainView.depth)
            }

            Item {
                Layout.fillWidth: true
            }

            ToolButton {
                id: installedBt
//                iconName: "system-software-update"
                text: qsTr("Installed")
                onClicked: {
                    var currentPage = historyModel.get(mainView.currentIndex)
                    if (currentPage.page === "Installed.qml") {
                        return
                    }

                    addPage({"page": "Installed.qml"})
                }
            }

            ToolButton {
                id: home
//                iconName: "system-software-update"
                text: qsTr("Updates")
                onClicked: {
                    var currentPage = historyModel.get(mainView.currentIndex)
                    if (currentPage.page === "Updates.qml") {
                        return
                    }

                    addPage({"page": "Updates.qml"})
                }
            }

            Item {
                Layout.fillWidth: true
            }

            TextField {
                id: searchText
                focus: true
                onAccepted: {
                    selectAll()
                    var currentPage = historyModel.get(mainView.currentIndex)
                    if (currentPage.page === "Search.qml" && currentPage.query === text) {
                        return
                    }

                    addPage({"page": "Search.qml", "query": text})
                }
            }
        }
    }

    ListModel {
        id: historyModel

        ListElement {
            page: "Home.qml"
            query: "none"
        }
    }

    StackView {
        id: mainView

        initialItem: Qt.resolvedUrl("Home.qml")

        delegate: StackViewDelegate {
            function transitionFinished(properties)
            {
                properties.exitItem.opacity = 1
            }

            pushTransition: StackViewTransition {
                PropertyAnimation {
                    target: enterItem
                    property: "opacity"
                    from: 0
                    to: 1
                }
                PropertyAnimation {
                    target: exitItem
                    property: "opacity"
                    from: 1
                    to: 0
                }
            }
        }
    }

    Component.onCompleted: {

        console.debug("mainView.depth changed: " + mainView.depth)
    }
}
