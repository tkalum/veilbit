cmake_minimum_required(VERSION 3.10)
project(veilbit)

# List all source files
set(SOURCES 
    src/main.c
    src/bmp.c
    src/veilbit.c 
    src/jpeg.c
)

# Create executable
add_executable(veilbit ${SOURCES})

# Tell compiler where to find headers
target_include_directories(veilbit PRIVATE include)