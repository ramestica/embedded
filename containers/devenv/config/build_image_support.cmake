#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __ 
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
# 
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

# process_scripts_and_config_files(<scripts> <configs>)
#
# Registers build rules to copy shell scripts and config files into the build
# tree, creating two ALL targets (copy_script_files, copy_config_files) that
# are driven by per-file add_custom_command(OUTPUT) rules and thus benefit
# from standard dependency tracking (files are only re-copied when sources
# change).
#
# Each argument is a CMake list of file specifiers with the form:
#
#   "src"        - source and destination filenames are the same
#   "src:dst"    - source and destination filenames differ
#
# Scripts  (scripts):
#   Source root : CMAKE_CURRENT_SOURCE_DIR
#   Dest root   : CMAKE_BINARY_DIR/bin/
#   Extra steps : shebang injection (if missing) and chmod +x, applied via
#                 a generated helper fix_shebang.sh written at configure time.
#
# Config files (configs):
#   Source root : CMAKE_CURRENT_SOURCE_DIR/config/
#   Dest root   : CMAKE_BINARY_DIR/config/
#   Extra steps : none (plain copy only)
#
# Both destination directories are created at configure time with
# file(MAKE_DIRECTORY ...).
#
# Example usage (from a subdirectory's CMakeLists.txt):
#
#   set(SCRIPT_FILE_NAMES
#       run_foo.sh
#       bar.sh:bar   # copied as bar
#   )
#   set(CONFIG_FILE_NAMES
#       settings.cmake
#   )
#   process_scripts_and_config_files(
#       "${SCRIPT_FILE_NAMES}"
#       "${CONFIG_FILE_NAMES}"
#   )
#
# Notes:
#   - Must be called from the CMakeLists.txt of the directory that owns the
#     source files; CMAKE_CURRENT_SOURCE_DIR is captured at call time.
#   - copy_script_files and copy_config_files are only created when their
#     respective lists are non-empty, to avoid registering empty targets.
#   - The function may only be called once per directory scope because the
#     target names copy_script_files and copy_config_files are global and
#     will conflict if registered a second time.
function(process_scripts_and_config_files scripts configs)
    set(SCRIPT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
    set(SCRIPT_DST_DIR "${CMAKE_BINARY_DIR}/bin")
    set(CONFIG_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/config")
    set(CONFIG_DST_DIR "${CMAKE_BINARY_DIR}/config")

    file(MAKE_DIRECTORY "${SCRIPT_DST_DIR}")
    file(MAKE_DIRECTORY "${CONFIG_DST_DIR}")

    # Write a small helper script at configure time
    set(FIX_SHEBANG_SCRIPT "${CMAKE_BINARY_DIR}/fix_shebang.sh")
    file(WRITE "${FIX_SHEBANG_SCRIPT}" "\
#!/usr/bin/env bash
set -e
DST=\"$1\"
if ! head -n1 \"$DST\" | grep -q '^#!'; then
    sed -i '1i#!/usr/bin/env bash' \"$DST\"
fi
chmod +x \"$DST\"
")
    execute_process(COMMAND chmod +x "${FIX_SHEBANG_SCRIPT}")

    # Process scripts: copy then fix shebang + chmod
    set(SCRIPT_OUTPUTS "")
    foreach(arg IN LISTS scripts)
        # Split the item on ":" to get source and destination
        string(REPLACE ":" ";" pair "${arg}")
        list(LENGTH pair pair_len)
        if(pair_len EQUAL 2)
            list(GET pair 0 SRC)
            list(GET pair 1 DST)
        else()
            list(GET pair 0 SRC)
            set(DST "${SRC}")
        endif()
        set(FILENAME "${SRC}")
        set(SRC "${SCRIPT_SRC_DIR}/${SRC}")
        set(DST "${SCRIPT_DST_DIR}/${DST}")
        add_custom_command(
            OUTPUT "${DST}"
            # remove before the copy, which ensures the destination is
            # always freshly written and then fixed regardless of what
            # was there before.
            COMMAND ${CMAKE_COMMAND} -E remove -f "${DST}"
            COMMAND ${CMAKE_COMMAND} -E copy "${SRC}" "${DST}"
            COMMAND bash "${FIX_SHEBANG_SCRIPT}" "${DST}"
            DEPENDS "${SRC}" "${FIX_SHEBANG_SCRIPT}"
            COMMENT "Installing script ${FILENAME}"
        )
        list(APPEND SCRIPT_OUTPUTS "${DST}")
    endforeach()

    if(SCRIPT_OUTPUTS)
        add_custom_target(copy_script_files ALL
            DEPENDS ${SCRIPT_OUTPUTS}
        )
    endif()

    # Process configs: copy only
    set(CONFIG_OUTPUTS "")
    foreach(arg IN LISTS configs)
        # Split the item on ":" to get source and destination
        string(REPLACE ":" ";" pair "${arg}")
        list(LENGTH pair pair_len)
        if(pair_len EQUAL 2)
            list(GET pair 0 SRC)
            list(GET pair 1 DST)
        else()
            list(GET pair 0 SRC)
            set(DST "${SRC}")
        endif()
        set(FILENAME "${SRC}")
        set(SRC "${CONFIG_SRC_DIR}/${SRC}")
        set(DST "${CONFIG_DST_DIR}/${DST}")
        add_custom_command(
            OUTPUT "${DST}"
            COMMAND ${CMAKE_COMMAND} -E copy "${SRC}" "${DST}"
            DEPENDS "${SRC}"
            COMMENT "Installing config ${FILENAME}"
        )
        list(APPEND CONFIG_OUTPUTS "${DST}")
    endforeach()

    if(CONFIG_OUTPUTS)
        add_custom_target(copy_config_files ALL
            DEPENDS ${CONFIG_OUTPUTS}
        )
    endif()

    list(LENGTH scripts NUM_SCRIPTS)
    list(LENGTH configs NUM_CONFIGS)
    message(STATUS "Processed ${NUM_SCRIPTS} script(s) and ${NUM_CONFIGS} config file(s)")
endfunction()

#___oOo___
