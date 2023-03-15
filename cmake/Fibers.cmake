# Project Name Project
# -------------------------
file(GLOB_RECURSE Fibers_LIBS_INC    ${SRC_DIR}/fibers/include/*.h*)
file(GLOB_RECURSE Fibers_LIBS_SRC    ${SRC_DIR}/fibers/src/*.cpp)


# Create the project filters
GROUPSOURCES(${SRC_DIR}/fibers/include include)
GROUPSOURCES(${SRC_DIR}/fibers/src src)


# Create the project
add_executable(Fibers ${Fibers_LIBS_INC} ${Fibers_LIBS_SRC})

# Compiler definitions
# add_compile_definitions("")

# Set the include directories
target_include_directories(Fibers PUBLIC ${SRC_DIR}/fibers/include)

# Set the static libraries you need to link to 
IF(MSVC)
	target_link_libraries(Fibers "Dbghelp.lib")
ENDIF()

# Set project properties
set_target_properties(Fibers PROPERTIES DEFINE_SYMBOL                                  "" )                     		# defines
IF(MSVC)
    set_target_properties(Fibers PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY                ${CMAKE_SOURCE_DIR}/data)        		# working directory
ENDIF()
