cmake_minimum_required(VERSION 3.5.0)
project(strm-test)

set(strm_include_folder "${PROJECT_SOURCE_DIR}/include")
set(strm_test_source_folder "${PROJECT_SOURCE_DIR}/test")

# strm install includes step.

file(GLOB_RECURSE strm_includes
    "${strm_include_folder}/*.h")

install(DIRECTORY "${strm_include_folder}/strm" DESTINATION include)

# compiler configuration.

if (UNIX)

  find_package(Threads)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra")

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -D_DEBUG -O0")
  elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -O3")
  endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

endif (UNIX)

# download and build googletest.

include(ExternalProject)
set(googletest_install_folder "${CMAKE_BINARY_DIR}/googletest-install")
set(googletest_include_folder "${googletest_install_folder}/include")
set(googletest_static_library "${googletest_install_folder}/lib/libgtest.a")
ExternalProject_Add(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.8.0
    SOURCE_DIR "${CMAKE_BINARY_DIR}/googletest-src"
    BINARY_DIR "${CMAKE_BINARY_DIR}/googletest-build"
    INSTALL_DIR ${googletest_install_folder}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${googletest_install_folder}
    BUILD_COMMAND ${CMAKE_COMMAND} --build . --target install
    BUILD_BYPRODUCTS ${googletest_static_library})

# strm unit tests target.

file(GLOB_RECURSE strm_test_sources
    "${strm_test_source_folder}/*.h"
    "${strm_test_source_folder}/*.cpp")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(STRM_TARGET strm_test_debug)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
  set(STRM_TARGET strm_test_release)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

add_executable(${STRM_TARGET} ${strm_test_sources})
target_include_directories(${STRM_TARGET} PUBLIC ${strm_include_folder})
target_include_directories(${STRM_TARGET} PUBLIC ${googletest_include_folder})
target_link_libraries(${STRM_TARGET} ${googletest_static_library})
add_dependencies(${STRM_TARGET} googletest)
install(TARGETS ${STRM_TARGET} DESTINATION test)
