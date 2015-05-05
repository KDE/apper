import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Item {
    id: root
    height: contentGrid.height + 20
    width: ListView.view.width

    property int maxColSize: root.width - iconImage.width - actionBt.width - 40

    GridLayout {
        id: contentGrid
        anchors.centerIn: parent
        width: parent.width - 20
        columns: 4

        Item {
            Layout.rowSpan: 3
            id: iconImage
            height: nameLabel.height + versionLabel.height * 2
            width: height

            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                source: roleIcon.length ? roleIcon : "image://icon/applications-other"
                asynchronous: true
            }
        }

        Label {
            Layout.columnSpan: 2
            id: nameLabel
            elide: Text.ElideRight
            font.pointSize: versionLabel.font.pointSize * 1.3
            text: roleName
        }

        Button {
            id: actionBt
            Layout.rowSpan: 3
            Layout.alignment: Qt.AlignVCenter
            text: qsTr("Remove")
        }

        Label {
            Layout.preferredWidth: width
            width: installedView.versionCol
            id: versionLabel
            elide: Text.ElideRight
            text: roleArch === "all" ? roleVersion : roleVersion + " / " + roleArch
        }

        Label {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.rowSpan: 2
            elide: Text.ElideRight
            text: roleSummary
            wrapMode: Text.WordWrap
        }

        Item {}

        Label {
            Layout.preferredWidth: width
            width: installedView.versionCol
            id: repoLabel
            elide: Text.ElideRight
            text: roleRepo
        }

        Component.onCompleted: {
            var width = Math.max(versionLabel.implicitWidth, repoLabel.implicitWidth)
            if (width > installedView.versionCol) {
                if (width > maxColSize) {
                    installedView.versionCol = maxColSize
                } else {
                    installedView.versionCol = width
                }
            }
        }
    }

    SystemPalette { id: sysPalette }
    Rectangle {
        z: -1
        anchors.centerIn: parent
        height: parent.height - 5
        width: parent.width - 10
        color: root.ListView.isCurrentItem ? sysPalette.highlight : sysPalette.alternateBase

        MouseArea {
            z: 1
            anchors.fill: parent
            onClicked: {
                console.debug(index)
                root.ListView.currentIndex = index
            }
        }
    }
}
