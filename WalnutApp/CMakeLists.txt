cmake_minimum_required(VERSION 3.16)

project(atisquitaine)

set(HTTPLIB_COMPILE ON CACHE BOOL "" FORCE)
add_subdirectory(cpp-httplib)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/airac.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/airac.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/metar.json ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/metar.json)

file(GLOB_RECURSE ATISQUITAINE_SOURCES CONFIGURE_DEPENDS 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)
add_executable(atisquitaine ${ATISQUITAINE_SOURCES})

target_precompile_headers(atisquitaine PRIVATE src/json.hpp)
target_link_libraries(atisquitaine walnut httplib)
