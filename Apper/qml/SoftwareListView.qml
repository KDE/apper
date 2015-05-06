import QtQuick 2.1
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.0

import Apper 1.0

GridView {
    id: softwareListView

    focus: true

    property int columns: root.viewport.width / cellWidth

    SystemPalette { id: sysPalette }

    Component {
        id: sectionHeading
        Item {
            visible: pkgModel.packageCount && pkgModel.applicationCount
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
        viewColumns: columns
        onCountChanged: {
            if (pkgModel.packageCount && !pkgModel.applicationCount) {
                showPackages = true
            }
        }
    }

    delegate: UpdateDelegate {}

    cellHeight: 50
    cellWidth: 200

    footer: sectionHeading
    Component.onCompleted: {
        pkgModel.getInstalled()
    }
}
