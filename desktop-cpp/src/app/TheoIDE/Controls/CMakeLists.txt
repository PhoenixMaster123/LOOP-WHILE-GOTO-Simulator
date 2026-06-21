qt_add_library(TheoIDEControls)

qt_add_qml_module(
    TheoIDEControls
    VERSION 1.0
    QML_FILES
        ApplicationMessageDialog.qml
        ClosableTabButton.qml
        CodeEditor.qml
        DebugControls.qml
        ExecutionStatePanel.qml
        LabeledControl.qml
        LineInfoCell.qml
        LineInfoColumn.qml
        PlainTextEditor.qml
        ResultTable.qml
        TabContent.qml
        TableCell.qml
    URI TheoIDE.Controls
    IMPORTS
        QtQuick
        QtQuick.Controls.Material
        TheoIDE.Persistence
        TheoIDE.Models
    SOURCES
        editormodel.cpp include/editormodel.hpp
        lineinfocolumnmodel.cpp include/lineinfocolumnmodel.hpp
        messagedialogmodel.cpp include/messagedialogmodel.hpp
        variablesstatemodel.cpp include/variablesstatemodel.hpp
        executionstate.cpp include/executionstate.hpp
)

target_include_directories(
    TheoIDEControls
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/
)

target_link_libraries(
    TheoIDEControls
    PUBLIC
        TheoIDEModels
        TheoIDEPersistence
        Qt::Concurrent
        Qt::Core
        Qt::Qml
        Qt::Quick
        Qt::QuickControls2
)
