cmake_minimum_required(VERSION 3.10)

# set the project name
project(SCREAM)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add the executable
add_executable(SCREAM server.cpp)

target_link_libraries(SCREAM PUBLIC Server)

target_include_directories(SCREAM PUBLIC
                      "${PROJECT_BINARY_DIR}"
                      "${PROJECT_SOURCE_DIR}/Server"
                          )
