cmake_minimum_required(VERSION 3.10)
project(YourProjectName)

# Set C++ standard (recommended)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add your source files
file(GLOB SOURCES "src/*.cpp" "src/*.c")
file(GLOB HEADERS "include/*.h" "include/*.hpp")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Platform-specific configurations
if(WIN32)
    # Windows-specific settings
    add_definitions(-DWINDOWS_PLATFORM)
    # Example: link against Windows-specific libraries
    # target_link_libraries(${PROJECT_NAME} some_windows_lib)
elseif(UNIX AND NOT APPLE)
    # Linux-specific settings
    add_definitions(-DLINUX_PLATFORM)
    # Example: link against pthread
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} Threads::Threads)
endif()

# Common dependencies (for both platforms)
# find_package(SomeCommonDependency REQUIRED)
# target_link_libraries(${PROJECT_NAME} SomeCommonDependency)