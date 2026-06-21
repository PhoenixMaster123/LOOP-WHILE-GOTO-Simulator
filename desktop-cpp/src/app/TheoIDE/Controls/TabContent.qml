import QtQuick
import TheoIDE.Persistence
import TheoIDE.Controls

Item {
    id: root
    property alias currentIndex: lineInfoColumn.currentIndex
    property alias currentItem: lineInfoColumn.currentItem
    readonly property int innerMargin: 8
    property alias text: textEdit.text
    property alias textDocument: textEdit.textDocument
    property int currentLineNumber: 1
    property alias currentCursorPosition: textEdit.cursorPosition

    Rectangle {
        id: currentLineHighlight
        color: ApplicationSettings.currentLineBackground
        anchors.left: parent.left
        anchors.right: parent.right
        height: parent.currentItem ? parent.currentItem.height : 0
        y: lineInfoColumn.currentItem ? lineInfoColumn.currentItem.y - lineInfoColumn.contentY : 0
        z: 0
    }

    LineInfoColumn {
        id: lineInfoColumn
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        leftMargin: root.innerMargin
        rightMargin: root.innerMargin
        bottomMargin: root.innerMargin
        topMargin: root.innerMargin
        width: implicitWidth
        currentIndex: root.currentLineNumber - 1
        model: LineInfoColumnModel {
            id: lineInfoColumnModel
            textDocument: textEdit.textDocument
        }
        contentY: textEdit.contentY
        interactive: false
        z: 1

        readonly property int currentLineIndex: root.currentLineNumber - 1

        function updateCurrentIndex(): void {
            if (currentLineIndex >= count) {
                return;
            }
            currentIndex = currentLineIndex;
        }

        Component.onCompleted: {
            root.currentLineNumberChanged.connect(updateCurrentIndex);
            lineInfoColumn.countChanged.connect(updateCurrentIndex);
        }
    }

    PlainTextEditor {
        id: textEdit
        anchors.top: parent.top
        anchors.left: lineInfoColumn.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        innerMargin: root.innerMargin
        z: 1
    }
}
