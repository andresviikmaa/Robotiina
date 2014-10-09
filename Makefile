# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

# Default target executed when no arguments are given to make.
default_target: all
.PHONY : default_target

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
CMAKE_SOURCE_DIR = /home/master/Robotiina

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/master/Robotiina

#=============================================================================
# Targets provided globally by CMake.

# Special rule for the target edit_cache
edit_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running interactive CMake command-line interface..."
	/usr/bin/cmake -i .
.PHONY : edit_cache

# Special rule for the target edit_cache
edit_cache/fast: edit_cache
.PHONY : edit_cache/fast

# Special rule for the target rebuild_cache
rebuild_cache:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --cyan "Running CMake to regenerate build system..."
	/usr/bin/cmake -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)
.PHONY : rebuild_cache

# Special rule for the target rebuild_cache
rebuild_cache/fast: rebuild_cache
.PHONY : rebuild_cache/fast

# The main all target
all: cmake_check_build_system
	$(CMAKE_COMMAND) -E cmake_progress_start /home/master/Robotiina/CMakeFiles /home/master/Robotiina/CMakeFiles/progress.marks
	$(MAKE) -f CMakeFiles/Makefile2 all
	$(CMAKE_COMMAND) -E cmake_progress_start /home/master/Robotiina/CMakeFiles 0
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
# Target rules for targets named robotiina

# Build rule for target.
robotiina: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 robotiina
.PHONY : robotiina

# fast build rule for target.
robotiina/fast:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/build
.PHONY : robotiina/fast

#=============================================================================
# Target rules for targets named wheelemulator

# Build rule for target.
wheelemulator: cmake_check_build_system
	$(MAKE) -f CMakeFiles/Makefile2 wheelemulator
.PHONY : wheelemulator

# fast build rule for target.
wheelemulator/fast:
	$(MAKE) -f CMakeFiles/wheelemulator.dir/build.make CMakeFiles/wheelemulator.dir/build
.PHONY : wheelemulator/fast

Robotiina.o: Robotiina.cpp.o
.PHONY : Robotiina.o

# target to build an object file
Robotiina.cpp.o:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/Robotiina.cpp.o
.PHONY : Robotiina.cpp.o

Robotiina.i: Robotiina.cpp.i
.PHONY : Robotiina.i

# target to preprocess a source file
Robotiina.cpp.i:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/Robotiina.cpp.i
.PHONY : Robotiina.cpp.i

Robotiina.s: Robotiina.cpp.s
.PHONY : Robotiina.s

# target to generate assembly for a file
Robotiina.cpp.s:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/Robotiina.cpp.s
.PHONY : Robotiina.cpp.s

autocalibrator.o: autocalibrator.cpp.o
.PHONY : autocalibrator.o

# target to build an object file
autocalibrator.cpp.o:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/autocalibrator.cpp.o
.PHONY : autocalibrator.cpp.o

autocalibrator.i: autocalibrator.cpp.i
.PHONY : autocalibrator.i

# target to preprocess a source file
autocalibrator.cpp.i:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/autocalibrator.cpp.i
.PHONY : autocalibrator.cpp.i

autocalibrator.s: autocalibrator.cpp.s
.PHONY : autocalibrator.s

# target to generate assembly for a file
autocalibrator.cpp.s:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/autocalibrator.cpp.s
.PHONY : autocalibrator.cpp.s

calibrationconfreader.o: calibrationconfreader.cpp.o
.PHONY : calibrationconfreader.o

# target to build an object file
calibrationconfreader.cpp.o:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/calibrationconfreader.cpp.o
.PHONY : calibrationconfreader.cpp.o

calibrationconfreader.i: calibrationconfreader.cpp.i
.PHONY : calibrationconfreader.i

# target to preprocess a source file
calibrationconfreader.cpp.i:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/calibrationconfreader.cpp.i
.PHONY : calibrationconfreader.cpp.i

calibrationconfreader.s: calibrationconfreader.cpp.s
.PHONY : calibrationconfreader.s

# target to generate assembly for a file
calibrationconfreader.cpp.s:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/calibrationconfreader.cpp.s
.PHONY : calibrationconfreader.cpp.s

camera.o: camera.cpp.o
.PHONY : camera.o

# target to build an object file
camera.cpp.o:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/camera.cpp.o
.PHONY : camera.cpp.o

camera.i: camera.cpp.i
.PHONY : camera.i

