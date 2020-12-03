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
#    usminhibit.cpp
#    usminhibit.h
    ../tools/ukuipower.cpp
    ../tools/powerprovider.cpp
)

#find_package(QT5XDG )
find_package(PkgConfig)
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
include_directories(${GSETTINGS_QT_INCLUDES})
#find_package(PkgConfig)

add_executable(ukui-session ${ukui-session_SRCS}
               ${PROJECT_SOURCE_DIR}/data/music.qrc)
target_link_libraries(ukui-session Qt5::Widgets Qt5::Core Qt5::DBus Qt5::Multimedia KF5::IdleTime ${GSETTINGS_QT_LIBRARIES})
#${GSETTINGS_QT_LIBRARIES}

install(TARGETS ukui-session DESTINATION bin)
