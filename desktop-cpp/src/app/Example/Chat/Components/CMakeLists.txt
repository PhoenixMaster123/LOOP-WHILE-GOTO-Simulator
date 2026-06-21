set(CPP_SOURCES
    sqlcontactmodel.cpp
    sqlcontactmodel.hpp
    sqlconversationmodel.cpp
    sqlconversationmodel.hpp
)

set(QML_SOURCES ContactPage.qml ConversationPage.qml)

set(IMAGES
    assets/images/Albert_Einstein.png
    assets/images/Ernest_Hemingway.png
    assets/images/Hans_Gude.png
)

qt_add_library(ExampleChatComponents)
qt_add_qml_module(ExampleChatComponents
    URI Example.Chat.Components
    VERSION 1.0
    QML_FILES ${QML_SOURCES}
    SOURCES ${CPP_SOURCES}
    RESOURCES ${IMAGES}
    IMPORTS
        QtQuick
        QtQuick.Controls
)

target_link_libraries(ExampleChatComponents PRIVATE Qt::Core Qt::Quick Qt::Sql)
