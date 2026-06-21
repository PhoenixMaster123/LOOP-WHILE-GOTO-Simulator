import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import TheoIDE.Persistence

ToolBar {
    id: root
    background: Rectangle {
        height: 48
        color: ApplicationSettings.background
    }

    property bool autoStepEnabled: false
    signal stepToNextBreakpointTriggered
    signal stepTriggered

    Action {
        id: stepNextAction
        text: qsTr("Step to Next Breakpoint")
        icon.name: "skip_next"
    }

    Action {
        id: stepAction
        text: qsTr("Step")
        icon.name: "step"
    }

    Action {
        id: restartAction
        text: qsTr("Restart")
        icon.name: "replay"
    }

    Action {
        id: stopAction
        text: qsTr("Stop")
        icon.name: "stop"
    }

    Action {
        id: enableAutoStepAction
        text: qsTr("Enable Auto Step")
        icon.name: "autoplay"
    }

    Action {
        id: disableAutoStepAction
        text: qsTr("Disable Auto Step")
        icon.name: "autopause"
    }

    RowLayout {
        spacing: 0

        ToolButton {
            action: stepNextAction
            display: AbstractButton.IconOnly
            icon.color: ApplicationSettings.foreground
        }
        ToolButton {
            action: stepAction
            display: AbstractButton.IconOnly
            icon.color: ApplicationSettings.foreground
        }
        ToolButton {
            action: root.autoStepEnabled ? disableAutoStepAction : enableAutoStepAction
            display: AbstractButton.IconOnly
            icon.color: ApplicationSettings.foreground
        }
        ToolButton {
            action: restartAction
            display: AbstractButton.IconOnly
            icon.color: ApplicationSettings.foreground
        }
        ToolButton {
            action: stopAction
            display: AbstractButton.IconOnly
            icon.color: ApplicationSettings.foreground
        }
    }
}
