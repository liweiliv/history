# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Default target executed when no arguments are given to make.
default_target: all

.PHONY : default_target

# Allow only one "make -f Makefile2" at a time, but pass parallelism.
.NOTPARALLEL:


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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/liwei/history

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/liwei/history

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake cache editor..."
	/usr/bin/ccmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache

.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/local/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache

.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/liwei/history/CMakeFiles /home/liwei/history/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/liwei/history/CMakeFiles 0
.PHONY : all

# The main clean target
clean:
	$(MAKE) -f CMakeFiles/Makefile2 clean
.PHONY : clean

# The main clean target
clean/fast: clean

.PHONY : clean/fast

# Prepare targets for installation.
preinstall: all
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall

# Prepare targets for installation.
preinstall/fast:
	$(MAKE) -f CMakeFiles/Makefile2 preinstall
.PHONY : preinstall/fast

# clear depends
depend:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 1
.PHONY : depend

#=============================================================================
# Target rules for targets named World

# Build rule for target.
World: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 World
.PHONY : World

# fast build rule for target.
World/fast:
	$(MAKE) -f CMakeFiles/World.dir/build.make CMakeFiles/World.dir/build
.PHONY : World/fast

#=============================================================================
# Target rules for targets named Gengin

# Build rule for target.
Gengin: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 Gengin
.PHONY : Gengin

# fast build rule for target.
Gengin/fast:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/build
.PHONY : Gengin/fast

#=============================================================================
# Target rules for targets named memlib

# Build rule for target.
memlib: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 memlib
.PHONY : memlib

# fast build rule for target.
memlib/fast:
	$(MAKE) -f CMakeFiles/memlib.dir/build.make CMakeFiles/memlib.dir/build
.PHONY : memlib/fast

#=============================================================================
# Target rules for targets named util

# Build rule for target.
util: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 util
.PHONY : util

# fast build rule for target.
util/fast:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/build
.PHONY : util/fast

g/consciousness.o: g/consciousness.cpp.o

.PHONY : g/consciousness.o

# target to build an object file
g/consciousness.cpp.o:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/consciousness.cpp.o
.PHONY : g/consciousness.cpp.o

g/consciousness.i: g/consciousness.cpp.i

.PHONY : g/consciousness.i

# target to preprocess a source file
g/consciousness.cpp.i:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/consciousness.cpp.i
.PHONY : g/consciousness.cpp.i

g/consciousness.s: g/consciousness.cpp.s

.PHONY : g/consciousness.s

# target to generate assembly for a file
g/consciousness.cpp.s:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/consciousness.cpp.s
.PHONY : g/consciousness.cpp.s

g/g_map.o: g/g_map.cpp.o

.PHONY : g/g_map.o

# target to build an object file
g/g_map.cpp.o:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/g_map.cpp.o
.PHONY : g/g_map.cpp.o

g/g_map.i: g/g_map.cpp.i

.PHONY : g/g_map.i

# target to preprocess a source file
g/g_map.cpp.i:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/g_map.cpp.i
.PHONY : g/g_map.cpp.i

g/g_map.s: g/g_map.cpp.s

.PHONY : g/g_map.s

# target to generate assembly for a file
g/g_map.cpp.s:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/g_map.cpp.s
.PHONY : g/g_map.cpp.s

g/g_point.o: g/g_point.cpp.o

.PHONY : g/g_point.o

# target to build an object file
g/g_point.cpp.o:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/g_point.cpp.o
.PHONY : g/g_point.cpp.o

g/g_point.i: g/g_point.cpp.i

.PHONY : g/g_point.i

# target to preprocess a source file
g/g_point.cpp.i:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/g_point.cpp.i
.PHONY : g/g_point.cpp.i

g/g_point.s: g/g_point.cpp.s

.PHONY : g/g_point.s

# target to generate assembly for a file
g/g_point.cpp.s:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/g_point.cpp.s
.PHONY : g/g_point.cpp.s

