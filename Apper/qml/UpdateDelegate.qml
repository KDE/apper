import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Item {
    id: root
    height: mainRow.height

    Row {
        id: mainRow
        CheckBox {
            checked: model.rChecked
        }

        Label {
            text: model.name
        }
    }
}
