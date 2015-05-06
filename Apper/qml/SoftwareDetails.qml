import QtQuick 2.1
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.0

Item {
    id: name

    property string iconName

    ScrollView {
        GridLayout {
            Item {
                Layout.rowSpan: 3
                id: iconImage
                height: nameLabel.height + versionLabel.height * 2
                width: height

                Image {
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    source: iconName.length ? iconName : "image://icon/applications-other"
                    asynchronous: true
                }
            }
        }
    }
}