g/g_sharp.o: g/g_sharp.cpp.o

.PHONY : g/g_sharp.o

# target to build an object file
g/g_sharp.cpp.o:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/g_sharp.cpp.o
.PHONY : g/g_sharp.cpp.o

g/g_sharp.i: g/g_sharp.cpp.i

.PHONY : g/g_sharp.i

# target to preprocess a source file
g/g_sharp.cpp.i:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/g_sharp.cpp.i
.PHONY : g/g_sharp.cpp.i

g/g_sharp.s: g/g_sharp.cpp.s

.PHONY : g/g_sharp.s

# target to generate assembly for a file
g/g_sharp.cpp.s:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/g_sharp.cpp.s
.PHONY : g/g_sharp.cpp.s

g/world.o: g/world.cpp.o

.PHONY : g/world.o

# target to build an object file
g/world.cpp.o:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/world.cpp.o
.PHONY : g/world.cpp.o

g/world.i: g/world.cpp.i

.PHONY : g/world.i

# target to preprocess a source file
g/world.cpp.i:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/world.cpp.i
.PHONY : g/world.cpp.i

g/world.s: g/world.cpp.s

.PHONY : g/world.s

# target to generate assembly for a file
g/world.cpp.s:
	$(MAKE) -f CMakeFiles/Gengin.dir/build.make CMakeFiles/Gengin.dir/g/world.cpp.s
.PHONY : g/world.cpp.s

main.o: main.cpp.o

.PHONY : main.o

# target to build an object file
main.cpp.o:
	$(MAKE) -f CMakeFiles/World.dir/build.make CMakeFiles/World.dir/main.cpp.o
.PHONY : main.cpp.o

main.i: main.cpp.i

.PHONY : main.i

# target to preprocess a source file
main.cpp.i:
	$(MAKE) -f CMakeFiles/World.dir/build.make CMakeFiles/World.dir/main.cpp.i
.PHONY : main.cpp.i

main.s: main.cpp.s

.PHONY : main.s

# target to generate assembly for a file
main.cpp.s:
	$(MAKE) -f CMakeFiles/World.dir/build.make CMakeFiles/World.dir/main.cpp.s
.PHONY : main.cpp.s

util/INIParser.o: util/INIParser.cpp.o

.PHONY : util/INIParser.o

# target to build an object file
util/INIParser.cpp.o:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/INIParser.cpp.o
.PHONY : util/INIParser.cpp.o

util/INIParser.i: util/INIParser.cpp.i

.PHONY : util/INIParser.i

# target to preprocess a source file
util/INIParser.cpp.i:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/INIParser.cpp.i
.PHONY : util/INIParser.cpp.i

util/INIParser.s: util/INIParser.cpp.s

.PHONY : util/INIParser.s

# target to generate assembly for a file
util/INIParser.cpp.s:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/INIParser.cpp.s
.PHONY : util/INIParser.cpp.s

util/Log.o: util/Log.cpp.o

.PHONY : util/Log.o

# target to build an object file
util/Log.cpp.o:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/Log.cpp.o
.PHONY : util/Log.cpp.o

util/Log.i: util/Log.cpp.i

.PHONY : util/Log.i

# target to preprocess a source file
util/Log.cpp.i:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/Log.cpp.i
.PHONY : util/Log.cpp.i

util/Log.s: util/Log.cpp.s

.PHONY : util/Log.s

# target to generate assembly for a file
util/Log.cpp.s:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/Log.cpp.s
.PHONY : util/Log.cpp.s

util/Log_r.o: util/Log_r.cpp.o

.PHONY : util/Log_r.o

# target to build an object file
util/Log_r.cpp.o:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/Log_r.cpp.o
.PHONY : util/Log_r.cpp.o

util/Log_r.i: util/Log_r.cpp.i

.PHONY : util/Log_r.i

# target to preprocess a source file
util/Log_r.cpp.i:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/Log_r.cpp.i
.PHONY : util/Log_r.cpp.i

util/Log_r.s: util/Log_r.cpp.s

