cmake_minimum_required(VERSION 3.10)
include(FetchContent)
include(GoogleTest)
include(FindSWIG)
include(UseSWIG)
find_package(PythonLibs 3 REQUIRED)
find_package(PythonInterp ${PYTHONLIBS_VERSION_STRING} REQUIRED)
project(CPERParse)

# GoogleTest requires at least C++14.
set(CMAKE_CXX_STANDARD 14)

# Output into subdirectories /lib and /bin.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Fetch json-c from git repository.
FetchContent_Declare(
  json-c
  GIT_REPOSITORY https://github.com/json-c/json-c.git
  GIT_TAG        9021cdcdd01fc9dbcbe1f06391848c2ac915212f # 0.15
)

# Fetch b64-c from git repository.
FetchContent_Declare(
  b64-c
  GIT_REPOSITORY https://github.com/jwerle/b64.c.git
  GIT_TAG        c33188cd541f19b072ee4988d8224ea6c964bed1 # 9/2/2021
)

# Fetch GoogleTest from git repository.
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(json-c b64-c googletest)

# Add library and test executable.
file(GLOB SectionSources sections/*.c)
file(GLOB EDKSources edk/*.c)
file(GLOB GeneratorSectionSources generator/sections/*.c)
add_library(cper-parse STATIC 
  cper-parse.c 
  ir-parse.c 
  cper-utils.c 
  common-utils.c
  json-schema.c 
  ${SectionSources} 
  ${EDKSources}
)
add_executable(cper-convert cli-app/cper-convert.c)
add_executable(cper-generate 
  generator/cper-generate-cli.c
  generator/cper-generate.c 
  generator/gen-utils.c 
  common-utils.c
  ${GeneratorSectionSources} 
  ${EDKSources}
)
add_executable(cper-tests 
  tests/ir-tests.cpp 
  tests/test-utils.cpp
  generator/cper-generate.c
  generator/gen-utils.c 
  ${GeneratorSectionSources} 
  ${EDKSources}
)

# Link library.
target_link_libraries(cper-tests cper-parse json-c GTest::gtest_main)
target_link_libraries(cper-parse json-c b64c)
target_link_libraries(cper-convert cper-parse)
target_compile_options(cper-parse PRIVATE -Wno-address-of-packed-member)

# Copy required specification JSON for command line application.
add_custom_command(TARGET cper-convert POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory bin/specification)
add_custom_command(TARGET cper-convert POST_BUILD COMMAND cp -r specification/json/* bin/specification)

# Add tests to GoogleTest.
gtest_discover_tests(cper-tests WORKING_DIRECTORY bin/)

# Generate Python bindings with SWIG.
include_directories(cperparse_pylib ${PYTHON_INCLUDE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
swig_add_library(cperparse_pylib 
  TYPE STATIC
  LANGUAGE python
  SOURCES cper-parse.i
  OUTPUT_DIR lib/
)
swig_link_libraries(cperparse_pylib 
  cper-parse
  json-c
  ${PYTHON_LIBRARIES}
)