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

# Fetch b64-c from git repository.
FetchContent_Declare(
  b64-c
  GIT_REPOSITORY https://github.com/jwerle/b64.c.git
  GIT_TAG        c33188cd541f19b072ee4988d8224ea6c964bed1 # 9/2/2021
)
FetchContent_MakeAvailable(json-c b64-c)

# Add library and test executable.
file(GLOB SectionSources sections/*.c)
file(GLOB EDKSources edk/*.c)
add_library(CPERParseLibrary STATIC cper-parse.c ir-parse.c cper-utils.c json-schema.c json-schema.h ${SectionSources} ${EDKSources})
add_executable(CPERParseTest testing/cper-test.c)

# Link library.
target_link_libraries(CPERParseLibrary json-c b64c)
target_link_libraries(CPERParseTest CPERParseLibrary)
target_compile_options(CPERParseLibrary PRIVATE -Wno-address-of-packed-member)