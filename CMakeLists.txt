# -------------------------------
# Setup CMake minimum version
# -------------------------------
cmake_minimum_required(VERSION 3.16)

# -------------------------------
# Version
# -------------------------------
SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 1)
SET(VERSION_PATCH 0)

# -------------------------------
# Configurations
# -------------------------------
SET(CONFIGURATION_DEBUG Debug)
SET(CONFIGURATION_RELWITHDEBINFO RelWithDebInfo)
SET(CONFIGURATION_MINSIZEREL Release)
SET(CONFIGURATION_RELEASE Release)

# -------------------------------
# Setup the solution
# -------------------------------
project(fibers VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}"
				  LANGUAGES C;CXX)

message(STATUS "#--------------------- Starting project generation ---------------------")
message("CMake Version: \t\t${CMAKE_VERSION}")
message("Architecture: \t\t${CMAKE_GENERATOR_PLATFORM}")
message("Generating project: \t${PROJECT_NAME} v${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# -------------------------------
# CMake dependencies
# -------------------------------
include(cmake\\precompiled_header.cmake)
include(cmake\\utilities.cmake)
include(cmake\\directories.cmake)

# --------------------------
# Setup global properties
# --------------------------
# Enable folder structure in the IDE
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

SET(WINDOWS Windows)
SET(LINUX Linux)
SET(MACOS MacOS)

SET(BUILD_TARGETS ${WINDOWS} ${LINUX} ${MACOS})

SET(BUILD_TARGET ${WINDOWS} CACHE STRING "Setup solution for the specific build target")
SET_PROPERTY(CACHE BUILD_TARGET PROPERTY STRINGS ${BUILD_TARGETS})

if(BUILD_TARGET STREQUAL ${WINDOWS})
add_compile_definitions("TEST_CODE_WINDOWS")
endif()

# Debug postfix
SET (CMAKE_DEBUG_POSTFIX _d)

# --------------------------------
# Setup compiler features
# --------------------------------
SET (CMAKE_CXX_STANDARD 23)
SET (CMAKE_CXX_STANDARD_REQUIRED ON)
SET (CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "Output directory: \t\t${OUTPUT_BINDIR}")

# --------------------------------
# Add subdirectories
# --------------------------------
add_subdirectory(${SRC_DIR})

message(STATUS "#---------------------- Ending project generation ----------------------")	