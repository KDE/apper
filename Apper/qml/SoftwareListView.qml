import QtQuick 2.1
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.0

import Apper 1.0

GridView {
    id: softwareListView

    focus: true

//    property real gridCenter: height * 0.5 - cellWidth / 2
    property bool showPackages: false

    SystemPalette { id: sysPalette }

    Component {
        id: sectionHeading
        Item {
            visible: pkgModel.packageCount
            width: softwareListView.width
            height: childrenRect.height + 5

            CheckBox {
                text: qsTr("Show %1 more system packages").arg(pkgModel.packageCount)
                onCheckedChanged: pkgModel.showPackages = checked
            }
        }
    }

    model: PackageModel {
        id: pkgModel
    }

    delegate: UpdateDelegate {
    }

    cellHeight: 50
    cellWidth: 200

    footer: sectionHeading
    Component.onCompleted: {
        pkgModel.getInstalled()
    }
}
