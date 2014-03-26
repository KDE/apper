import QtQuick 2.1

Item {
    Loader {
        id: pageLoader
        anchors.fill: parent
        source: model.page
        asynchronous: true
        visible: status == Loader.Ready
    }

    Text {
        visible: !pageLoader.visible
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: pageLoader.status == Loader.Loading ? qsTr("Loading") : qsTr("Failed to load page: %1").arg(model.page)
    }
}
