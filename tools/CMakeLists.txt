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
    loginedusers.h
    lockchecker.h
    lockchecker.cpp
    loginedusers.cpp
    ../ukui-session/xdgdirs.cpp
    ../ukui-session/xdgdesktopfile.cpp
    myiconlabel.cpp
    myiconlabel.h
    mypushbutton.cpp
    mypushbutton.h
    pushbutton.cpp
    pushbutton.h
    waylandgrabkeys.cpp
    waylandgrabkeys.h
)

find_package(PkgConfig)
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
pkg_check_modules(GIO2 REQUIRED gio-2.0)
include_directories(${GSETTINGS_QT_INCLUDES})
include_directories(${GLIB2_INCLUDE_DIRS})
include_directories(${GIO2_INCLUDE_DIRS})

add_executable(ukui-session-tools
               ${tools_SRCS}
               ${PROJECT_SOURCE_DIR}/data/qss.qrc
               )

target_link_libraries(ukui-session-tools
    Qt5::Widgets
    Qt5::Core
    Qt5::Gui
    Qt5::DBus
    Qt5::Multimedia
    Qt5::X11Extras
    KF5::WaylandClient
    KF5::GlobalAccel
    ${GSETTINGS_QT_LIBRARIES}
    -lxcb
    -lX11
    -lXtst
    -lukui-log4qt
    ${GLIB2_LIBRARIES}
    ${GIO2_LIBRARIES}
)

install(TARGETS
        ukui-session-tools
        DESTINATION bin)
