#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __ 
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
# 
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

function(emb_config)

    _emb_current_module(module)

    set(oneValueArgs INPUT OUTPUT DESTINATION)
    cmake_parse_arguments(CFG "" "${oneValueArgs}" "" ${ARGN})

    if(NOT CFG_INPUT)
        message(FATAL_ERROR "emb_config(INPUT ...) required")
    endif()

    if(NOT CFG_OUTPUT)
        get_filename_component(filename "${CFG_INPUT}" NAME)

        if(filename MATCHES "\\.in$")
            string(REGEX REPLACE "\\.in$" "" filename "${filename}")
        endif()

        set(CFG_OUTPUT "${filename}")
    endif()

    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/config")

    set(out "${CMAKE_BINARY_DIR}/config/${CFG_OUTPUT}")

    configure_file(
        "${CFG_INPUT}"
        "${out}"
        @ONLY
    )

    if(EMBEDDED_ENABLE_INSTALL)
        if(NOT CFG_DESTINATION)
            set(CFG_DESTINATION ${CMAKE_INSTALL_DATADIR})
        endif()

        install(
            FILES "${out}"
            DESTINATION "${CFG_DESTINATION}"
            COMPONENT "${module}"
        )
    endif()

endfunction()
