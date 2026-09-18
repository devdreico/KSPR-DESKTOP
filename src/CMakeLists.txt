set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(SOURCES
    main.cpp
    mainwindow.cpp
    processmanager.cpp
    chatpanel.cpp
    filepanel.cpp
    dashboardpanel.cpp
    installer.cpp
    styles.cpp
    welcomepanel.cpp
    settingspanel.cpp
    notificationmanager.cpp
    loadingoverlay.cpp
    searchbar.cpp
)

set(HEADERS
    mainwindow.h
    processmanager.h
    chatpanel.h
    filepanel.h
    dashboardpanel.h
    installer.h
    styles.h
    welcomepanel.h
    settingspanel.h
    notificationmanager.h
    loadingoverlay.h
    searchbar.h
)

set(RESOURCES
    ${CMAKE_SOURCE_DIR}/resources/resources.qrc
)

qt_add_executable(ksrp-desktop
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_link_libraries(ksrp-desktop PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Network
)

set_target_properties(ksrp-desktop PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.ksrp.desktop"
    MACOSX_BUNDLE_BUNDLE_NAME "KSPR Desktop"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_ICON_FILE "desktop-app-icon.png"
)

if(APPLE)
    set_target_properties(ksrp-desktop PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/resources/Info.plist.in"
    )
endif()
