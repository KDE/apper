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

    function removeForwardHistory() {
        // Remove history forward to the current location
        if (goNext.enabled) {
            historyModel.remove(mainView.currentIndex + 1, historyModel.count - (mainView.currentIndex + 1))
        }
    }

    toolBar: ToolBar {
        RowLayout {
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
                onAccepted: {
                    console.debug(text)
                    var currentPage = historyModel.get(mainView.currentIndex)
                    if (currentPage.kind === "search" && currentPage.search === text) {
                        console.debug("Same query, ignoring...")
                        return
                    }

                    removeForwardHistory()
                    historyModel.append({"kind": "search", "search": text})
                    mainView.currentIndex = historyModel.count - 1
                }
            }
        }
    }

    ListModel {
        id: historyModel

        ListElement {
            kind: "home"
            cost: 2.45
        }
//        onItemsInserted: mainView.currentIndex = historyModel.count - 1
    }

    ListView {
        id: mainView
        anchors.fill: parent
        interactive: false
        orientation: ListView.Horizontal
        model: historyModel
        delegate: Text {
            height: ListView.view.height
            width: ListView.view.width
            text: model.kind + ", search string: " + model.search
        }
//        onAdd: currentIndex = historyModel.count
//        onAdd: {
//            currentIndex = historyModel.count
//        }
    }
}
