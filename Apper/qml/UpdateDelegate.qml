import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

GridLayout {
    columns: 5
    rowSpacing: 0
    CheckBox {
        Layout.rowSpan: 2
        id: checkBox
        checked: model.roleChecked
        onClicked: pkgModel.toggleSelection(roleId)
        Rectangle {
            anchors.fill: parent
            z: -1
            color: "blue"
        }
    }
    Label {
        Layout.columnSpan: 4
        text: roleSummary
    }

    Label {
        text: roleName + "/" + roleRepo
    }
    Label {
        text: roleVersion
    }
    Label {
        text: roleArch
    }
    Item {
        Layout.fillWidth: true
    }
}

