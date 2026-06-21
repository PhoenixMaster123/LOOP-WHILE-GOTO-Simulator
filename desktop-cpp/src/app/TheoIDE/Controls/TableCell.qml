import QtQuick
import TheoIDE.Persistence
import QtQuick.Controls.Material

Rectangle {
    id: root
    property alias text: tableCellText.text
    property alias horizontalAlignment: tableCellText.horizontalAlignment
    property alias font: tableCellText.font

    color: ApplicationSettings.background
    implicitHeight: tableCellText.implicitHeight
    implicitWidth: tableCellText.implicitWidth
    radius: Material.ExtraSmallScale

    Text {
        id: tableCellText
        anchors.fill: parent
        rightPadding: ApplicationSettings.editorFontSize
        leftPadding: rightPadding
        topPadding: ApplicationSettings.editorFontSize / 2
        bottomPadding: topPadding
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignBottom
        font: ApplicationSettings.editorFont
        color: ApplicationSettings.foreground
    }
}
