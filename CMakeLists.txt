cmake_minimum_required(VERSION 3.12)
project(veilbit)

# Automatically find all source files (will update when new files are added)
file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.c")
file(GLOB HEADERS CONFIGURE_DEPENDS "include/*.h")

add_executable(veilbit ${SOURCES})
target_include_directories(veilbit PRIVATE include)

find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
target_link_libraries(veilbit PRIVATE ${JPEG_LIBRARIES} ${PNG_LIBRARIES})

# Set C standard if needed
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)