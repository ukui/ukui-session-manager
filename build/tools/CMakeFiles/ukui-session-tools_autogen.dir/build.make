# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/tanjing/work/ukui-session-manager/ukui-session-manager

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/tanjing/work/ukui-session-manager/ukui-session-manager/build

# Utility rule file for ukui-session-tools_autogen.

# Include the progress variables for this target.
include tools/CMakeFiles/ukui-session-tools_autogen.dir/progress.make

tools/CMakeFiles/ukui-session-tools_autogen:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/tanjing/work/ukui-session-manager/ukui-session-manager/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Automatic MOC and UIC for target ukui-session-tools"
	cd /home/tanjing/work/ukui-session-manager/ukui-session-manager/build/tools && /usr/bin/cmake -E cmake_autogen /home/tanjing/work/ukui-session-manager/ukui-session-manager/build/tools/CMakeFiles/ukui-session-tools_autogen.dir/AutogenInfo.json RelWithDebInfo

ukui-session-tools_autogen: tools/CMakeFiles/ukui-session-tools_autogen
ukui-session-tools_autogen: tools/CMakeFiles/ukui-session-tools_autogen.dir/build.make

.PHONY : ukui-session-tools_autogen

# Rule to build all files generated by this target.
tools/CMakeFiles/ukui-session-tools_autogen.dir/build: ukui-session-tools_autogen

.PHONY : tools/CMakeFiles/ukui-session-tools_autogen.dir/build

tools/CMakeFiles/ukui-session-tools_autogen.dir/clean:
	cd /home/tanjing/work/ukui-session-manager/ukui-session-manager/build/tools && $(CMAKE_COMMAND) -P CMakeFiles/ukui-session-tools_autogen.dir/cmake_clean.cmake
.PHONY : tools/CMakeFiles/ukui-session-tools_autogen.dir/clean

tools/CMakeFiles/ukui-session-tools_autogen.dir/depend:
	cd /home/tanjing/work/ukui-session-manager/ukui-session-manager/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/tanjing/work/ukui-session-manager/ukui-session-manager /home/tanjing/work/ukui-session-manager/ukui-session-manager/tools /home/tanjing/work/ukui-session-manager/ukui-session-manager/build /home/tanjing/work/ukui-session-manager/ukui-session-manager/build/tools /home/tanjing/work/ukui-session-manager/ukui-session-manager/build/tools/CMakeFiles/ukui-session-tools_autogen.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : tools/CMakeFiles/ukui-session-tools_autogen.dir/depend

