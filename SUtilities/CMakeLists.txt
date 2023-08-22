# CMakeList.txt : CMake project for SUtilities, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Add source to this project's executable.
set(SRC_DIR "${PROJECT_SOURCE_DIR}/SUtilities/src")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/SUtilities/include")
# add_executable (SUtilities "SUtilities.cpp" "SUtilities.h"  "include/TypeList.hpp" "include/Visitor.hpp" "include/Visitable.hpp" "include/Cast.hpp" "include/Singleton.hpp")
add_subdirectory (test)

# TODO: Add tests and install targets if needed.
