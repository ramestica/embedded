#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __ 
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
# 
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

function(emb_script)
    set(oneValueArgs FILE RENAME DESTINATION)
    cmake_parse_arguments(S "" "${oneValueArgs}" "" ${ARGN})
    if(NOT S_FILE)
        message(FATAL_ERROR "emb_script(FILE ...) required")
    endif()
    #
    # ------------------------------------------------------------
    # Resolve source file (never executed)
    # ------------------------------------------------------------
    #
    get_filename_component(src_file
        "${S_FILE}"
        ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    )
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    #
    # ------------------------------------------------------------
    # Output name
    # ------------------------------------------------------------
    #
    if(S_RENAME)
        set(name "${S_RENAME}")
    else()
        # Strip a single trailing ".in" first (if present), then strip a
        # single trailing ".sh" — so "foo.sh.in" -> "foo", "foo.sh" -> "foo",
        # and "foo.in" -> "foo".
        get_filename_component(base_name "${src_file}" NAME)
        string(REGEX REPLACE "\\.in$" "" base_name "${base_name}")
        string(REGEX REPLACE "\\.sh$" "" base_name "${base_name}")
        set(name "${base_name}")
    endif()
    set(target "${CMAKE_BINARY_DIR}/bin/${name}")
    #
    # ------------------------------------------------------------
    # Phase 1: stage into build (temp file, removed at end of function)
    # ------------------------------------------------------------
    #
    set(staged "${CMAKE_BINARY_DIR}/bin/_staged_${name}")
    if(src_file MATCHES "\\.in$")
        configure_file(
            "${src_file}"
            "${staged}"
            @ONLY
        )
    else()
        configure_file(
            "${src_file}"
            "${staged}"
            COPYONLY
        )
    endif()
    #
    # ------------------------------------------------------------
    # Phase 2: FORCE BASH TOOLCHAIN (consistent model)
    # ------------------------------------------------------------
    #
    # IMPORTANT: your scripts already use bash features
    #
    set(interpreter "/usr/bin/env bash")
    set(validate_cmd "bash -n")
    #
    # ------------------------------------------------------------
    # Phase 3: build final executable
    # ------------------------------------------------------------
    #
    file(READ "${staged}" content)
    if(content MATCHES "^#!")
        set(final_content "${content}")
    else()
        set(final_content "#!/usr/bin/env bash\n${content}")
    endif()
    file(WRITE "${target}" "${final_content}")
    file(REMOVE "${staged}")
    file(CHMOD
        "${target}"
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
    )
    #
    # ------------------------------------------------------------
    # Phase 4: VALIDATE FINAL ARTIFACT ONLY
    # ------------------------------------------------------------
    #
    execute_process(
        COMMAND bash -n "${target}"
        RESULT_VARIABLE res
    )
    if(NOT res EQUAL 0)
        message(FATAL_ERROR
            "Script validation failed: ${target}"
        )
    endif()
    #
    # ------------------------------------------------------------
    # Phase 5: install (optional)
    # ------------------------------------------------------------
    #
    if(EMB_ENABLE_INSTALL)
        if(NOT S_DESTINATION)
            set(S_DESTINATION ${CMAKE_INSTALL_BINDIR})
        endif()
        install(
            PROGRAMS "${target}"
            DESTINATION "${S_DESTINATION}"
            COMPONENT "${PROJECT_NAME}"
        )
    endif()
endfunction()
