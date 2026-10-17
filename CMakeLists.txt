cmake_minimum_required(VERSION 3.19)

file(READ "${CMAKE_SOURCE_DIR}/VERSION" VER_RAW)
string(STRIP ${VER_RAW} ABYSSCURSOR_VERSION)

add_compile_definitions(ABYSSCURSOR_VERSION="${ABYSSCURSOR_VERSION}")

project(
  abysscursor
  VERSION ${ABYSSCURSOR_VERSION}
  DESCRIPTION "A library and toolkit for the Abyssland cursor format")

include(CTest)
include(GNUInstallDirs)

set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(INCLUDE ${CMAKE_INSTALL_FULL_INCLUDEDIR})
set(LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})

configure_file(abysscursor.pc.in abysscursor.pc @ONLY)

set(CMAKE_CXX_STANDARD 23)
add_compile_options(
  -Wall
  -Wextra
  -Wpedantic
  -Wno-unused-parameter
  -Wno-unused-value
  -Wno-missing-field-initializers
  -Wno-narrowing
  -Wno-pointer-arith)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

find_package(PkgConfig REQUIRED)
pkg_check_modules(
  deps
  REQUIRED
  IMPORTED_TARGET
  abyssparse>=0.4.2
  libzip
  cairo
  librsvg-2.0
  tomlplusplus)

if(CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES DEBUG)
  message(STATUS "Configuring abysscursor in Debug")
  add_compile_definitions(ABYSSLAND_DEBUG)
else()
  add_compile_options(-O3)
  message(STATUS "Configuring abysscursor in Release")
endif()

file(
  GLOB_RECURSE
  SRCFILES
  CONFIGURE_DEPENDS
  "libabysscursor/*.cpp"
  "include/abysscursor/abysscursor.hpp"
  "include/abysscursor/abysscursor.h"
  "include/abysscursor/shared.h")

add_library(abysscursor SHARED ${SRCFILES})
target_include_directories(
  abysscursor
  PUBLIC "./include"
  PRIVATE "./libabysscursor")
set_target_properties(
  abysscursor
  PROPERTIES VERSION ${abysscursor_VERSION}
             SOVERSION 0
             PUBLIC_HEADER include/abysscursor/abysscursor.hpp
             include/abysscursor/abysscursor.h include/abysscursor/shared.h)

target_link_libraries(abysscursor PkgConfig::deps)

# abysscursor-util
file(
  GLOB_RECURSE
  UTILSRCFILES
  CONFIGURE_DEPENDS
  "abysscursor-util/src/*.cpp"
  "include/abysscursor/abysscursor.hpp"
  "include/abysscursor/abysscursor.h"
  "include/abysscursor/shared.h")
add_executable(abysscursor-util ${UTILSRCFILES})
target_include_directories(
  abysscursor-util
  PUBLIC "./include"
  PRIVATE "./libabysscursor" "./abysscursor-util/src")
target_link_libraries(abysscursor-util PkgConfig::deps abysscursor)

# tests
add_custom_target(tests)

add_executable(abysscursor_test1 "tests/full_rendering.cpp")
target_link_libraries(abysscursor_test1 PRIVATE abysscursor)
add_test(
  NAME "Test libabysscursor in C++ (full rendering)"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests
  COMMAND abysscursor_test1)
add_dependencies(tests abysscursor_test1)

add_executable(abysscursor_test2 "tests/only_metadata.cpp")
target_link_libraries(abysscursor_test2 PRIVATE abysscursor)
add_test(
  NAME "Test libabysscursor in C++ (only metadata)"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests
  COMMAND abysscursor_test2)
add_dependencies(tests abysscursor_test2)

add_executable(abysscursor_test_c "tests/c_test.c")
target_link_libraries(abysscursor_test_c PRIVATE abysscursor)
add_test(
  NAME "Test libabysscursor in C"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tests
  COMMAND abysscursor_test_c)
add_dependencies(tests abysscursor_test_c)

# Keep tests independent of installed cursor themes and desktop environment.
add_executable(abysscursor_test_fixture "tests/fixture.cpp")
target_link_libraries(abysscursor_test_fixture PRIVATE PkgConfig::deps)
add_test(NAME "Prepare cursor fixtures" COMMAND abysscursor_test_fixture "${CMAKE_BINARY_DIR}/test-home")
set_tests_properties("Prepare cursor fixtures" PROPERTIES FIXTURES_SETUP cursor_themes)
add_executable(abysscursor_test_regressions "tests/regressions.cpp")
target_link_libraries(abysscursor_test_regressions PRIVATE abysscursor)
add_test(NAME "Cursor parser and lifecycle regressions" COMMAND abysscursor_test_regressions)
add_dependencies(tests abysscursor_test_fixture abysscursor_test_regressions)
add_test(NAME "CLI rejects missing output" COMMAND abysscursor-util --create unused --output)
add_test(NAME "CLI rejects missing resize algorithm" COMMAND abysscursor-util --create unused --resize)
add_test(
  NAME "CLI output deletion guard"
  COMMAND ${CMAKE_COMMAND} "-DUTIL=$<TARGET_FILE:abysscursor-util>"
          "-DTEST_ROOT=${CMAKE_BINARY_DIR}" -P "${CMAKE_SOURCE_DIR}/tests/deletion_guard.cmake")
set_tests_properties("CLI rejects missing output" "CLI rejects missing resize algorithm" PROPERTIES WILL_FAIL TRUE)
set_tests_properties(
  "Test libabysscursor in C++ (full rendering)"
  "Test libabysscursor in C++ (only metadata)"
  "Test libabysscursor in C"
  "Cursor parser and lifecycle regressions"
  PROPERTIES FIXTURES_REQUIRED cursor_themes
             WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
             ENVIRONMENT "HOME=${CMAKE_BINARY_DIR}/test-home;XDG_DATA_HOME=${CMAKE_BINARY_DIR}/test-home/.local/share;XDG_DATA_DIRS=${CMAKE_BINARY_DIR}/test-home/system;ABYSSCURSOR_THEME=review-fixture")

# Installation
install(TARGETS abysscursor)
install(TARGETS abysscursor-util)
install(DIRECTORY "include/abysscursor" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_BINARY_DIR}/abysscursor.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

if(INSTALL_TESTS)
  install(TARGETS abysscursor_test1)
  install(TARGETS abysscursor_test2)
  install(TARGETS abysscursor_test_c)
endif()
