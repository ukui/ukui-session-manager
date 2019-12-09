# Populate a CMake variable with the sources
set(ukui-session_SRCS
    main.cpp
    sessionapplication.cpp
    modulemanager.cpp
    ukuimodule.cpp
    sessiondbusadaptor.h
    ../tools/ukuipower.cpp
    ../tools/powerprovider.cpp
)

find_package(QT5XDG )
#find_package(PkgConfig)
#pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
#include_directories(${GSETTINGS_QT_INCLUDES})

add_executable(ukui-session ${ukui-session_SRCS})
target_link_libraries(ukui-session Qt5::Widgets Qt5::Core Qt5Xdg)
#${GSETTINGS_QT_LIBRARIES}

install(TARGETS ukui-session DESTINATION bin)
