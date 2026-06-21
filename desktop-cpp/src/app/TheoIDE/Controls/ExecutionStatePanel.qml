import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import TheoIDE.Controls
import TheoIDE.Persistence

Item {
    id: root

    property bool autoStepEnabled: false
    property alias implicitWidth: debugControls.implicitWidth
    required property EditorModel model

    states: [
        State {
            name: "debugging"
            when: root.model.executionState !== ExecutionState.Idle && root.model.runningMode === EditorModel.Debug
            extend: "running"
        },
        State {
            name: "running"
            when: root.model.executionState !== ExecutionState.Idle && root.model.runningMode === EditorModel.Default
            PropertyChanges {
                mainTabComboBox {
                    enabled: false
                }
            }
        }
    ]

    Column {
        id: debugControlsContainer
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 0

        Flickable {
            id: debugControlsFlickable
            anchors.left: parent.left
            anchors.right: parent.right
            height: debugControls.height
            contentWidth: debugControls.width
            contentHeight: debugControls.height
            clip: true

            DebugControls {
                id: debugControls
                leftPadding: 8
                rightPadding: 8
                width: implicitWidth
                height: implicitHeight
            }
        }

        Rectangle {
            id: toolBarSeparator
            anchors.left: parent.left
            anchors.right: parent.right
            color: ApplicationSettings.primary
            height: 1
        }
    }

    Rectangle {
        id: mainContentBackground
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: debugControlsContainer.bottom
        anchors.bottom: parent.bottom
        color: ApplicationSettings.primary
    }

    ScrollView {
        id: contentScrollView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: debugControlsContainer.bottom
        anchors.bottom: parent.bottom
        clip: true
        contentWidth: availableWidth
        contentHeight: mainContent.implicitHeight
        z: 1

        Column {
            id: mainContent
            anchors.left: parent.left
            anchors.right: parent.right
            spacing: 16
            readonly property int contentWidth: width - leftPadding - rightPadding
            padding: 16

            LabeledControl {
                text: qsTr("Auto Step Speed")
                width: parent.contentWidth
                spacing: 0
                RowLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    Label {
                        text: qsTr("Slow")
                    }
                    Slider {
                        Layout.fillWidth: true
                        from: 0.1
                        to: 2
                        value: 1
                    }
                    Label {
                        text: qsTr("Fast")
                    }
                }
            }

            LabeledControl {
                text: qsTr("Main Script")
                width: parent.contentWidth
                ComboBox {
                    id: mainTabComboBox
                    model: root.model
                    anchors.left: parent.left
                    anchors.right: parent.right
                    textRole: "tabName"
                    enabled: count > 0

                    function updateMainTabIndex(index: int): void {
                        root.model.mainTabIndex = index;
                    }
                    function updateCurrentIndex(index: int): void {
                        currentIndex = root.model.mainTabIndex;
                    }
                    Component.onCompleted: {
                        currentIndex = root.model.mainTabIndex;
                        activated.connect(updateMainTabIndex);
                        root.model.mainTabIndexChanged.connect(updateCurrentIndex);
                    }
                }
            }

            LabeledControl {
                width: parent.contentWidth
                text: qsTr("Output")
                Flickable {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: resultTable.height
                    contentWidth: resultTable.implicitWidth
                    clip: true
                    ResultTable {
                        id: resultTable
                        anchors.right: parent.right
                        height: implicitHeight
                        model: VariablesStateModel {
                            virtualMachineService: root.model.virtualMachineService
                        }
                    }
                }
            }
        }
    }
}
