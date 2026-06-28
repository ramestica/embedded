#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __ 
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
# 
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

function(emb_module)

    set(oneValueArgs NAME TITLE)
    cmake_parse_arguments(MOD "" "${oneValueArgs}" "" ${ARGN})

    if(NOT MOD_NAME)
        message(FATAL_ERROR "emb_module(NAME ...) required")
    endif()

    set_property(DIRECTORY PROPERTY EMB_MODULE_NAME "${MOD_NAME}")
    set_property(DIRECTORY PROPERTY EMB_MODULE_TITLE "${MOD_TITLE}")

    set_property(GLOBAL APPEND PROPERTY EMB_MODULES "${MOD_NAME}")

endfunction()
