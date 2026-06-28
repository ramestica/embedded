#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __ 
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
# 
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

function(_emb_current_module out)
    get_property(m DIRECTORY PROPERTY EMB_MODULE_NAME)

    if(NOT m)
        message(FATAL_ERROR "emb_module() must be called first")
    endif()

    set(${out} "${m}" PARENT_SCOPE)
endfunction()

function(_emb_default_install_dir out TYPE)

    if(TYPE STREQUAL "BIN")
        set(dir ${CMAKE_INSTALL_BINDIR})
    elseif(TYPE STREQUAL "LIB")
        set(dir ${CMAKE_INSTALL_LIBDIR})
    elseif(TYPE STREQUAL "DATA")
        set(dir ${CMAKE_INSTALL_DATADIR})
    else()
        set(dir ${CMAKE_INSTALL_DATADIR})
    endif()

    set(${out} "${dir}" PARENT_SCOPE)

endfunction()

function(_emb_default_component out)

    _emb_current_module(m)
    set(${out} "${m}" PARENT_SCOPE)

endfunction()

function(_emb_default_output_name out input)

    get_filename_component(name ${input} NAME_WE)
    set(${out} "${name}" PARENT_SCOPE)

endfunction()

function(_emb_script_interpreter out_var filename)

    get_filename_component(ext "${filename}" EXT)

    if(ext STREQUAL ".sh")
        set(interpreter "bash")
        set(check_cmd bash -n)

    elseif(ext STREQUAL ".py")
        set(interpreter "python3")
        set(check_cmd python3 -m py_compile)

    elseif(ext STREQUAL ".pl")
        set(interpreter "perl")
        set(check_cmd perl -c)

    elseif(ext STREQUAL ".rb")
        set(interpreter "ruby")
        set(check_cmd ruby -c)

    else()
        set(interpreter "sh")
        set(check_cmd sh -n)
    endif()

    set(${out_var} "${interpreter}" PARENT_SCOPE)
    set(${out_var}_CHECK "${check_cmd}" PARENT_SCOPE)

endfunction()

function(_emb_script_validate filename)

    get_filename_component(ext "${filename}" EXT)

    if(ext STREQUAL ".sh")

        execute_process(
            COMMAND bash -n "${filename}"
            RESULT_VARIABLE res
        )

    elseif(ext STREQUAL ".py")

        execute_process(
            COMMAND python3 -m py_compile "${filename}"
            RESULT_VARIABLE res
        )

    elseif(ext STREQUAL ".pl")

        execute_process(
            COMMAND perl -c "${filename}"
            RESULT_VARIABLE res
        )

    elseif(ext STREQUAL ".rb")

        execute_process(
            COMMAND ruby -c "${filename}"
            RESULT_VARIABLE res
        )

    else()

        execute_process(
            COMMAND sh -n "${filename}"
            RESULT_VARIABLE res
        )

    endif()

    if(NOT res EQUAL 0)
        message(FATAL_ERROR "Script validation failed: ${filename}")
    endif()

endfunction()
