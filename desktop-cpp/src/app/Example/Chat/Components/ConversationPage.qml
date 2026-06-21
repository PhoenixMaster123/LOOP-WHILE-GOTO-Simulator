pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Example.Chat.Components

Page {
    id: root

    property string inConversationWith

    ColumnLayout {
        anchors.fill: parent

        ListView {
            id: listView

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: pane.leftPadding + messageField.leftPadding
            displayMarginBeginning: 40
            displayMarginEnd: 40
            verticalLayoutDirection: ListView.BottomToTop
            spacing: 12

            model: SqlConversationModel {
                recipient: root.inConversationWith
            }

            delegate: Column {
                id: messageDelegate

                required property string author
                required property string recipient
                required property string timestamp
                required property string message

                readonly property bool sentByMe: recipient !== "Me"

                anchors.right: sentByMe ? listView.contentItem.right : undefined
                spacing: 6

                Row {
                    id: messageRow

                    spacing: 6
                    anchors.right: messageDelegate.sentByMe ? parent.right : undefined

                    Image {
                        id: avatar

                        source: !messageDelegate.sentByMe ? "qrc:/qt/qml/Example/Chat/Components/assets/images/" + messageDelegate.author.replace(" ", "_") + ".png" : ""
                    }

                    Rectangle {
                        width: Math.min(messageText.implicitWidth + 24, listView.width - (!messageDelegate.sentByMe ? avatar.width + messageRow.spacing : 0))
                        height: messageText.implicitHeight + 24
                        color: messageDelegate.sentByMe ? "lightgray" : "steelblue"

                        Label {
                            id: messageText

                            text: messageDelegate.message
                            color: messageDelegate.sentByMe ? "black" : "white"
                            anchors.fill: parent
                            anchors.margins: 12
                            wrapMode: Label.Wrap
                        }
                    }
                }

                Label {
                    id: timestampText

                    text: Qt.formatDateTime(messageDelegate.timestamp, "d MMM hh:mm")
                    color: "lightgray"
                    anchors.right: messageDelegate.sentByMe ? parent.right : undefined
                }
            }

            ScrollBar.vertical: ScrollBar {}
        }

        Pane {
            id: pane

            Layout.fillWidth: true
            Layout.fillHeight: false

            RowLayout {
                width: parent.width

                TextArea {
                    id: messageField

                    Layout.fillWidth: true
                    placeholderText: qsTr("Compose Message")
                    wrapMode: TextArea.Wrap
                }

                Button {
                    id: sendButton

                    text: qsTr("Send")
                    enabled: messageField.length > 0
                    Layout.fillWidth: false
                    onClicked: {
                        listView.model.sendMessage(root.inConversationWith, messageField.text);
                        messageField.text = "";
                    }
                }
            }
        }
    }

    header: ToolBar {
        ToolButton {
            text: qsTr("Back")
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            onClicked: root.StackView.view.pop()
        }

        Label {
            id: pageTitle

            text: root.inConversationWith
            font.pixelSize: 20
            anchors.centerIn: parent
        }
    }
}