.PHONY : util/Log_r.s

# target to generate assembly for a file
util/Log_r.cpp.s:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/Log_r.cpp.s
.PHONY : util/Log_r.cpp.s

util/arena.o: util/arena.cc.o

.PHONY : util/arena.o

# target to build an object file
util/arena.cc.o:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/arena.cc.o
.PHONY : util/arena.cc.o

util/arena.i: util/arena.cc.i

.PHONY : util/arena.i

# target to preprocess a source file
util/arena.cc.i:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/arena.cc.i
.PHONY : util/arena.cc.i

util/arena.s: util/arena.cc.s

.PHONY : util/arena.s

# target to generate assembly for a file
util/arena.cc.s:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/arena.cc.s
.PHONY : util/arena.cc.s

util/hash.o: util/hash.c.o

.PHONY : util/hash.o

# target to build an object file
util/hash.c.o:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/hash.c.o
.PHONY : util/hash.c.o

util/hash.i: util/hash.c.i

.PHONY : util/hash.i

# target to preprocess a source file
util/hash.c.i:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/hash.c.i
.PHONY : util/hash.c.i

util/hash.s: util/hash.c.s

.PHONY : util/hash.s

# target to generate assembly for a file
util/hash.c.s:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/hash.c.s
.PHONY : util/hash.c.s

util/itoa.o: util/itoa.cpp.o

.PHONY : util/itoa.o

# target to build an object file
util/itoa.cpp.o:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/itoa.cpp.o
.PHONY : util/itoa.cpp.o

util/itoa.i: util/itoa.cpp.i

.PHONY : util/itoa.i

# target to preprocess a source file
util/itoa.cpp.i:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/itoa.cpp.i
.PHONY : util/itoa.cpp.i

util/itoa.s: util/itoa.cpp.s

.PHONY : util/itoa.s

# target to generate assembly for a file
util/itoa.cpp.s:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/itoa.cpp.s
.PHONY : util/itoa.cpp.s

util/linux_platform/DL_linux.o: util/linux_platform/DL_linux.c.o

.PHONY : util/linux_platform/DL_linux.o

# target to build an object file
util/linux_platform/DL_linux.c.o:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/linux_platform/DL_linux.c.o
.PHONY : util/linux_platform/DL_linux.c.o

util/linux_platform/DL_linux.i: util/linux_platform/DL_linux.c.i

.PHONY : util/linux_platform/DL_linux.i

# target to preprocess a source file
util/linux_platform/DL_linux.c.i:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/linux_platform/DL_linux.c.i
.PHONY : util/linux_platform/DL_linux.c.i

util/linux_platform/DL_linux.s: util/linux_platform/DL_linux.c.s

.PHONY : util/linux_platform/DL_linux.s

# target to generate assembly for a file
util/linux_platform/DL_linux.c.s:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/linux_platform/DL_linux.c.s
.PHONY : util/linux_platform/DL_linux.c.s

util/linux_platform/file_opt_linux.o: util/linux_platform/file_opt_linux.c.o

.PHONY : util/linux_platform/file_opt_linux.o

# target to build an object file
util/linux_platform/file_opt_linux.c.o:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/linux_platform/file_opt_linux.c.o
.PHONY : util/linux_platform/file_opt_linux.c.o

util/linux_platform/file_opt_linux.i: util/linux_platform/file_opt_linux.c.i

.PHONY : util/linux_platform/file_opt_linux.i

# target to preprocess a source file
util/linux_platform/file_opt_linux.c.i:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/linux_platform/file_opt_linux.c.i
.PHONY : util/linux_platform/file_opt_linux.c.i

util/linux_platform/file_opt_linux.s: util/linux_platform/file_opt_linux.c.s

.PHONY : util/linux_platform/file_opt_linux.s

# target to generate assembly for a file
util/linux_platform/file_opt_linux.c.s:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/linux_platform/file_opt_linux.c.s
.PHONY : util/linux_platform/file_opt_linux.c.s

