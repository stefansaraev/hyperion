# Define the main-project name
project(hyperion)

# define the minimum cmake version (as required by cmake)
cmake_minimum_required(VERSION 2.8)

IF ( POLICY CMP0026 )
	CMAKE_POLICY( SET CMP0026 OLD )
ENDIF()


SET ( DEFAULT_AMLOGIC    OFF )
SET ( DEFAULT_FB         OFF )

option(ENABLE_AMLOGIC "Enable the AMLOGIC video grabber" ${DEFAULT_AMLOGIC} )
message(STATUS "ENABLE_AMLOGIC = " ${ENABLE_AMLOGIC})

option(ENABLE_FB "Enable the framebuffer grabber" ${DEFAULT_FB} )
message(STATUS "ENABLE_FB = " ${ENABLE_FB})

# configure a header file to pass some of the CMake settings
# to the source code
configure_file("${PROJECT_SOURCE_DIR}/HyperionConfig.h.in" "${PROJECT_BINARY_DIR}/HyperionConfig.h")
include_directories("${PROJECT_BINARY_DIR}")

# Define the global output path of binaries
SET(LIBRARY_OUTPUT_PATH    ${PROJECT_BINARY_DIR}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

# Add the project include directory as additional include path
include_directories(${CMAKE_SOURCE_DIR}/dependencies/include)
include_directories(${CMAKE_SOURCE_DIR}/include)

# enable C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++11 -Wall")

find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}    ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

add_definitions(${QT_DEFINITIONS})

# Add the source/lib directories
add_subdirectory(dependencies)
add_subdirectory(libsrc)
add_subdirectory(src)
