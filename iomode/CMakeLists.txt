cmake_minimum_required(VERSION 2.8)

# Name the project
project(iomode)

# Include the 'include' of the project itself
include_directories("include")

# The file(GLOB...) allows for wildcard additions
file(GLOB SRCS
	"include/*.h"
    "src/*.cpp")

# Add an executable target called ConvexHull to be build from 
# the source files.
add_executable(iomode ${SRCS})

# Install the executeable program in the bin folder of the
# current project directory.
install(TARGETS iomode DESTINATION ${CMAKE_SOURCE_DIR}/bin)
