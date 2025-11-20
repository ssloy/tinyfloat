cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

Include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.3.1)

FetchContent_MakeAvailable(Catch2)

FILE(GLOB SRCTEST arithmetic.cpp comparisons.cpp printer.cpp roundtrip-float.cpp roundtrip-int.cpp)
add_executable(tinyfloat-test-all ${SRCTEST})
target_link_libraries(tinyfloat-test-all PRIVATE ${CMAKE_DL_LIBS} tinyfloat Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(tinyfloat-test-all)
