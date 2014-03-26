import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

ApplicationWindow {
    id: mainWindow
    width: 800
    height: 600
    visible: true

    title: "Apper"

    SystemPalette { id: sysPalette }

    function addPage(dict) {
        // Remove history forward to the current location
        if (goNext.enabled) {
            historyModel.remove(mainView.currentIndex + 1, historyModel.count - (mainView.currentIndex + 1))
        }

        historyModel.append(dict)
    }

    toolBar: ToolBar {
        RowLayout {
            anchors.fill: parent

            ToolButton {
                iconName: "go-previous"
                enabled: mainView.currentIndex
                onClicked: mainView.currentIndex = mainView.currentIndex - 1
            }

            ToolButton {
                id: goNext
                iconName: "go-next"
                enabled: mainView.currentIndex + 1 < historyModel.count
                onClicked: mainView.currentIndex = mainView.currentIndex + 1
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

            Item {
                Layout.fillWidth: true
            }

            ToolButton {
                id: home
                iconName: "system-software-update"
                text: qsTr("Updates")
                onClicked: {
                    var currentPage = historyModel.get(mainView.currentIndex)
                    if (currentPage.page === "Updates.qml") {
                        return
                    }

                    addPage({"page": "Updates.qml"})
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

    ListView {
        id: mainView
        anchors.fill: parent
        interactive: false
        orientation: ListView.Horizontal
        highlightMoveDuration: 500
        model: historyModel
        delegate: Page {
            height: ListView.view.height
            width: ListView.view.width
        }

        onCountChanged: currentIndex = historyModel.count - 1
    }
}
