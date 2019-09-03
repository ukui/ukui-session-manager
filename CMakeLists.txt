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

# Enable funcion names and line numbers even for release builds
add_definitions("-DQT_MESSAGELOGCONTEXT")

# Find the QtWidgets library
find_package(Qt5 COMPONENTS Widgets Core DBus REQUIRED)

# For debug: cmake -DCMAKE_BUILD_TYPE=debug
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_DEBUG "-g -O0")

add_subdirectory(ukui-session)
add_subdirectory(tools)
add_subdirectory(data)