util/logger.o: util/logger.cpp.o

.PHONY : util/logger.o

# target to build an object file
util/logger.cpp.o:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/logger.cpp.o
.PHONY : util/logger.cpp.o

util/logger.i: util/logger.cpp.i

.PHONY : util/logger.i

# target to preprocess a source file
util/logger.cpp.i:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/logger.cpp.i
.PHONY : util/logger.cpp.i

util/logger.s: util/logger.cpp.s

.PHONY : util/logger.s

# target to generate assembly for a file
util/logger.cpp.s:
	$(MAKE) -f CMakeFiles/util.dir/build.make CMakeFiles/util.dir/util/logger.cpp.s
.PHONY : util/logger.cpp.s

util/memlib/MempRing.o: util/memlib/MempRing.cpp.o

.PHONY : util/memlib/MempRing.o

# target to build an object file
util/memlib/MempRing.cpp.o:
	$(MAKE) -f CMakeFiles/memlib.dir/build.make CMakeFiles/memlib.dir/util/memlib/MempRing.cpp.o
.PHONY : util/memlib/MempRing.cpp.o

util/memlib/MempRing.i: util/memlib/MempRing.cpp.i

.PHONY : util/memlib/MempRing.i

# target to preprocess a source file
util/memlib/MempRing.cpp.i:
	$(MAKE) -f CMakeFiles/memlib.dir/build.make CMakeFiles/memlib.dir/util/memlib/MempRing.cpp.i
.PHONY : util/memlib/MempRing.cpp.i

util/memlib/MempRing.s: util/memlib/MempRing.cpp.s

.PHONY : util/memlib/MempRing.s

# target to generate assembly for a file
util/memlib/MempRing.cpp.s:
	$(MAKE) -f CMakeFiles/memlib.dir/build.make CMakeFiles/memlib.dir/util/memlib/MempRing.cpp.s
.PHONY : util/memlib/MempRing.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... World"
	@echo "... Gengin"
	@echo "... memlib"
	@echo "... util"
	@echo "... g/consciousness.o"
	@echo "... g/consciousness.i"
	@echo "... g/consciousness.s"
	@echo "... g/g_map.o"
	@echo "... g/g_map.i"
	@echo "... g/g_map.s"
	@echo "... g/g_point.o"
	@echo "... g/g_point.i"
	@echo "... g/g_point.s"
	@echo "... g/g_sharp.o"
	@echo "... g/g_sharp.i"
	@echo "... g/g_sharp.s"
	@echo "... g/world.o"
	@echo "... g/world.i"
	@echo "... g/world.s"
	@echo "... main.o"
	@echo "... main.i"
	@echo "... main.s"
	@echo "... util/INIParser.o"
	@echo "... util/INIParser.i"
	@echo "... util/INIParser.s"
	@echo "... util/Log.o"
	@echo "... util/Log.i"
	@echo "... util/Log.s"
	@echo "... util/Log_r.o"
	@echo "... util/Log_r.i"
	@echo "... util/Log_r.s"
	@echo "... util/arena.o"
	@echo "... util/arena.i"
	@echo "... util/arena.s"
	@echo "... util/hash.o"
	@echo "... util/hash.i"
	@echo "... util/hash.s"
	@echo "... util/itoa.o"
	@echo "... util/itoa.i"
	@echo "... util/itoa.s"
	@echo "... util/linux_platform/DL_linux.o"
	@echo "... util/linux_platform/DL_linux.i"
	@echo "... util/linux_platform/DL_linux.s"
	@echo "... util/linux_platform/file_opt_linux.o"
	@echo "... util/linux_platform/file_opt_linux.i"
	@echo "... util/linux_platform/file_opt_linux.s"
	@echo "... util/logger.o"
	@echo "... util/logger.i"
	@echo "... util/logger.s"
	@echo "... util/memlib/MempRing.o"
	@echo "... util/memlib/MempRing.i"
	@echo "... util/memlib/MempRing.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

