set(QML_SOURCES EditorPage.qml)

qt_add_library(TheoIDEPages)
qt_add_qml_module(
    TheoIDEPages
    URI TheoIDE.Pages
    VERSION 1.0
    QML_FILES ${QML_SOURCES}
    IMPORTS
        QtQuick
        QtQuick.Controls.Material
        QtQuick.Dialogs
        QtQuick.Layouts
        TheoIDE.Persistence
        TheoIDE.Controls
)

target_link_libraries(
    TheoIDEPages
    PRIVATE
        Qt::Core
        Qt::Quick
        Qt::QuickControls2
        TheoIDEPersistence
        TheoIDEControls
)
