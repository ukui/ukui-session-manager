file(GLOB SESSION_FILES_IN ukui.desktop)

install(FILES ${SESSION_FILES_IN} DESTINATION "${CMAKE_INSTALL_PREFIX}/share/xsessions")

install(FILES org.ukui.session.gschema.xml DESTINATION "${CMAKE_INSTALL_PREFIX}/share/glib-2.0/schemas")

install(FILES logout.wav weakup.wav startup.wav shutdown.wav DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ukui/ukui-session-manager")

install(DIRECTORY images DESTINATION "${CMAKE_INSTALL_PREFIX}/share/ukui/ukui-session-manager")
