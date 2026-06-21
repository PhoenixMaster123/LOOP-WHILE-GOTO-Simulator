pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import Example.Chat.Components

Page {
    id: root

    ListView {
        id: listView

        anchors.fill: parent
        topMargin: 48
        leftMargin: 48
        bottomMargin: 48
        rightMargin: 48
        spacing: 20

        model: SqlContactModel {}

        delegate: ItemDelegate {
            required property string modelData

            text: modelData
            width: listView.width - listView.leftMargin - listView.rightMargin
            height: avatar.implicitHeight
            leftPadding: avatar.implicitWidth + 32
            onClicked: root.StackView.view.push("qrc:/qt/qml/Example/Chat/Components/ConversationPage.qml", {
                "inConversationWith": modelData
            })

            Image {
                id: avatar

                source: "qrc:/qt/qml/Example/Chat/Components/assets/images/" + parent.modelData.replace(" ", "_") + ".png"
            }
        }
    }

    header: ToolBar {
        Label {
            text: qsTr("Contacts")
            font.pixelSize: 20
            anchors.centerIn: parent
        }
    }
}
