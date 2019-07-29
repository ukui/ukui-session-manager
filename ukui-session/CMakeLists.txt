# Populate a CMake variable with the sources
set(ukui-session_SRCS
    main.cpp
    sessionapplication.cpp
    modulemanager.cpp
    ukuimodule.cpp
    sessiondbusadaptor.h
    ../session-tools/ukuipower.cpp
    ../session-tools/powerprovider.cpp
)

find_package(QT5XDG)
add_executable(ukui-session ${ukui-session_SRCS})
target_link_libraries(ukui-session Qt5::Widgets Qt5::Core Qt5Xdg)
