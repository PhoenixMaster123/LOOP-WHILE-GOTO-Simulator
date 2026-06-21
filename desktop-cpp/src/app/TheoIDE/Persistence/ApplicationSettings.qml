pragma Singleton

import QtCore
import QtQuick
import QtQuick.Controls.Material
import TheoIDE.Persistence

Item {
    id: root

    Settings {
        id: themeSettings
        category: "Theme"
        property string accentDark: "#d65d0e"
        property string backgroundDark: "#282828"
        property string foregroundDark: "#ebdbb2"
        property string primaryDark: "#1d2021"
        property string currentLineBackgroundDark: "#3c3836"
        property string accentLight: "#d65d0e"
        property string backgroundLight: "#fbf1c7"
        property string foregroundLight: "#3c3836"
        property string primaryLight: "#ebdbb2"
        property string currentLineBackgroundLight: "#ebdbb2"
        property int theme: Material.System
    }

    property alias accentDark: themeSettings.accentDark
    property alias backgroundDark: themeSettings.backgroundDark
    property alias foregroundDark: themeSettings.foregroundDark
    property alias primaryDark: themeSettings.primaryDark
    property alias currentLineBackgroundDark: themeSettings.currentLineBackgroundDark
    property alias accentLight: themeSettings.accentLight
    property alias backgroundLight: themeSettings.backgroundLight
    property alias foregroundLight: themeSettings.foregroundLight
    property alias primaryLight: themeSettings.primaryLight
    property alias currentLineBackgroundLight: themeSettings.currentLineBackgroundLight
    property alias theme: themeSettings.theme

    Material.theme: theme

    readonly property color accent: Material.theme === Material.Dark ? accentDark : accentLight
    readonly property color background: Material.theme === Material.Dark ? backgroundDark : backgroundLight
    readonly property color foreground: Material.theme === Material.Dark ? foregroundDark : foregroundLight
    readonly property color primary: Material.theme === Material.Dark ? primaryDark : primaryLight
    readonly property color currentLineBackground: Material.theme === Material.Dark ? currentLineBackgroundDark : currentLineBackgroundLight

    Settings {
        id: fontSettings
        category: "Font"
        property int uiFontSize: SystemFontConfiguraton.defaultFontSize
        property int editorFontSize: SystemFontConfiguraton.defaultMonospaceFontSize
        property string editorFontFamily: SystemFontConfiguraton.defaultMonospaceFont.family
    }

    property alias uiFontSize: fontSettings.uiFontSize
    property alias editorFontSize: fontSettings.editorFontSize
    property alias editorFontFamily: fontSettings.editorFontFamily
    readonly property font editorFont: ({
            family: editorFontFamily,
            pointSize: editorFontSize
        })

    Settings {
        id: generalSettings
        category: "General"
        property int maxReadFileSizeBytes: 2 * 1024 * 1024
        property int blockSizeBytes: 1024
        property int tabStopDistanceInSpaces: 3
        property bool expandTabsToSpaces: false
        property int compilationTimeoutMs: 5000
        property int executionTimeoutMs: 5000
    }

    property alias maxReadFileSizeBytes: generalSettings.maxReadFileSizeBytes
    property alias blockSizeBytes: generalSettings.blockSizeBytes
    property alias tabStopDistanceInSpaces: generalSettings.tabStopDistanceInSpaces
    property alias expandTabsToSpaces: generalSettings.expandTabsToSpaces
    property alias compilationTimeoutMs: generalSettings.compilationTimeoutMs
    property alias executionTimeoutMs: generalSettings.executionTimeoutMs

    readonly property int tabStopDistance: SystemFontConfiguraton.calculateSpaceWidthOfFont(editorFont) * tabStopDistanceInSpaces

    Settings {
        id: windowSettings
        category: "Window"
        property int width: 1000
        property int height: 1000
    }

    property alias windowWidth: windowSettings.width
    property alias windowHeight: windowSettings.height
}
