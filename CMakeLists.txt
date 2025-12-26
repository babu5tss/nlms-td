cmake_minimum_required(VERSION 3.0.0)
project(nlms-td VERSION 1.0.0 LANGUAGES C)

# Set default build type if not provided by user
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/nlms_td.c
)

# Build a static library
add_library(lib_nlms_td STATIC ${SOURCES})
# Public include directory for the library
target_include_directories(lib_nlms_td PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)

# Per-configuration compile options and definitions
target_compile_options(lib_nlms_td PRIVATE
    $<$<CONFIG:Debug>:-g;-O0;-Wall;-Wextra;-Wpedantic;-DDEBUG>
    $<$<CONFIG:Release>:-O3;-DNDEBUG;-Wall;-Wextra;-Wpedantic>
)

# create test executable
add_executable(nlms-td  ${CMAKE_CURRENT_SOURCE_DIR}/test/test_nlms_td.c)
target_link_libraries(nlms-td  PRIVATE lib_nlms_td)
target_include_directories(nlms-td PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
