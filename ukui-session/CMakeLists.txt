# Populate a CMake variable with the sources
set(ukui-session_SRCS
    main.cpp
    sessionapplication.cpp
    modulemanager.cpp
    ukuimodule.cpp
    idlewatcher.cpp
    sessiondbusadaptor.h
    idlewatcher.cpp
    idlewatcher.h
    idleadbusdaptor.h
    xdgautostart.cpp
    xdgautostart.h
    xdgdesktopfile.cpp
    xdgdesktopfile.h
    xdgdirs.cpp
    xdgdirs.h
    xdgmacros.h
    usminhibit.cpp
    usminhibit.h
    ../tools/ukuipower.cpp
    ../tools/powerprovider.cpp
    ukuismserver.h
    ukuismserver.cpp
    ukuismclient.h
    ukuismclient.cpp
    ukuismconnection.h
    ukuisessiondebug.cpp
    ukuisessiondebug.h
    sessionmanagercontext.h
    sessionmanagercontext.cpp
)

qt5_add_dbus_interface(ukui-session_SRCS org.ukui.KWin.Session.xml ukuikwinsession_interface)

#find_package(QT5XDG )
find_package(PkgConfig)
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
include_directories(${GSETTINGS_QT_INCLUDES})

pkg_search_module(X11 REQUIRED x11)
include_directories(${X11_INCLUDES})

add_executable(ukui-session ${ukui-session_SRCS})
target_link_libraries(ukui-session
    Qt5::Widgets
    Qt5::Core
    Qt5::DBus
    Qt5::Multimedia
    KF5::IdleTime
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::ConfigGui
    KF5::WindowSystem
    -lSM -lICE
    ${GSETTINGS_QT_LIBRARIES} ${X11_LIBRARIES})
#${GSETTINGS_QT_LIBRARIES}

install(TARGETS ukui-session DESTINATION bin)
