#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __ 
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
# 
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

function(emb_executable)

    _emb_current_module(module)

    set(oneValueArgs NAME)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(EXE "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${EXE_NAME} ${EXE_SOURCES})

    set_target_properties(${EXE_NAME}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    if(EMBEDDED_ENABLE_INSTALL)
        install(
            TARGETS ${EXE_NAME}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT ${module}
        )
    endif()

endfunction()
