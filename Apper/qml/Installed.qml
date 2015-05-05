import QtQuick 2.1
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.0

import Apper 1.0

ScrollView {
    id: root

    ListView {
        id: installedView
        width: root.viewport.width

        property int versionCol: 100

        Component {
            id: sectionHeading
            Rectangle {
                width: ListView.view.width
                height: childrenRect.height
                color: "lightsteelblue"

                Text {
                    text: section === "f" ? qsTr("Applications") : qsTr("System Packages")
                    font.bold: true
                    font.pixelSize: 20
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

        section.property: "roleIsPkg"
        section.criteria: ViewSection.FirstCharacter
        section.delegate: sectionHeading
    }

    Component.onCompleted: {
        pkgModel.getInstalled()
    }
}
