# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.17

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Disable VCS-based implicit rules.
% : %,v


# Disable VCS-based implicit rules.
% : RCS/%


# Disable VCS-based implicit rules.
% : RCS/%,v


# Disable VCS-based implicit rules.
% : SCCS/s.%


# Disable VCS-based implicit rules.
% : s.%


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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.17.0_1/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.17.0_1/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/wangmiaomiao/coding/compile/assign1/ast-interpreter

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/wangmiaomiao/coding/compile/assign1/ast-interpreter

# Include any dependencies generated for this target.
include CMakeFiles/ast-interpreter.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ast-interpreter.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ast-interpreter.dir/flags.make

CMakeFiles/ast-interpreter.dir/ASTInterpreter.cpp.o: CMakeFiles/ast-interpreter.dir/flags.make
CMakeFiles/ast-interpreter.dir/ASTInterpreter.cpp.o: ASTInterpreter.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/wangmiaomiao/coding/compile/assign1/ast-interpreter/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ast-interpreter.dir/ASTInterpreter.cpp.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ast-interpreter.dir/ASTInterpreter.cpp.o -c /Users/wangmiaomiao/coding/compile/assign1/ast-interpreter/ASTInterpreter.cpp

CMakeFiles/ast-interpreter.dir/ASTInterpreter.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ast-interpreter.dir/ASTInterpreter.cpp.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/wangmiaomiao/coding/compile/assign1/ast-interpreter/ASTInterpreter.cpp > CMakeFiles/ast-interpreter.dir/ASTInterpreter.cpp.i

CMakeFiles/ast-interpreter.dir/ASTInterpreter.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ast-interpreter.dir/ASTInterpreter.cpp.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/wangmiaomiao/coding/compile/assign1/ast-interpreter/ASTInterpreter.cpp -o CMakeFiles/ast-interpreter.dir/ASTInterpreter.cpp.s

# Object files for target ast-interpreter
ast__interpreter_OBJECTS = \
"CMakeFiles/ast-interpreter.dir/ASTInterpreter.cpp.o"

# External object files for target ast-interpreter
ast__interpreter_EXTERNAL_OBJECTS =

ast-interpreter: CMakeFiles/ast-interpreter.dir/ASTInterpreter.cpp.o
ast-interpreter: CMakeFiles/ast-interpreter.dir/build.make
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangAST.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangBasic.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangFrontend.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangTooling.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangFrontend.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangParse.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangDriver.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangSerialization.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangSema.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangEdit.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangAnalysis.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangASTMatchers.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangFormat.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangToolingInclusions.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangToolingCore.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangAST.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangRewrite.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangLex.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libclangBasic.a
ast-interpreter: /usr/local/Cellar/llvm/10.0.1_1/lib/libLLVM.dylib
ast-interpreter: CMakeFiles/ast-interpreter.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/wangmiaomiao/coding/compile/assign1/ast-interpreter/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ast-interpreter"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ast-interpreter.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ast-interpreter.dir/build: ast-interpreter

.PHONY : CMakeFiles/ast-interpreter.dir/build

CMakeFiles/ast-interpreter.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ast-interpreter.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ast-interpreter.dir/clean

CMakeFiles/ast-interpreter.dir/depend:
	cd /Users/wangmiaomiao/coding/compile/assign1/ast-interpreter && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/wangmiaomiao/coding/compile/assign1/ast-interpreter /Users/wangmiaomiao/coding/compile/assign1/ast-interpreter /Users/wangmiaomiao/coding/compile/assign1/ast-interpreter /Users/wangmiaomiao/coding/compile/assign1/ast-interpreter /Users/wangmiaomiao/coding/compile/assign1/ast-interpreter/CMakeFiles/ast-interpreter.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ast-interpreter.dir/depend