# target to preprocess a source file
camera.cpp.i:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/camera.cpp.i
.PHONY : camera.cpp.i

camera.s: camera.cpp.s
.PHONY : camera.s

# target to generate assembly for a file
camera.cpp.s:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/camera.cpp.s
.PHONY : camera.cpp.s

colorcalibrator.o: colorcalibrator.cpp.o
.PHONY : colorcalibrator.o

# target to build an object file
colorcalibrator.cpp.o:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/colorcalibrator.cpp.o
.PHONY : colorcalibrator.cpp.o

colorcalibrator.i: colorcalibrator.cpp.i
.PHONY : colorcalibrator.i

# target to preprocess a source file
colorcalibrator.cpp.i:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/colorcalibrator.cpp.i
.PHONY : colorcalibrator.cpp.i

colorcalibrator.s: colorcalibrator.cpp.s
.PHONY : colorcalibrator.s

# target to generate assembly for a file
colorcalibrator.cpp.s:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/colorcalibrator.cpp.s
.PHONY : colorcalibrator.cpp.s

dialog.o: dialog.cpp.o
.PHONY : dialog.o

# target to build an object file
dialog.cpp.o:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/dialog.cpp.o
.PHONY : dialog.cpp.o

dialog.i: dialog.cpp.i
.PHONY : dialog.i

# target to preprocess a source file
dialog.cpp.i:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/dialog.cpp.i
.PHONY : dialog.cpp.i

dialog.s: dialog.cpp.s
.PHONY : dialog.s

# target to generate assembly for a file
dialog.cpp.s:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/dialog.cpp.s
.PHONY : dialog.cpp.s

objectfinder.o: objectfinder.cpp.o
.PHONY : objectfinder.o

# target to build an object file
objectfinder.cpp.o:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/objectfinder.cpp.o
.PHONY : objectfinder.cpp.o

objectfinder.i: objectfinder.cpp.i
.PHONY : objectfinder.i

# target to preprocess a source file
objectfinder.cpp.i:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/objectfinder.cpp.i
.PHONY : objectfinder.cpp.i

objectfinder.s: objectfinder.cpp.s
.PHONY : objectfinder.s

# target to generate assembly for a file
objectfinder.cpp.s:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/objectfinder.cpp.s
.PHONY : objectfinder.cpp.s

remotecontrol.o: remotecontrol.cpp.o
.PHONY : remotecontrol.o

# target to build an object file
remotecontrol.cpp.o:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/remotecontrol.cpp.o
.PHONY : remotecontrol.cpp.o

remotecontrol.i: remotecontrol.cpp.i
.PHONY : remotecontrol.i

# target to preprocess a source file
remotecontrol.cpp.i:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/remotecontrol.cpp.i
.PHONY : remotecontrol.cpp.i

remotecontrol.s: remotecontrol.cpp.s
.PHONY : remotecontrol.s

# target to generate assembly for a file
remotecontrol.cpp.s:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/remotecontrol.cpp.s
.PHONY : remotecontrol.cpp.s

robot.o: robot.cpp.o
.PHONY : robot.o

# target to build an object file
robot.cpp.o:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/robot.cpp.o
.PHONY : robot.cpp.o

robot.i: robot.cpp.i
.PHONY : robot.i

# target to preprocess a source file
robot.cpp.i:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/robot.cpp.i
.PHONY : robot.cpp.i

robot.s: robot.cpp.s
.PHONY : robot.s

# target to generate assembly for a file
robot.cpp.s:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/robot.cpp.s
.PHONY : robot.cpp.s

stillcamera.o: stillcamera.cpp.o
.PHONY : stillcamera.o

# target to build an object file
stillcamera.cpp.o:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/stillcamera.cpp.o
.PHONY : stillcamera.cpp.o

stillcamera.i: stillcamera.cpp.i
.PHONY : stillcamera.i

# target to preprocess a source file
stillcamera.cpp.i:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/stillcamera.cpp.i
.PHONY : stillcamera.cpp.i

stillcamera.s: stillcamera.cpp.s
.PHONY : stillcamera.s

# target to generate assembly for a file
stillcamera.cpp.s:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/stillcamera.cpp.s
.PHONY : stillcamera.cpp.s

wheel.o: wheel.cpp.o
.PHONY : wheel.o

# target to build an object file
wheel.cpp.o:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/wheel.cpp.o
.PHONY : wheel.cpp.o

wheel.i: wheel.cpp.i
.PHONY : wheel.i

