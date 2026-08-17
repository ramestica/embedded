#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
#
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0
#
# Interface targets for vendored SDKs (git submodules). Kept out of the
# submodules themselves — cmsis/ and tivaware/ are separate repos, not ours
# to add build files into.

add_library(cmsis-core INTERFACE)
target_include_directories(cmsis-core INTERFACE ${CMAKE_SOURCE_DIR}/cmsis/core/CMSIS/Core/Include)
add_library(emb::cmsis-core ALIAS cmsis-core)

add_library(tivaware INTERFACE)
target_include_directories(tivaware INTERFACE ${CMAKE_SOURCE_DIR}/tivaware)
add_library(emb::tivaware ALIAS tivaware)

add_library(cmsis-dsp INTERFACE)
target_include_directories(cmsis-dsp INTERFACE ${CMAKE_SOURCE_DIR}/cmsis/dsp/Include)
target_link_libraries(cmsis-dsp INTERFACE cmsis-core)
add_library(emb::cmsis-dsp ALIAS cmsis-dsp)

#___oOo___
