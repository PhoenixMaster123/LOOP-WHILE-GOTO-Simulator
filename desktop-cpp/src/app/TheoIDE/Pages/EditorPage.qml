import QtCore
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Layouts
import TheoIDE.Controls
import TheoIDE.Persistence

Page {
    id: root

    required property EditorModel model

    states: [
        State {
            name: "debugging"
            when: root.model.executionState !== ExecutionState.Idle && root.model.runningMode === EditorModel.Debug
            extend: "running"
            PropertyChanges {
                secondaryActionButton {
                    action: stopExecutionAction
                }
                primaryActionButton {
                    action: runScriptAction
                }
            }
        },
        State {
            name: "running"
            when: root.model.executionState !== ExecutionState.Idle && root.model.runningMode === EditorModel.Default
            PropertyChanges {
                primaryActionButton {
                    action: stopExecutionAction
                }
                openFileAction {
                    enabled: false
                }
                runScriptAction {
                    enabled: false
                }
                debugScriptAction {
                    enabled: false
                }
                createFileAction {
                    enabled: false
                }
            }
        }
    ]

    Action {
        id: openFileAction
        icon.name: "file_open"
        text: qsTr("Open File")
        onTriggered: fileOpenDialog.open()
    }

    Action {
        id: stopExecutionAction
        icon.name: "stop"
        text: qsTr("Stop Execution")
        onTriggered: root.model.stopExecution()
    }

    Action {
        id: saveAllFilesAction
        icon.name: "save"
        text: qsTr("Save All")
        shortcut: StandardKey.Save
        onTriggered: root.model.saveAllTabs()
    }

    Action {
        id: runScriptAction
        icon.name: "play_arrow"
        text: qsTr("Run")
        shortcut: "F12"
        onTriggered: root.model.runScript()
    }

    Action {
        id: debugScriptAction
        icon.name: "bug_report"
        text: qsTr("Debug")
        shortcut: "F11"
        onTriggered: root.model.runScriptInDebugMode()
    }

    Action {
        id: openMoreActionsMenuAction
        icon.name: "more_vert"
        text: qsTr("Open More Actions")
        onTriggered: moreActionsMenu.open()
    }

    Action {
        id: createFileAction
        icon.name: "add"
        text: qsTr("Create File")
        onTriggered: root.model.createNewTab()
    }

    Action {
        id: openSettingsAction
        icon.name: "settings"
        text: qsTr("Open Settings")
    }

    header: ToolBar {
        id: headerToolBar

        height: 56
        leftPadding: 14
        rightPadding: showMoreActionMenu ? 0 : 14

        readonly property bool showMoreActionMenu: width < 480

        RowLayout {
            anchors.fill: parent

            readonly property int toolTipDelay: 1000

            ToolButton {
                action: createFileAction
                display: AbstractButton.IconOnly
                visible: !headerToolBar.showMoreActionMenu
                ToolTip.visible: hovered
                ToolTip.text: text
                ToolTip.delay: parent.toolTipDelay
                icon.color: ApplicationSettings.foreground
            }
            ToolButton {
                action: openFileAction
                display: AbstractButton.IconOnly
                visible: !headerToolBar.showMoreActionMenu
                ToolTip.visible: hovered
                ToolTip.text: text
                ToolTip.delay: parent.toolTipDelay
                icon.color: ApplicationSettings.foreground
            }
            ToolButton {
                action: saveAllFilesAction
                display: AbstractButton.IconOnly
                visible: !headerToolBar.showMoreActionMenu
                ToolTip.visible: hovered
                ToolTip.text: text
                ToolTip.delay: parent.toolTipDelay
                icon.color: ApplicationSettings.foreground
            }

            Label {
                Layout.fillWidth: true
                elide: Label.ElideRight
                font.pixelSize: 20
                font.weight: Font.DemiBold
                horizontalAlignment: headerToolBar.showMoreActionMenu ? Qt.AlignLeft : Qt.AlignHCenter
                text: qsTr("Theo IDE")
                verticalAlignment: Qt.AlignVCenter
                color: ApplicationSettings.foreground
            }

            // Primary Action
            ToolButton {
                id: primaryActionButton
                action: runScriptAction
                display: AbstractButton.IconOnly
                ToolTip.visible: hovered
                ToolTip.text: text
                ToolTip.delay: parent.toolTipDelay
                icon.color: ApplicationSettings.foreground
            }
            // Secondary Action
            ToolButton {
                id: secondaryActionButton
                action: debugScriptAction
                display: AbstractButton.IconOnly
                ToolTip.visible: hovered
                ToolTip.text: text
                ToolTip.delay: parent.toolTipDelay
                icon.color: ApplicationSettings.foreground
            }

            ToolButton {
                action: openSettingsAction
                display: AbstractButton.IconOnly
                ToolTip.visible: hovered
                ToolTip.text: text
                ToolTip.delay: parent.toolTipDelay
                visible: !headerToolBar.showMoreActionMenu
                icon.color: ApplicationSettings.foreground
            }

            ToolButton {
                id: openMoreActionsMenuButton
                action: openMoreActionsMenuAction
                display: AbstractButton.IconOnly
                visible: headerToolBar.showMoreActionMenu
                icon.color: ApplicationSettings.foreground
            }
        }
    }

    Menu {
        id: moreActionsMenu
        x: openMoreActionsMenuButton.x
        y: openMoreActionsMenuButton.y

        MenuItem {
            action: createFileAction
        }
        MenuItem {
            action: openFileAction
        }
        MenuItem {
            action: saveAllFilesAction
        }
        MenuItem {
            action: openSettingsAction
        }
    }

    SplitView {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        orientation: width < 900 ? Qt.Vertical : Qt.Horizontal

        CodeEditor {
            id: codeEditor
            SplitView.fillWidth: true
            SplitView.fillHeight: true
            model: root.model
        }

        ExecutionStatePanel {
            id: codeExecutionStatePanel
            SplitView.preferredWidth: implicitWidth
            SplitView.preferredHeight: 200
            model: root.model
        }
    }

    FileDialog {
        id: fileOpenDialog
        nameFilters: ["Theo IDE Scripts (*.theo)", "Text Files (*.txt)", "All Files (*)"]
        acceptLabel: qsTr("Open")
        rejectLabel: qsTr("Cancel")
        Component.onCompleted: {
            const documentsLocations = StandardPaths.standardLocations(StandardPaths.DocumentsLocation);
            currentFolder = documentsLocations.length > 0 ? documentsLocations[0] : undefined;
        }
        fileMode: FileDialog.OpenFiles
        onAccepted: {
            currentFiles.forEach(fileUrl => {
                root.model.openFile(fileUrl);
            });
        }
    }
}
