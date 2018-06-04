cmake_minimum_required(VERSION 3.5.0)
project(strm-test)

include(ExternalProject)

set(strm_include_folder "${PROJECT_SOURCE_DIR}/include")
set(strm_test_source_folder "${PROJECT_SOURCE_DIR}/test")

# strm install includes step.

file(GLOB_RECURSE strm_includes
    "${strm_include_folder}/*.h")

install(DIRECTORY "${strm_include_folder}/strm" DESTINATION include)

# compiler configuration.

if (UNIX)

  find_package(Threads)

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -pthread")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra")
  set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -latomic")

  set(CXX_FLAGS_DEBUG "-g -D_DEBUG -O0")
  set(CXX_FLAGS_RELEASE  "-DNDEBUG -O3")

endif (UNIX)

# download boost asio.

add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY) # To avoid compiling boost_system.

if (NOT STRM_BOOST_VERSION)
  set(STRM_BOOST_VERSION 1.67.0)
endif (NOT STRM_BOOST_VERSION)

string(REPLACE "." "_" STRM_BOOST_VERSION_UNDERSCORE ${STRM_BOOST_VERSION})

set(boost_asio_install_folder "${CMAKE_BINARY_DIR}/boost-asio")
set(boost_asio_include_folder "${boost_asio_install_folder}")
ExternalProject_Add(boost_asio
    URL https://dl.bintray.com/boostorg/release/${STRM_BOOST_VERSION}/source/boost_${STRM_BOOST_VERSION_UNDERSCORE}.tar.gz
    SOURCE_DIR ${boost_asio_install_folder}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

# download and build googletest.

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

# strm unit tests targets.

file(GLOB_RECURSE strm_test_sources
    "${strm_test_source_folder}/*.h"
    "${strm_test_source_folder}/*.cpp")

include_directories(${boost_asio_include_folder})
include_directories(${strm_include_folder})
include_directories(${googletest_include_folder})

add_executable(strm_test_debug ${strm_test_sources})
set_target_properties(strm_test_debug PROPERTIES COMPILE_FLAGS ${CXX_FLAGS_DEBUG})
target_link_libraries(strm_test_debug PRIVATE ${googletest_static_library})
add_dependencies(strm_test_debug boost_asio googletest)
install(TARGETS strm_test_debug DESTINATION test)

add_executable(strm_test_release ${strm_test_sources})
set_target_properties(strm_test_release PROPERTIES COMPILE_FLAGS ${CXX_FLAGS_RELEASE})
target_link_libraries(strm_test_release PRIVATE ${googletest_static_library})
add_dependencies(strm_test_release boost_asio googletest)
install(TARGETS strm_test_release DESTINATION test)

# coverage support.

set(COVERAGE OFF CACHE BOOL "Coverage")

if (COVERAGE)
  message(STATUS "Building with coverage support")
  target_compile_options(strm_test_debug PRIVATE --coverage)
  target_link_libraries(strm_test_debug PRIVATE --coverage)
endif (COVERAGE)
