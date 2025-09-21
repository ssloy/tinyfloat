cmake_minimum_required(VERSION 3.12...3.26)

project(tinyfloat LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT is_multi_config AND NOT (CMAKE_BUILD_TYPE OR DEFINED ENV{CMAKE_BUILD_TYPE}))
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Release default")
endif()

if (MSVC)
    # warning level 4 (and all warnings as errors, /WX)
    add_compile_options(/W4)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${RELATIVE_LIB_DIR}/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${RELATIVE_BIN_DIR}/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
file(GLOB SOURCES tinyfloat.cpp tinyfloat.h printer.cpp printer.h)
add_library(tinyfloat ${SOURCES})

include(CTest)
enable_testing()
add_subdirectory(tests)

add_executable(paranoia paranoia.cpp)
target_link_libraries(paranoia PRIVATE ${CMAKE_DL_LIBS} tinyfloat)


file(GENERATE OUTPUT .gitignore CONTENT "*")
