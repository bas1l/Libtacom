# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.7

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
CMAKE_SOURCE_DIR = /home/pi/Tacom/libtacom

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pi/Tacom/libtacom

# Include any dependencies generated for this target.
include executables/neutral/CMakeFiles/neutral.dir/depend.make

# Include the progress variables for this target.
include executables/neutral/CMakeFiles/neutral.dir/progress.make

# Include the compile flags for this target's objects.
include executables/neutral/CMakeFiles/neutral.dir/flags.make

executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.o: executables/neutral/CMakeFiles/neutral.dir/flags.make
executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.o: executables/neutral/neutral.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pi/Tacom/libtacom/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.o"
	cd /home/pi/Tacom/libtacom/executables/neutral && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/neutral.dir/neutral.cpp.o -c /home/pi/Tacom/libtacom/executables/neutral/neutral.cpp

executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/neutral.dir/neutral.cpp.i"
	cd /home/pi/Tacom/libtacom/executables/neutral && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pi/Tacom/libtacom/executables/neutral/neutral.cpp > CMakeFiles/neutral.dir/neutral.cpp.i

executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/neutral.dir/neutral.cpp.s"
	cd /home/pi/Tacom/libtacom/executables/neutral && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pi/Tacom/libtacom/executables/neutral/neutral.cpp -o CMakeFiles/neutral.dir/neutral.cpp.s

executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.o.requires:

.PHONY : executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.o.requires

executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.o.provides: executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.o.requires
	$(MAKE) -f executables/neutral/CMakeFiles/neutral.dir/build.make executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.o.provides.build
.PHONY : executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.o.provides

executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.o.provides.build: executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.o


# Object files for target neutral
neutral_OBJECTS = \
"CMakeFiles/neutral.dir/neutral.cpp.o"

# External object files for target neutral
neutral_EXTERNAL_OBJECTS =

executables/neutral/neutral: executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.o
executables/neutral/neutral: executables/neutral/CMakeFiles/neutral.dir/build.make
executables/neutral/neutral: liblibad5383.a
executables/neutral/neutral: executables/neutral/CMakeFiles/neutral.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/pi/Tacom/libtacom/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable neutral"
	cd /home/pi/Tacom/libtacom/executables/neutral && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/neutral.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
executables/neutral/CMakeFiles/neutral.dir/build: executables/neutral/neutral

.PHONY : executables/neutral/CMakeFiles/neutral.dir/build

executables/neutral/CMakeFiles/neutral.dir/requires: executables/neutral/CMakeFiles/neutral.dir/neutral.cpp.o.requires

.PHONY : executables/neutral/CMakeFiles/neutral.dir/requires

executables/neutral/CMakeFiles/neutral.dir/clean:
	cd /home/pi/Tacom/libtacom/executables/neutral && $(CMAKE_COMMAND) -P CMakeFiles/neutral.dir/cmake_clean.cmake
.PHONY : executables/neutral/CMakeFiles/neutral.dir/clean

executables/neutral/CMakeFiles/neutral.dir/depend:
	cd /home/pi/Tacom/libtacom && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pi/Tacom/libtacom /home/pi/Tacom/libtacom/executables/neutral /home/pi/Tacom/libtacom /home/pi/Tacom/libtacom/executables/neutral /home/pi/Tacom/libtacom/executables/neutral/CMakeFiles/neutral.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : executables/neutral/CMakeFiles/neutral.dir/depend
