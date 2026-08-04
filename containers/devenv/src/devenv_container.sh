#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __ 
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
# 
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

# a shared group between host and container; which makes possible
# permissions sharing. on macos do the following 
# sudo dseditgroup -o create -i 2000 -r "devenv container group" devenv
# sudo dseditgroup -o edit -a ramestica -t user devenv

set -euo pipefail

show_help() {
    cat << EOF
Usage:
    $(basename $0) [OPTIONS]

SYNOPSYS:
    $(basename $0) -s ~/src

OPTIONS:
    -s, --start SRC:DST  Start container with host and container paths to bind mount
    -x, --stop           Stop container
    -h, --help           Show this help message

EOF
}

TEMP=$("$GETOPT_BIN" -o 's:xh' --long 'start:,stop,help' -n "$0" -- "$@")

# ── Check if getopt encountered an error ──────────────────────────────────────
if [ $? -ne 0 ]; then
    echo "Error parsing arguments. Use -h or --help for usage information." >&2
    exit 1
fi

# ── Note the quotes around "$TEMP": they are essential! ───────────────────────
eval set -- "$TEMP"
unset TEMP

IS_START=0
IS_STOP=0

# ── Parse the arguments ───────────────────────────────────────────────────────
while true; do
    case "$1" in
        '-s'|'--start')
            IS_START=1
            if [[ -n "${2:-}" && "$2" != '--' && "$2" != -* ]]; then
                export USER_SANDBOX_PAIR="$2"
                shift 2
            else
                shift 1   # only consume '-s'/'--start'; leave whatever's next
                # (nothing, '--', or another flag) for the outer loop
            fi
            ;;
        '-x'|'--stop')
            IS_STOP=1
            shift
            ;;
        '-h'|'--help')
            show_help
            exit 0
            ;;
        '--')
            shift
            break
            ;;
        *)
            echo "Unknown option: $1" >&2
            exit 1
            ;;
    esac
done

# ── mutual exclusion check ────────────────────────────────────────────────────
if [[ $IS_START -eq 1 && $IS_STOP -eq 1 ]]; then
    echo "commanded to stop and start the container at the same time" >&2
    exit 1
fi

# ── Substituted by CMake at configure time ────────────────────────────────────
BUILD_DIR=$(realpath $(dirname ${BASH_SOURCE[0]})/..)

# ── Resolve UID/GID ───────────────────────────────────────────────────────────
if [ -z "${UID:-}" ]
then
    UID=$(id -u)
fi
USER_GROUP=devenv
GID=2000
export UID GID USER_GROUP

# ── SSH port map ──────────────────────────────────────────────────────────────
declare -A ssh_ports
ssh_ports[ramestica]=8026

if [[ ! -v ssh_ports[$USER] ]]; then
    echo "error: user=$USER not mapped to a ssh port number" >&2
    exit 1
fi
export SSH_USER_PORT=${ssh_ports[$USER]}

# ── Optional sandbox volume fragment ──────────────────────────────────────────
if [[ -n "${USER_SANDBOX_PAIR:-}" ]]
then
    if [[ "${USER_SANDBOX_PAIR}" != *:* ]]; then
        echo "expected colon-separated sandbox pait, got: $${USER_SANDBOX_PAIR}" >&2
        exit 1
    fi
    IFS=':' read -r SRC DST <<< "${USER_SANDBOX_PAIR}"
    SANDBOX_ENVIRONMENT="      USER_SANDBOX: \"${DST}\""
    SANDBOX_VOLUME="      - \"${SRC}:${DST}\""
else
    SANDBOX_ENVIRONMENT=""
    SANDBOX_VOLUME=""
fi

# ── target platform on which the container runs ────────────────────────────────
HOST_PROCESSOR=$(uname -m)
case "$HOST_PROCESSOR" in
    arm64|aarch64)
        DEVENV_TARGET_PLATFORM="linux/arm64"
        ;;
    x86_64|AMD64|amd64)
        DEVENV_TARGET_PLATFORM="linux/amd64"
        ;;
    *)
        echo "WARNING: Unrecognized host processor '$HOST_PROCESSOR' — defaulting DEVENV_TARGET_PLATFORM to linux/amd64" >&2
        DEVENV_TARGET_PLATFORM="linux/amd64"
        ;;
esac
echo "devenv container target platform: ${DEVENV_TARGET_PLATFORM}"

# ── Generate compose file in build tree ───────────────────────────────────────
COMPOSE_FILE="${BUILD_DIR}/config/devenv_compose.yml"
cat > "${COMPOSE_FILE}" << EOF
services:
  devenv:
    image: devenv:latest
    container_name: devenv-${USER}
    hostname: devenv
    build:
      context: ${BUILD_DIR}
      dockerfile: ${BUILD_DIR}/config/Dockerfile.devenv
      platforms:
        - ${DEVENV_TARGET_PLATFORM}
    ports:
      - "${SSH_USER_PORT}:22"
    environment:
      HOST_USER: "${USER}"
      HOST_GROUP: "${USER_GROUP}"
      HOST_UID:  "${UID}"
      HOST_GID:  "${GID}"
${SANDBOX_ENVIRONMENT}
    volumes:
      - "${HOME}/.ssh:/home/${USER}/.ssh:ro"
      - "${BUILD_DIR}/.known_hosts:/home/${USER}/.ssh/known_hosts"
      - "${HOME}/.oh-my-zsh:/home/${USER}/.oh-my-zsh"
      - "${HOME}/.zshenv:/home/${USER}/.zshenv"
      - "${HOME}/.zshrc:/home/${USER}/.zshrc"
      - ${HOME}/.gitconfig:/home/${USER}/.gitconfig:ro
${SANDBOX_VOLUME}
    restart: unless-stopped
EOF
    
# ── Launch or stop container ──────────────────────────────────────────────────
if [[ $IS_START -eq 1 ]]
then
    docker compose -f ${BUILD_DIR}/config/devenv_compose.yml up --build -d
    # because it runs detached, checking for a successful entrypoint
    # execution takes the following loop
    for i in {1..10}; do
        sleep 1
        status=$(docker inspect -f '{{.State.Status}}' atacsw-devenv-${USER} 2>/dev/null)
        [[ "$status" == "running" ]] && { echo "container is running"; exit 0; }
        [[ "$status" == "exited"  ]] && break
    done
    echo "error: container exited unexpectedly" >&2
    docker logs atacsw-devenv-${USER}
    exit 1
else
    docker compose -f ${BUILD_DIR}/config/devenv_compose.yml down
fi

#___oOo___
