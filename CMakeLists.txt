# embedded/CMakeLists.txt
cmake_minimum_required(VERSION 3.25)

project(
    embedded
    VERSION 1.0.0
    LANGUAGES C CXX
)

set(EMB_CONFIG_DIR "${CMAKE_BINARY_DIR}/config")

# Modules use this to guard sources/libraries that only make sense on one
# side (e.g. bare-metal syscall stubs vs. a hosted libc) instead of on
# CMAKE_CROSSCOMPILING directly.
if(CMAKE_CROSSCOMPILING)
    set(EMB_HOST FALSE)
else()
    set(EMB_HOST TRUE)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(helpers)
include(module)
include(library)
include(executable)
include(script)
include(config)
include(container)
include(uv)

include(GNUInstallDirs)
include(vendor)

enable_testing()

#
# Modules
#
add_subdirectory(containers/devenv)
add_subdirectory(application)

#___oOo___
