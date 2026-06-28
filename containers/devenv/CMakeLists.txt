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

#
# Optional generated config file
#
emb_config(
    INPUT  config/devenv_compose.yml.in
    OUTPUT devenv_compose.yml
)

emb_config(
    INPUT config/Dockerfile.devenv
)

#
# Scripts belonging to this module
#
emb_script(
    FILE src/devenv_entrypoint.sh
)

emb_script(FILE src/start_devenv_container.sh.in)

#
# Container build target
#
emb_container(
    NAME image
    COMMAND ${CMAKE_BINARY_DIR}/bin/start_devenv_container
)

#
# (Optional) install raw config files if needed
#
emb_config(
    INPUT  config/Dockerfile.devenv
)
