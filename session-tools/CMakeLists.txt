set(session-tools_SRCS
    main.cpp
    toolswindow.cpp
    ukuipower.cpp
    powerprovider.cpp
)

add_executable(session-tools ${session-tools_SRCS})
target_link_libraries(session-tools Qt5::Widgets Qt5::Core Qt5::Gui Qt5::DBus)
