import QtQuick.Templates as T
import QtQuick.Controls.impl
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl
import QtQuick.Layouts

T.TabButton {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, implicitContentHeight + topPadding + bottomPadding)

    signal closeTriggered
    property bool closeEnabled: true

    leftPadding: 12
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0
    spacing: 0

    icon.width: 24
    icon.height: 24
    icon.color: !enabled ? Material.hintTextColor : down || checked ? Material.accentColor : Material.foreground

    contentItem: RowLayout {
        height: control.Material.touchTarget

        IconLabel {
            spacing: control.spacing
            mirrored: control.mirrored
            display: control.display
            Layout.minimumWidth: 0
            Layout.fillWidth: true

            icon: control.icon
            text: control.text
            font: control.font
            color: !control.enabled ? control.Material.hintTextColor : control.down || control.checked ? control.Material.accentColor : control.Material.foreground
        }

        ToolButton {
            padding: 0
            icon.name: "close"
            icon.width: 18
            icon.height: 18
            onClicked: control.closeTriggered()
            enabled: control.closeEnabled
            Layout.minimumWidth: width
        }
    }

    background: Ripple {
        implicitHeight: control.Material.touchTarget

        clip: true
        pressed: control.pressed
        anchor: control
        active: enabled && (control.down || control.visualFocus || control.hovered)
        color: control.Material.rippleColor
    }
}
