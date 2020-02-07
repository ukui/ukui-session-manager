set(tools_SRCS
    main.cpp
    ukuipower.cpp
    powerprovider.cpp
    mainwindow.h
    mainwindow.cpp
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
)

install(TARGETS
        ukui-session-tools
        DESTINATION bin)