# target to preprocess a source file
wheel.cpp.i:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/wheel.cpp.i
.PHONY : wheel.cpp.i

wheel.s: wheel.cpp.s
.PHONY : wheel.s

# target to generate assembly for a file
wheel.cpp.s:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/wheel.cpp.s
.PHONY : wheel.cpp.s

wheelcontroller.o: wheelcontroller.cpp.o
.PHONY : wheelcontroller.o

# target to build an object file
wheelcontroller.cpp.o:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/wheelcontroller.cpp.o
.PHONY : wheelcontroller.cpp.o

wheelcontroller.i: wheelcontroller.cpp.i
.PHONY : wheelcontroller.i

# target to preprocess a source file
wheelcontroller.cpp.i:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/wheelcontroller.cpp.i
.PHONY : wheelcontroller.cpp.i

wheelcontroller.s: wheelcontroller.cpp.s
.PHONY : wheelcontroller.s

# target to generate assembly for a file
wheelcontroller.cpp.s:
	$(MAKE) -f CMakeFiles/robotiina.dir/build.make CMakeFiles/robotiina.dir/wheelcontroller.cpp.s
.PHONY : wheelcontroller.cpp.s

wheelemulator.o: wheelemulator.cpp.o
.PHONY : wheelemulator.o

# target to build an object file
wheelemulator.cpp.o:
	$(MAKE) -f CMakeFiles/wheelemulator.dir/build.make CMakeFiles/wheelemulator.dir/wheelemulator.cpp.o
.PHONY : wheelemulator.cpp.o

wheelemulator.i: wheelemulator.cpp.i
.PHONY : wheelemulator.i

# target to preprocess a source file
wheelemulator.cpp.i:
	$(MAKE) -f CMakeFiles/wheelemulator.dir/build.make CMakeFiles/wheelemulator.dir/wheelemulator.cpp.i
.PHONY : wheelemulator.cpp.i

wheelemulator.s: wheelemulator.cpp.s
.PHONY : wheelemulator.s

# target to generate assembly for a file
wheelemulator.cpp.s:
	$(MAKE) -f CMakeFiles/wheelemulator.dir/build.make CMakeFiles/wheelemulator.dir/wheelemulator.cpp.s
.PHONY : wheelemulator.cpp.s

# Help Target
help:
	@echo "The following are some of the valid targets for this Makefile:"
	@echo "... all (the default if no target is provided)"
	@echo "... clean"
	@echo "... depend"
	@echo "... edit_cache"
	@echo "... rebuild_cache"
	@echo "... robotiina"
	@echo "... wheelemulator"
	@echo "... Robotiina.o"
	@echo "... Robotiina.i"
	@echo "... Robotiina.s"
	@echo "... autocalibrator.o"
	@echo "... autocalibrator.i"
	@echo "... autocalibrator.s"
	@echo "... calibrationconfreader.o"
	@echo "... calibrationconfreader.i"
	@echo "... calibrationconfreader.s"
	@echo "... camera.o"
	@echo "... camera.i"
	@echo "... camera.s"
	@echo "... colorcalibrator.o"
	@echo "... colorcalibrator.i"
	@echo "... colorcalibrator.s"
	@echo "... dialog.o"
	@echo "... dialog.i"
	@echo "... dialog.s"
	@echo "... objectfinder.o"
	@echo "... objectfinder.i"
	@echo "... objectfinder.s"
	@echo "... remotecontrol.o"
	@echo "... remotecontrol.i"
	@echo "... remotecontrol.s"
	@echo "... robot.o"
	@echo "... robot.i"
	@echo "... robot.s"
	@echo "... stillcamera.o"
	@echo "... stillcamera.i"
	@echo "... stillcamera.s"
	@echo "... wheel.o"
	@echo "... wheel.i"
	@echo "... wheel.s"
	@echo "... wheelcontroller.o"
	@echo "... wheelcontroller.i"
	@echo "... wheelcontroller.s"
	@echo "... wheelemulator.o"
	@echo "... wheelemulator.i"
	@echo "... wheelemulator.s"
.PHONY : help



#=============================================================================
# Special targets to cleanup operation of make.

# Special rule to run CMake to check the build system integrity.
# No rule that depends on this can have commands that come from listfiles
# because they might be regenerated.
cmake_check_build_system:
	$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) --check-build-system CMakeFiles/Makefile.cmake 0
.PHONY : cmake_check_build_system

