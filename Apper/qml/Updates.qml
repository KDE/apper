import QtQuick 2.1
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.0

import Apper 1.0

ScrollView {
    id: root

    GridView {
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
            sourcePkgModel: PackageModel {
                id: pkgModel
            }
            applicationsOnly: true
        }

        footer: ListView {
            id: updatesView
            height: contentHeight

            property int col1: 0

            delegate: UpdateDelegate {
                width: ListView.view.width
            }

            model: ApplicationSortFilterModel {
                sourcePkgModel: pkgModel
            }

            // The delegate for each section header
            Component {
                id: sectionHeading
                Rectangle {
                    //                width: container.width
                    height: childrenRect.height
                    color: sysPalette.window

                    CheckBox {
                        id: systemPackages
                        visible: section == "true"
                        text: "System Packages"
                    }

                    Text {
                        visible: !systemPackages.visible
                        text: "Application"
                        font.bold: true
                        font.pixelSize: 20
                    }
                }
            }
            spacing: 5

            section.property: "rIsPackageRole"
            section.delegate: sectionHeading
        }
    }

    Component.onCompleted: {
        pkgModel.getUpdates(true, true)
    }
}
