cmake_minimum_required(VERSION 3.15)
project(gyatt VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CURL_INCLUDE_DIRS})

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Create the executable
add_executable(gyatt ${SOURCES})

# Link libraries
target_link_libraries(gyatt OpenSSL::SSL OpenSSL::Crypto ${CURL_LIBRARIES})

# Compiler flags
target_compile_options(gyatt PRIVATE -Wall -Wextra -pedantic)

# Install target
install(TARGETS gyatt DESTINATION bin)
