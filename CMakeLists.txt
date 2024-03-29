cmake_minimum_required(VERSION 3.1.0)
project(ukui-session-manager)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Handle the Qt rcc code generator automatically.
set(CMAKE_AUTORCC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 11)

include(GNUInstallDirs)

# Enable funcion names and line numbers even for release builds
add_definitions("-DQT_MESSAGELOGCONTEXT")

# Find the QtWidgets library
find_package(Qt5 COMPONENTS Widgets Core Multimedia X11Extras REQUIRED)
find_package(Qt5LinguistTools)
find_package(KF5IdleTime)
find_package(Qt5DBus)
find_package(KF5CoreAddons)
find_package(KF5Config)
find_package(KF5WindowSystem)

# For debug: cmake -DCMAKE_BUILD_TYPE=debug
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")

# i18n
set(UKUI_TRANSLATIONS_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/ukui/translations)
add_definitions(
    -DUKUI_TRANSLATIONS_DIR="${UKUI_TRANSLATIONS_DIR}"
)
if (NOT DEFINED UPDATE_TRANSLATIONS)
    set(UPDATE_TRANSLATIONS "No")
endif()

# To create a new ts file: lupdate -recursive . -target-language en_US -ts translations/en_US.ts
file(GLOB TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts")

# cmake -DUPDATE_TRANSLATIONS=yes
if (UPDATE_TRANSLATIONS)
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()
add_custom_target(translations ALL DEPENDS ${QM_FILES})
install(FILES ${QM_FILES} DESTINATION ${UKUI_TRANSLATIONS_DIR}/${PROJECT_NAME})

add_subdirectory(ukui-session)
add_subdirectory(tools)
add_subdirectory(data)
