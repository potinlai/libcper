cmake_minimum_required(VERSION 3.10)
include(FetchContent)
project(CPERParse)

# Output into subdirectories /lib and /bin.
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Fetch json-c from git repository.
FetchContent_Declare(
  json-c
  GIT_REPOSITORY https://github.com/json-c/json-c.git
  GIT_TAG        d28ac67dde77566f53a97f22b4ea7cb36afe6582 # 4/6/2022
)
FetchContent_MakeAvailable(json-c)

# Add library and test executable.
file(GLOB SectionSources sections/*.c)
file(GLOB EDKSources edk/*.c)
add_library(CPERParseLibrary STATIC cper-parse.c cper-utils.c ${SectionSources} ${EDKSources})
add_executable(CPERParseTest cper-test.c)

# Link library.
target_link_libraries(CPERParseLibrary json-c)
target_link_libraries(CPERParseTest CPERParseLibrary)