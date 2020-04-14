#pkg_check_modules(X11 REQUIRED x11)
#pkg_check_modules(XCB REQUIRED xcb)

#include_directories(
#    ${X11_INCLUDE_DIRS}
#    ${XCB_INCLUDE_DIRS}
#    )

#set(EXTRA_LIBS
#    ${X11_LIBRARIES}
#    ${XCB_LIBRARIES}
#    )

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
)

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
    -lxcb
    -lX11
    -lXtst
)

install(TARGETS
        ukui-session-tools
        DESTINATION bin)
