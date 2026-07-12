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
    set(options ALL)
    set(oneValueArgs NAME WORKDIR COMMENT)
    set(multiValueArgs COMMAND DEPENDS)
    cmake_parse_arguments(CON "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT CON_NAME)
        message(FATAL_ERROR "emb_container(NAME ...) required")
    endif()
    if(NOT CON_COMMAND)
        message(FATAL_ERROR "emb_container(COMMAND ...) required")
    endif()
    if(NOT CON_WORKDIR)
        set(CON_WORKDIR "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
    if(NOT CON_COMMENT)
        set(CON_COMMENT "Building container ${CON_NAME}")
    endif()

    set(target_name ${module}_${CON_NAME})
    set(all_kw "")
    if(CON_ALL)
        set(all_kw ALL)
    endif()

    add_custom_target(
        ${target_name}
        ${all_kw}
        COMMAND ${CON_COMMAND}
        WORKING_DIRECTORY "${CON_WORKDIR}"
        DEPENDS ${CON_DEPENDS}
        COMMENT "${CON_COMMENT}"
        USES_TERMINAL
        VERBATIM
    )
endfunction()
