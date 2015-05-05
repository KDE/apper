import QtQuick 2.1
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.0

import Apper 1.0

ScrollView {
    id: root

    ListView {
        width: root.viewport.width

        header: Rectangle {
            width: parent.width
            height: topLayout.height + 10
            color: sysPalette.window
            RowLayout {
                id: topLayout
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 5
                Label {
                    Layout.fillWidth: true
                    text: qsTr("You have %1 updates").arg(pkgModel.count)
                }
                Button {
                    text: qsTr("Update All")
                }
            }
        }

        model: ApplicationSortFilterModel {
            id: sortModel
            sourcePkgModel: PackageModel {
                id: pkgModel
                onCountChanged: sortModel.sortNow()
            }
            applicationsOnly: false
        }

        delegate: UpdateDelegate {
            width: ListView.view.width
        }
    }

    Component.onCompleted: {
        pkgModel.getInstalled()
    }
}
