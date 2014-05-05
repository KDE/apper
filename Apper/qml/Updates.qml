import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

import org.packagekit 1.0

ScrollView {
    id: root

    ColumnLayout {
        width: root.viewport.width - 10
        x: 5
        y: 5

        Rectangle {
            Layout.fillWidth: true
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

        PackageModel {
            id: pkgModel
        }

        GroupBox {
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: appsRepeater.count
            title: qsTr("Applications")

            ColumnLayout {
                Repeater {
                    id: appsRepeater
                    property int col1: 0
                    delegate: UpdateDelegate {
                        width: parent.width
                    }
                    model: ApplicationSortFilterModel {
                        sourcePkgModel: pkgModel
                        applicationsOnly: true
                    }
                }
            }
        }

        GroupBox {
            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: pkgsRepeater.count
            title: qsTr("System Packages")

            ColumnLayout {
                Repeater {
                    id: pkgsRepeater
                    delegate: UpdateDelegate {
                        width: parent.width
                    }
                    model: ApplicationSortFilterModel {
                        sourcePkgModel: pkgModel
                        packagesOnly: true
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        pkgModel.getUpdates(true, true)
    }
}
