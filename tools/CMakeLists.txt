set(tools_SRCS
    main.cpp
    ukuipower.cpp
    powerprovider.cpp
    mainwindow.h
    mainwindow.cpp
    grab-x11.cpp
    grab-x11.h
    xeventmonitor.cpp
    xeventmonitor.h
    mainwindow.ui
    window.cpp
    window.h
)

find_package(PkgConfig)
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
include_directories(${GSETTINGS_QT_INCLUDES})

add_executable(ukui-session-tools
               ${tools_SRCS}
               ${PROJECT_SOURCE_DIR}/data/images.qrc
               ${PROJECT_SOURCE_DIR}/data/qss.qrc
               ${PROJECT_SOURCE_DIR}/data/music.qrc
               )

target_link_libraries(ukui-session-tools
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::DBus
    Qt5::Multimedia
    Qt5::X11Extras
    ${GSETTINGS_QT_LIBRARIES}
    -lxcb
    -lX11
    -lXtst
)

install(TARGETS
        ukui-session-tools
        DESTINATION bin)
