cmake_minimum_required(VERSION 3.20)
project(SUCCINCT)

set(CMAKE_CXX_STANDARD 20)

option(SUCCINCT_USE_LIBCXX "Use libc++ with Clang instead of libstdc++" OFF)
option(SUCCINCT_USE_INTRINSICS
       "Use a set of intrinsics available on all x86-64 architectures" ON)
option(SUCCINCT_USE_POPCNT
       "Use popcount intrinsic. Available on x86-64 since SSE4.2." OFF)

configure_file(${SUCCINCT_SOURCE_DIR}/succinct_config.hpp.in
               ${SUCCINCT_SOURCE_DIR}/succinct_config.hpp)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  if(SUCCINCT_USE_LIBCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  endif()
endif()

if(SUCCINCT_USE_POPCNT)
  if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
  endif()
  # XXX(ot): what to do for MSVC?
endif()

# XXX(ot): enable this on all compilers
if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-missing-braces")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wconversion")
endif()

include_directories(${PROJECT_SOURCE_DIR})

set(SUCCINCT_SOURCES rs_bit_vector.cpp bp_vector.cpp)

add_library(succinct STATIC ${SUCCINCT_SOURCES})

add_subdirectory(perftest)

# make and run tests only if library is compiled stand-alone
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  find_package(GTest REQUIRED)
  enable_testing()
  file(GLOB SUCCINCT_TEST_SOURCES test_*.cpp)
  list(FILTER SUCCINCT_TEST_SOURCES EXCLUDE REGEX "test_main\\.cpp"
  )# Remove test_main.cpp from the list
  foreach(TEST_SRC ${SUCCINCT_TEST_SOURCES})
    get_filename_component(TEST_SRC_NAME ${TEST_SRC} NAME_WE)
    add_executable(${TEST_SRC_NAME} ${TEST_SRC} test_main.cpp)
    target_link_libraries(${TEST_SRC_NAME} succinct gtest gmock)
    add_test(${TEST_SRC_NAME} ${TEST_SRC_NAME})
  endforeach(TEST_SRC)
endif()
