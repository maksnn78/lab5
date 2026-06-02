cmake_minimum_required(VERSION 3.4)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(lab05)

include_directories(banking)
add_subdirectory(banking banking)

option(BUILD_TESTS "Build tests" OFF)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(third-party/gtest)
  add_executable(check tests/bank_test.cpp)
  target_compile_options(check PRIVATE --coverage)
  target_link_libraries(check banking gtest_main gmock_main --coverage)
  add_test(NAME check COMMAND check)
endif()
