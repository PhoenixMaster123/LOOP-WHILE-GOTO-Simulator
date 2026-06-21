pragma ComponentBehavior: Bound
import QtQuick
import TheoIDE.Controls
import TheoIDE.Persistence

Column {
    id: root

    required property VariablesStateModel model
    property int columnSpacing: spacing

    spacing: 2

    Row {
        spacing: root.columnSpacing
        TableCell {
            id: nameHeader
            width: root.model.variableNameColumnWidth
            text: qsTr("Variable Name")
            font {
                pointSize: ApplicationSettings.editorFontSize
                family: ApplicationSettings.editorFontFamily
                bold: true
            }
            Component.onCompleted: {
                root.model.setVariableNameHeaderImplicitWidth(implicitWidth);
            }
        }
        TableCell {
            id: valueHeader
            width: root.model.variableValueColumnWidth
            text: qsTr("Value")
            font {
                pointSize: ApplicationSettings.editorFontSize
                family: ApplicationSettings.editorFontFamily
                bold: true
            }
            Component.onCompleted: {
                root.model.setVariableValueHeaderImplicitWidth(implicitWidth);
            }
        }
    }

    Repeater {
        model: root.model
        delegate: Row {
            id: row
            spacing: root.columnSpacing
            required property string variableName
            required property int variableValue
            required property int variableNameColumnWidth
            required property int variableValueColumnWidth
            required property var model

            TableCell {
                id: nameCell
                text: row.variableName
                width: row.variableNameColumnWidth
                Component.onCompleted: {
                    updateModelImplicitWidth();
                    implicitWidthChanged.connect(updateModelImplicitWidth);
                }
                function updateModelImplicitWidth(): void {
                    row.model.variableNameColumnImplicitWidth = implicitWidth;
                }
            }
            TableCell {
                id: valueCell
                text: row.variableValue
                width: row.variableValueColumnWidth
                function updateModelImplicitWidth(): void {
                    row.model.variableValueColumnImplicitWidth = implicitWidth;
                }
                Component.onCompleted: {
                    updateModelImplicitWidth();
                    implicitWidthChanged.connect(updateModelImplicitWidth);
                }
            }
        }
    }
}
