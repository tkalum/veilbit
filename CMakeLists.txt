cmake_minimum_required(VERSION 3.10)
project(veilbit C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# List all source files
set(SOURCES
    src/main.c
    src/bmp.c
    src/veilbit.c
)

# Create executable
add_executable(veilbit ${SOURCES})

# Include directory
target_include_directories(veilbit PRIVATE include)