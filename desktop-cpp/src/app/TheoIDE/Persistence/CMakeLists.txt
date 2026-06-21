qt_add_library(TheoIDEPersistence)

set(QML_FILE_SINGLETONS ApplicationSettings.qml)

set_source_files_properties(
    ${QML_FILE_SINGLETONS}
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_qml_module(
    TheoIDEPersistence
    URI TheoIDE.Persistence
    VERSION 1.0
    QML_FILES ${QML_FILE_SINGLETONS}
    SOURCES
        systemfontconfiguration.cpp include/systemfontconfiguration.hpp
)

target_include_directories(
    TheoIDEPersistence
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/
)

target_link_libraries(
    TheoIDEPersistence
    PRIVATE Qt::Core Qt::Quick Qt::QuickControls2
)
