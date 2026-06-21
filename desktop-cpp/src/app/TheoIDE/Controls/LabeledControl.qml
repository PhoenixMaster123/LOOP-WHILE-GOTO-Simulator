import QtQuick
import QtQuick.Controls.Material

Column {
    id: root
    spacing: 6
    property alias text: label.text
    Label {
        id: label
        anchors.right: parent.right
        anchors.left: parent.left
    }
}
