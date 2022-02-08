cmake_minimum_required(VERSION 3.12)

project(flex_bison_embedded_example VERSION 0.1.0
                  DESCRIPTION "Project to adapt flex/bison parser system to embedded applications"
                  LANGUAGES C)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)



set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_executable(Example 
    lex.yy.c 
    calc.tab.c 
    calc.tab.h 
    memory_management.c 
    memory_management.h)

target_include_directories(Example
	PUBLIC ${CMAKE_CURRENT_LIST_DIR}
)