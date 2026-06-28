#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __ 
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
# 
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

function(emb_library)

    _emb_current_module(module)

    set(oneValueArgs NAME TYPE)
    set(multiValueArgs SOURCES)
    cmake_parse_arguments(LIB "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT LIB_TYPE)
        set(LIB_TYPE STATIC)
    endif()

    add_library(${LIB_NAME} ${LIB_TYPE} ${LIB_SOURCES})

    set_target_properties(${LIB_NAME}
        PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )

    target_include_directories(${LIB_NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    add_library(emb::${LIB_NAME} ALIAS ${LIB_NAME})

    if(EMBEDDED_ENABLE_INSTALL)
        install(
            TARGETS ${LIB_NAME}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT ${module}
        )
    endif()

endfunction()
