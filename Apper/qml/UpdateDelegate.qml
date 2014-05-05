import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

RowLayout {
    id: mainRow
    CheckBox {
        Layout.preferredWidth: appsRepeater.col1
        checked: model.roleChecked
        onClicked: pkgModel.toggleSelection(roleId)
        text: roleName
        Component.onCompleted: {
            if (appsRepeater.col1 < implicitWidth) {
                appsRepeater.col1 = implicitWidth
            }
        }
    }
    Label {
        text: roleSummary
    }
}
