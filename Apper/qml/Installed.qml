import QtQuick 2.1
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.0

import Apper 1.0

Item {

    ScrollView {
        id: root
        anchors.fill: parent

        SoftwareListView {
            id: view
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: (root.viewport.width % cellWidth) / 2
            onWidthChanged: {
                console.debug((width % cellWidth))
            }
        }

    }

}
