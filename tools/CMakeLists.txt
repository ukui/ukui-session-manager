set(session-tools_SRCS
    main.cpp
    ukuipower.cpp
    powerprovider.cpp
    powerwindow.h
    powerwindow.cpp
    pw.cpp
    pw.h
)

add_executable(ukui-session-tools ${session-tools_SRCS})
target_link_libraries(ukui-session-tools Qt5::Widgets Qt5::Core Qt5::Gui Qt5::DBus)
qt5_add_resources(qrc_FILES image.qrc)

install(TARGETS
        ukui-session-tools
        DESTINATION bin)
