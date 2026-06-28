#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __ 
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
# 
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

function(emb_container)

    _emb_current_module(module)

    set(oneValueArgs NAME COMMAND WORKDIR)
    cmake_parse_arguments(CON "" "${oneValueArgs}" "" ${ARGN})

    if(NOT CON_WORKDIR)
        set(CON_WORKDIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    add_custom_target(
        ${module}_${CON_NAME}
        ALL
        COMMAND ${CON_COMMAND}
        WORKING_DIRECTORY "${CON_WORKDIR}"
        COMMENT "Building container ${CON_NAME}"
    )

endfunction()
