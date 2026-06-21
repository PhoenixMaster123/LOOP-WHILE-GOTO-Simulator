import QtQuick
import QtQuick.Controls
import TheoIDE.Persistence

ScrollView {
    id: root

    property alias textDocument: textEdit.textDocument
    property int innerMargin: 0
    property alias cursorPosition: textEdit.cursorPosition
    property alias contentY: flickable.contentY
    property alias text: textEdit.text

    Flickable {
        id: flickable
        anchors.fill: parent
        clip: true
        contentWidth: width - leftMargin - rightMargin
        contentHeight: height - topMargin - bottomMargin
        leftMargin: root.innerMargin
        rightMargin: root.innerMargin
        topMargin: root.innerMargin
        bottomMargin: root.innerMargin

        TextEdit {
            id: textEdit
            anchors.fill: parent
            wrapMode: TextEdit.NoWrap
            color: ApplicationSettings.foreground
            selectionColor: ApplicationSettings.accent
            font {
                family: ApplicationSettings.editorFontFamily
                pointSize: ApplicationSettings.editorFontSize
            }
            onCursorRectangleChanged: flickable.scrollIntoView(cursorRectangle)
            tabStopDistance: ApplicationSettings.tabStopDistance

            // Workaround to prevent the keyboard from closing on Android
            Keys.priority: Keys.BeforeItem
            Keys.onEnterPressed: function (event) {
                insertNewline(event);
            }
            Keys.onReturnPressed: function (event) {
                insertNewline(event);
            }

            Keys.onTabPressed: function (event) {
                insertTab();
                event.accepted = true;
            }

            function insertTab(): void {
                if (ApplicationSettings.expandTabsToSpaces) {
                    for (let i = 0; i < ApplicationSettings.tabStopDistanceInSpaces; ++i) {
                        textEdit.insert(textEdit.cursorPosition, " ");
                    }
                } else {
                    textEdit.insert(textEdit.cursorPosition, "\t");
                }
            }

            function insertNewline(event: KeyEvent): void {
                textEdit.insert(textEdit.cursorPosition, "\n");
                event.accepted = true;
                textEdit.focus = true;
            }
        }

        function scrollIntoView(r: rect): void {
            const contentRight = contentX + width - root.innerMargin;
            const cursorRight = r.x + r.width;
            if (contentX >= r.x) {
                contentX = r.x;
            } else if (contentRight <= cursorRight) {
                contentX = cursorRight - width + root.innerMargin;
            }
            const contentBottom = contentY + height - root.innerMargin;
            const cursorBottom = r.y + r.height;
            if (contentY >= r.y) {
                contentY = r.y;
            } else if (contentBottom <= cursorBottom) {
                contentY = cursorBottom - height + root.innerMargin;
            }
        }

        Binding on contentWidth {
            value: textEdit.implicitWidth
            when: flickable.width < textEdit.implicitWidth
        }
        Binding on contentHeight {
            value: textEdit.implicitHeight
            when: flickable.height < textEdit.implicitHeight
        }
    }
}
