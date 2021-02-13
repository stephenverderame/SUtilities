# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

macro(external_add filename pkname)
	configure_file(${filename} "${pkname}-download/CMakeLists.txt")
	execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${pkname}-download")
	execute_process(COMMAND "${CMAKE_COMMAND}" --build .
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/${pkname}-download")
	set(${pkname}_SUBDIRS "${CMAKE_BINARY_DIR}/${pkname}-src"
	"${CMAKE_BINARY_DIR}/${pkname}-build")
endmacro() 

#Install googletest
external_add(GTestCMakeLists.txt.in gtest)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${gtest_SUBDIRS})

project ("SUtilities")

enable_testing()
include (CTest)

# Include sub-projects.
add_subdirectory ("SUtilities")
