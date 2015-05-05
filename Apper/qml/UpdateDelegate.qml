import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Item {
    id: root
    height: contentGrid.height + 10
    width: ListView.view.width

    GridLayout {
        id: contentGrid
        anchors.centerIn: parent
        width: parent.width - 10
        columns: 3
        rowSpacing: 0
        CheckBox {
            Layout.rowSpan: 2
            id: checkBox
            checked: model.roleChecked
            onClicked: pkgModel.toggleSelection(roleId)
        }

        Image {
            height: checkBox.height
            width: height
            visible: !rIsPackageRole
            Layout.rowSpan: 2
            source: roleIcon
            asynchronous: true
        }

        RowLayout {
            Label {
                text: roleName
                font.pointSize: font.pointSize * 1.3
            }
            Label {
                Layout.fillWidth: true
                text: roleSummary
            }
        }

        RowLayout {
            Label {
                Layout.fillWidth: true
                text: roleSummary
            }
            Label {
                text: roleVersion
            }
            Label {
                text: roleRepo
            }
            Label {
                text: roleArch
            }
        }
    }

    SystemPalette { id: sysPalette }
    Rectangle {
        z: -1
        anchors.centerIn: parent
        height: parent.height - 10
        width: parent.width - 10
        color: ListView.currentItem ? sysPalette.highlight : sysPalette.alternateBase
    }
}
