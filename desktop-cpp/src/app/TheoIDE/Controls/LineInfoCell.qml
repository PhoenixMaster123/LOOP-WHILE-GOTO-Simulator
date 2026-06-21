import QtQuick.Controls.Material
import QtQuick
import TheoIDE.Persistence

Control {
    id: root
    required property int lineNumber
    required property bool breakPointActive
    required property real lineHeight
    required property var model

    property color breakPointActiveBackgroundColor: ApplicationSettings.accent
    property color breakPointActiveHoverBackgroundColor: Qt.lighter(ApplicationSettings.accent, 1.1)
    property color defaultBackgroundColor: "transparent"
    property color hoverBackgroundColor: Material.theme === Material.Dark ? "#19ffffff" : "#19000000"
    hoverEnabled: false
    implicitWidth: Math.max(leftInset + implicitBackgroundWidth + rightInset, leftPadding + implicitContentWidth + rightPadding)
    implicitHeight: Math.max(topInset + implicitBackgroundHeight + bottomInset, topPadding + implicitContentHeight + bottomPadding)
    height: lineHeight

    function toggleBreakpoint() {
        model.breakPointActive = !breakPointActive;
    }

    background: Rectangle {
        id: numberHighlight
        color: root.defaultBackgroundColor
        radius: height / 2
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            onClicked: root.toggleBreakpoint()
            hoverEnabled: true
            property bool hovered: false
            onEntered: hovered = true
            onExited: hovered = false
        }
        height: numberText.implicitHeight
    }

    contentItem: Text {
        id: numberText
        text: root.lineNumber
        leftPadding: implicitHeight / 2
        rightPadding: leftPadding
        color: Qt.tint(ApplicationSettings.foreground, Qt.alpha(ApplicationSettings.background, 0.4))
        font {
            pointSize: ApplicationSettings.editorFontSize
            family: ApplicationSettings.editorFontFamily
        }
    }

    states: [
        State {
            name: "breakPointActiveAndHovered"
            extend: "breakPointActive"
            when: root.breakPointActive && mouseArea.hovered
            changes: [
                PropertyChanges {
                    numberHighlight.color: root.breakPointActiveHoverBackgroundColor
                }
            ]
        },
        State {
            name: "breakPointActive"
            when: root.breakPointActive
            changes: [
                PropertyChanges {
                    numberHighlight.color: root.breakPointActiveBackgroundColor
                },
                PropertyChanges {
                    numberText.color: ApplicationSettings.background
                }
            ]
        },
        State {
            name: "hovered"
            when: mouseArea.hovered
            changes: PropertyChanges {
                numberHighlight.color: root.hoverBackgroundColor
            }
        }
    ]
    transitions: [
        Transition {
            ParallelAnimation {
                ColorAnimation {
                    duration: 100
                    target: numberHighlight
                    property: "color"
                }
                ColorAnimation {
                    duration: 100
                    target: numberText
                    property: "color"
                }
            }
        }
    ]
}
