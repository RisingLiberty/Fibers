# CMake precompiled header macro
# Distributed under the MIT Software License
# Copyright (c) 2015-2017 Borislav Stanimirov
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of 
# this software and associated documentation files (the "Software"), to deal in 
# the Software without restriction, including without limitation the rights to 
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
# of the Software, and to permit persons to whom the Software is furnished to do 
# so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all 
# copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
# SOFTWARE.
#
# Revision history:
#
#  1.0 (2017.08.16) Initial standalone release
#
# add_precompiled_header
#
# Sets a precompiled header for a given target
# Args:
# TARGET_NAME - Name of the target. Only valid after add_library or add_executable
# PCH_TARGET - Header file name to precompile
# PCH_SOURCE_TARGET - MSVC specific source file name. Ignored on other platforms
#
# Example Usage
# add_executable(myproj
#   src/myproj.pch.h
#   src/myproj.pch.cpp
#   src/main.cpp
#   ...
#   src/z.cpp
#   )
# add_precompiled_header(myproj src/myproj.pch.h src/myproj.pch.cpp)
#
macro(add_precompiled_header TARGET_NAME PCH_TARGET PCH_SOURCE_TARGET)
    message(STATUS "Precompiled header generation")
    if(MSVC)
        get_filename_component(PCH_DIRECTORY ${PCH_TARGET} DIRECTORY)
        target_include_directories(${TARGET_NAME} PRIVATE ${PCH_DIRECTORY}) # fixes occasional IntelliSense glitches

        get_filename_component(PCH_TARGET_WE ${PCH_TARGET} NAME_WE)
        get_filename_component(PCH_SOURCE_NAME ${PCH_SOURCE_TARGET} NAME)
        set(PRECOMPILED_BINARY "$(IntDir)/${PCH_TARGET_WE}.pch")
        message(STATUS "\tPrecompiled header: ${PCH_TARGET}")
        message(STATUS "\tPrecompiled source: ${PCH_SOURCE_TARGET}")

        get_target_property(SOURCE_FILES ${TARGET_NAME} SOURCES)
        set(SOURCE_FILE_FOUND FALSE)
        foreach(SOURCE_FILE ${SOURCE_FILES})
            get_filename_component(SOURCE_FILE_NAME ${SOURCE_FILE} NAME)
            if(SOURCE_FILE MATCHES \\.\(c|cc|cxx|cpp\)$)
                if(${PCH_SOURCE_NAME} STREQUAL ${SOURCE_FILE_NAME})
                    # Set source file to generate header
                    set_source_files_properties(
                        ${SOURCE_FILE}
                        PROPERTIES
                        COMPILE_FLAGS "/Yc\"${PCH_TARGET}\" /Fp\"${PRECOMPILED_BINARY}\""
                        OBJECT_OUTPUTS "${PRECOMPILED_BINARY}")
                    set(SOURCE_FILE_FOUND TRUE)
                else()
                    # Set and automatically include precompiled header
                    set_source_files_properties(
                        ${SOURCE_FILE}
                        PROPERTIES
                        COMPILE_FLAGS "/Yu\"${PCH_TARGET}\" /Fp\"${PRECOMPILED_BINARY}\" /FI\"${PCH_TARGET}\""
                        OBJECT_DEPENDS "${PRECOMPILED_BINARY}")
                endif()
            endif()
        endforeach()
        if(SOURCE_FILE_FOUND)
            message(STATUS "\tSuccess")
        else()
            message(FATAL_ERROR "\tA source file for ${PCH_TARGET} was not found. Required for MSVC builds.")
        endif(SOURCE_FILE_FOUND)
    elseif(CMAKE_GENERATOR STREQUAL Xcode)
        set_target_properties(
            ${TARGET_NAME}
            PROPERTIES
            XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${PCH_TARGET}"
            XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES"
            )
    elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Create and set output directory.
        set(OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${PCH_TARGET}.gch")
        make_directory(${OUTPUT_DIR})
        set(OUTPUT_NAME "${OUTPUT_DIR}/${PCH_TARGET}.gch")

    # Export compiler flags via a generator to a response file
        set(PCH_FLAGS_FILE "${OUTPUT_DIR}/${PCH_TARGET}.rsp")
        set(_include_directories "$<TARGET_PROPERTY:${TARGET_NAME},INCLUDE_DIRECTORIES>")
        set(_compile_definitions "$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_DEFINITIONS>")
        set(_compile_flags "$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_FLAGS>")
        set(_compile_options "$<TARGET_PROPERTY:${TARGET_NAME},COMPILE_OPTIONS>")
        set(_include_directories "$<$<BOOL:${_include_directories}>:-I$<JOIN:${_include_directories},\n-I>\n>")
        set(_compile_definitions "$<$<BOOL:${_compile_definitions}>:-D$<JOIN:${_compile_definitions},\n-D>\n>")
        set(_compile_flags "$<$<BOOL:${_compile_flags}>:$<JOIN:${_compile_flags},\n>\n>")
        set(_compile_options "$<$<BOOL:${_compile_options}>:$<JOIN:${_compile_options},\n>\n>")
        file(GENERATE OUTPUT "${PCH_FLAGS_FILE}" CONTENT "${_compile_definitions}${_include_directories}${_compile_flags}${_compile_options}\n")

        # Gather global compiler options, definitions, etc.
        string(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" CXX_FLAGS)
        set(COMPILER_FLAGS "${${CXX_FLAGS}} ${CMAKE_CXX_FLAGS}")
        separate_arguments(COMPILER_FLAGS)

        # Add a custom target for building the precompiled header.
        # HACK: Add explicit -std=${CXX_STD} to work around an ugly issue for CMake 3.2+
        # which prevents us from actually scraping the -std=??? flag set by target_compile_features
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
     set(CXX_STD c++11)
        else()
            set(CXX_STD gnu++11)
        endif()
        add_custom_command(
            OUTPUT ${OUTPUT_NAME}
            COMMAND ${CMAKE_CXX_COMPILER} @${PCH_FLAGS_FILE} ${COMPILER_FLAGS} -x c++-header -std=${CXX_STD} -o ${OUTPUT_NAME} ${PCH_TARGET}
            DEPENDS ${PCH_TARGET})
        add_custom_target(${TARGET_NAME}_gch DEPENDS ${OUTPUT_NAME})
        add_dependencies(${TARGET_NAME} ${TARGET_NAME}_gch)

        # set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "-include ${PCH_TARGET} -Winvalid-pch")
        get_target_property(SOURCE_FILES ${TARGET_NAME} SOURCES)
        get_target_property(asdf ${TARGET_NAME} COMPILE_FLAGS)
        foreach(SOURCE_FILE ${SOURCE_FILES})
            if(SOURCE_FILE MATCHES \\.\(c|cc|cxx|cpp\)$)
                set_source_files_properties(${SOURCE_FILE} PROPERTIES
                   COMPILE_FLAGS "-include ${OUTPUT_DIR}/${PCH_TARGET} -Winvalid-pch"
                )
            endif()
        endforeach()
    else()
        message(FATAL_ERROR "\tUnknown generator for add_precompiled_header.")
    endif()
endmacro(add_precompiled_header)
