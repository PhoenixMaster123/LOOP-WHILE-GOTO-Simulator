import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import TheoIDE.Persistence
import TheoIDE.Controls

Item {
    id: root

    required property EditorModel model

    TabBar {
        id: openFilesTabBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        background: Rectangle {
            implicitHeight: 48
            color: ApplicationSettings.background
        }
        height: Math.max(background.implicitHeight, contentItem.implicitHeight)

        property int previousCount: -1

        onCountChanged: {
            if (previousCount < count) {
                setCurrentIndex(count - 1);
            }
            previousCount = count;
        }

        function updateModelCurrentTabIndex(): void {
            root.model.currentTabIndex = currentIndex;
        }
        function updateCurrentIndex(index: int): void {
            currentIndex = index;
        }

        Component.onCompleted: {
            currentIndex = root.model.currentTabIndex;
            root.model.currentTabIndexChanged.connect(updateCurrentIndex);
            currentIndexChanged.connect(updateModelCurrentTabIndex);
            previousCount = count;
        }

        Repeater {
            model: root.model
            delegate: ClosableTabButton {
                required property string displayTabName
                required property bool isReadOnly
                required property var model
                onCloseTriggered: model.open = false
                text: displayTabName
                width: implicitWidth
                closeEnabled: !isReadOnly
            }
        }
    }

    Rectangle {
        id: tabBarEditorSeparator
        anchors.top: openFilesTabBar.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        height: 1
        color: ApplicationSettings.primary
    }

    StackLayout {
        id: tabContentStack
        anchors.top: tabBarEditorSeparator.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: openFilesTabBar.currentIndex

        Repeater {
            model: root.model
            delegate: TabContent {
                required property string storedTabText
                required property var model
                required property int cursorPosition
                required property int cursorLineNumber
                currentCursorPosition: cursorPosition
                onCurrentCursorPositionChanged: {
                    model.cursorPositionEdit = currentCursorPosition;
                }
                currentLineNumber: cursorLineNumber
                text: storedTabText
                Component.onCompleted: {
                    model.textDocument = textDocument;
                    model.cursorPositionEdit = currentCursorPosition;
                }
            }
        }
    }
}
