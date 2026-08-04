#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __ 
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
# 
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

emb_module(
    NAME  devenv
    TITLE "Development environment container image"
)

file(TOUCH "${CMAKE_BINARY_DIR}/.known_hosts")

emb_config(INPUT config/conanfile.py)

emb_config(INPUT config/conan.lock)

emb_script(FILE src/devenv_entrypoint.sh)

emb_script(FILE src/devenv_container.sh)

emb_config(INPUT config/Dockerfile.devenv)

emb_config(INPUT config/pyproject.toml)

emb_config(INPUT config/uv.lock)

emb_config(INPUT config/cortex-m4)

#___oOo___
