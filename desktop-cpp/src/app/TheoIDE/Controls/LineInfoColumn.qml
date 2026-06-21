import QtQuick

ListView {
    id: root
    anchors.left: parent.left
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    contentWidth: contentItem.childrenRect.width
    implicitWidth: contentWidth + leftMargin + rightMargin
    clip: true
    delegate: LineInfoCell {
        anchors.right: parent ? parent.right : undefined
    }
    highlightFollowsCurrentItem: false
}
