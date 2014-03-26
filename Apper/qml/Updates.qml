import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

import org.packagekit 1.0

ScrollView {
    anchors.fill: parent

    ListView {
        id: updatesView
        delegate: UpdateDelegate {
            width: ListView.view.width
        }
        model: PackageModel {
            id: pkgModel
        }
    }

    Component.onCompleted: {
        pkgModel.getUpdates(true, true)
    }
}
