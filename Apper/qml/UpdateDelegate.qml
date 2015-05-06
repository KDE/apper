import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0

Item {
    id: root
    visible: roleVisible
    height: contentGrid.height + contentGrid.rowSpacing * 2
    width: contentGrid.width + contentGrid.rowSpacing * 2

    property bool isCurrent: GridView.isCurrentItem

    GridLayout {
        id: contentGrid
        anchors.centerIn: parent
        width: actionBt.width * 3 + iconImage.width * 4 + 4 * rowSpacing
        columns: 3

        Item {
            Layout.rowSpan: 3
            id: iconImage
            height: nameLabel.height + versionLabel.height + repoLabel.height + contentGrid.rowSpacing * 2
            width: height

            Image {
                anchors.fill: parent
                fillMode: Image.PreserveAspectFit
                cache: true
                source: roleIcon.length ? roleIcon : "image://icon/applications-other"
                asynchronous: true
            }
        }

        Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            id: nameLabel
            elide: Text.ElideRight
            textFormat: Text.StyledText
            maximumLineCount: 1
            color: isCurrent ? sysPalette.highlightedText : sysPalette.text
            text: "<b>" + roleName + "</b> " + roleSummary
        }

        Label {
            Layout.fillWidth: true
            id: versionLabel
            elide: Text.ElideRight
            maximumLineCount: 1
            color: isCurrent ? sysPalette.highlightedText : sysPalette.text
            text: roleVersion + " / " + roleArch
        }

        Item {
            Layout.rowSpan: 2
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignBottom
            id: actionBt
//            text: qsTr("Remove")
        }

//        Button {
//            Layout.rowSpan: 2
//            Layout.alignment: Qt.AlignBottom
//            id: actionBt
//            text: qsTr("Remove")
//        }

        Label {
            Layout.fillWidth: true
            id: repoLabel
            elide: Text.ElideRight
            color: isCurrent ? sysPalette.highlightedText : sysPalette.text
            text: roleRepo
        }

//        Item {
//            Layout.columnSpan: contentGrid.columns
//            Layout.fillHeight: true
////            Layout.alignment: Qt.AlignBottom
////            id: actionBt
////            text: qsTr("Remove")
//        }
    }

    Component.onCompleted: {
        if (index > 1) {
            return
        }

        if (contentGrid.height > softwareListView.cellHeight) {
            softwareListView.cellHeight = contentGrid.height + contentGrid.rowSpacing * 4
        }

        if (contentGrid.width > softwareListView.cellWidth) {
            softwareListView.cellWidth = contentGrid.width + contentGrid.rowSpacing * 6
        }
    }

    Rectangle {
        z: -1
        anchors.centerIn: parent
        height: parent.height - contentGrid.rowSpacing
        width: parent.width - contentGrid.rowSpacing
        color: isCurrent ? sysPalette.highlight : "transparent"
    }

    MouseArea {
        z: 1
        anchors.fill: contentGrid
        onClicked: {
            softwareListView.currentIndex = index
        }
    }
}
