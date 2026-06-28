#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __ 
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
# 
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

function(emb_uv)

    _emb_current_module(module)

    set(oneValueArgs DIRECTORY)
    cmake_parse_arguments(UV "" "${oneValueArgs}" "" ${ARGN})

    if(NOT UV_DIRECTORY)
        set(UV_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    add_custom_target(
        ${module}_python
        COMMAND uv sync
        WORKING_DIRECTORY ${UV_DIRECTORY}
    )

endfunction()
