cmake_minimum_required(VERSION 3.16)
project(leetcode_hot100_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# Debug 下也开 -O1，跑 100 道题的测试会快很多；需要调试再改成 -O0
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wno-unused-parameter -O1)
endif()

file(GLOB_RECURSE SOLUTION_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

add_executable(hot100 ${SOLUTION_SOURCES})
target_include_directories(hot100 PRIVATE "${CMAKE_SOURCE_DIR}/include")

enable_testing()
add_test(NAME hot100_all COMMAND hot100)
