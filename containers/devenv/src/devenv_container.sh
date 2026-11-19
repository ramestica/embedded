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
    $(basename $0) -s -m ~/src:/home/\$USER/src

OPTIONS:
    -s, --start           Start container
    -m, --mount SRC:DST[:ro]  With --start: bind mount host path SRC (relative: to
                          the current directory) at container path DST; read-only
                          if :ro is appended; may be repeated
    -S, --shell VAL       User shell inside the container (default: /bin/zsh)
    -p, --photomatix      With --start: build the image with PhotomatixCL installed
                          (pinned to the release the current license was activated
                          against) and bind mount the license token at the two
                          paths it looks for it
    -x, --stop            Stop container
    -h, --help            Show this help message

EOF
}

# ── Check if getopt encountered an error ──────────────────────────────────────
# Tested in the `if` itself: under `set -e` a failing assignment would
# exit before any separate `$?` check ran.
if ! TEMP=$("$GETOPT_BIN" -o 'm:S:spxh' --long 'mount:,shell:,start,photomatix,stop,help' -n "$0" -- "$@"); then
    echo "Error parsing arguments. Use -h or --help for usage information." >&2
    exit 1
fi

# ── Note the quotes around "$TEMP": they are essential! ───────────────────────
eval set -- "$TEMP"
unset TEMP

IS_START=0
IS_STOP=0
IS_PHOTOMATIX=0
USER_MOUNTS=()

# ── Parse the arguments ───────────────────────────────────────────────────────
while true; do
    case "$1" in
        '-S'|'--shell')
            export USER_SHELL="$2"
            shift 2
            ;;
        '-s'|'--start')
            IS_START=1
            shift 1
            ;;
        '-m'|'--mount')
            USER_MOUNTS+=("$2")
            shift 2
            ;;
        '-p'|'--photomatix')
            IS_PHOTOMATIX=1
            shift 1
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
if [[ ${#USER_MOUNTS[@]} -gt 0 && $IS_START -eq 0 ]]; then
    echo "--mount only applies to --start" >&2
    exit 1
fi
if [[ $IS_PHOTOMATIX -eq 1 && $IS_START -eq 0 ]]; then
    echo "--photomatix only applies to --start" >&2
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

# ── SHELL assigned to user ────────────────────────────────────────────────────
if [[ -z "${USER_SHELL:-}" ]]
then
    export USER_SHELL=/bin/zsh
fi

# ── SSH port map ──────────────────────────────────────────────────────────────
declare -A ssh_ports
ssh_ports[ramestica]=8026

if [[ ! -v ssh_ports[$USER] ]]; then
    echo "error: user=$USER not mapped to a ssh port number" >&2
    exit 1
fi
export SSH_USER_PORT=${ssh_ports[$USER]}

# ── Optional sandbox volumes fragment ─────────────────────────────────────────
# Each --mount is SRC:DST[:ro]; SRC a host path bind mounted as is.
SANDBOX_VOLUME=""
for MOUNT in "${USER_MOUNTS[@]}"
do
    MODE=""
    FIELDS="${MOUNT}"
    if [[ "${FIELDS}" == *:ro ]]; then
        MODE=ro
        FIELDS="${FIELDS%:ro}"
    fi
    if [[ "${FIELDS}" != *:* ]]; then
        echo "--mount expects SRC:DST[:ro], got: ${MOUNT}" >&2
        exit 1
    fi
    IFS=':' read -r SRC DST <<< "${FIELDS}"
    # a relative SRC means from the current directory: compose would take
    # it from the compose file's directory instead
    if [[ "${SRC}" != /* ]]; then
        SRC=$(realpath -m -- "${SRC}")
    fi
    if [[ "${DST}" != /* ]]; then
        echo "--mount expects an absolute container path DST, got: ${MOUNT}" >&2
        exit 1
    fi
    SANDBOX_VOLUME+="${SANDBOX_VOLUME:+$'\n'}      - \"${SRC}:${DST}${MODE:+:${MODE}}\""
done

# ── PhotomatixCL license token (--photomatix) ─────────────────────────────────
# Bind mounted at the two paths the app reads it from; neither is covered by
# any other mount above, so without this the activation is lost once the
# (ephemeral) container is removed.
export INSTALL_PHOTOMATIX=0
if [[ $IS_PHOTOMATIX -eq 1 ]]
then
    INSTALL_PHOTOMATIX=1
    PHOTOMATIX_LICENSE="${HOME}/src/github/ramestica/color/photomatix/.clx"
    if [[ ! -f "${PHOTOMATIX_LICENSE}" ]]; then
        echo "error: --photomatix needs the license token at ${PHOTOMATIX_LICENSE}" >&2
        exit 1
    fi
    SANDBOX_VOLUME+="${SANDBOX_VOLUME:+$'\n'}      - \"${PHOTOMATIX_LICENSE}:/home/${USER}/.clx\""
    SANDBOX_VOLUME+=$'\n'"      - \"${PHOTOMATIX_LICENSE}:/home/${USER}/.local/share/photomatixcl/.clx502\""
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

if [[ $IS_PHOTOMATIX -eq 1 && "${DEVENV_TARGET_PLATFORM}" != "linux/arm64" ]]; then
    echo "error: --photomatix is pinned to the arm64 PhotomatixCL build" >&2
    exit 1
fi

# ── AI related stuff ──────────────────────────────────────────────────────────
mkdir -p ~/.claude.devenv
chmod 700 ~/.claude.devenv

# ── Files bind mounted into devenv that may be missing ───────────────────────
# Docker must not create a missing bind source (create_host_path: false, the
# default): it would create a directory where a file belongs. So a missing
# one is asked about here: create it empty, or abort. Without a terminal to
# ask on, abort.
ensure_file() {
    local file="$1" why="$2" answer
    [[ -f "${file}" ]] && return 0
    if [[ -e "${file}" ]]; then
        echo "error: ${file} exists but is not a regular file" >&2
        exit 1
    fi
    echo "${file} does not exist (${why})." >&2
    if [[ ! -t 0 ]]; then
        echo "error: no terminal to ask whether to create it; create it and retry" >&2
        exit 1
    fi
    read -r -p "Create it empty? [y/N] " answer
    if [[ "${answer}" =~ ^[Yy]([Ee][Ss])?$ ]]; then
        touch "${file}"
        echo "created empty ${file}" >&2
    else
        echo "aborted" >&2
        exit 1
    fi
}
if [[ $IS_START -eq 1 ]]
then
    ensure_file "${BUILD_DIR}/.known_hosts" \
        "the container's ~/.ssh/known_hosts, normally created by \`make devenv\`"
    ensure_file "${HOME}/.gitconfig" "mounted read-only as the container's ~/.gitconfig"
fi

# ── Generate compose file in build tree ───────────────────────────────────────
COMPOSE_FILE="${BUILD_DIR}/config/devenv_compose.yml"
cat > "${COMPOSE_FILE}" << EOF
services:
  devenv:
    image: devenv:latest
    container_name: devenv-${USER}
    hostname: devenv
    env_file:
      - ${HOME}/.claude.devenv/token.env
    build:
      context: ${BUILD_DIR}
      dockerfile: ${BUILD_DIR}/config/Dockerfile.devenv
      platforms:
        - ${DEVENV_TARGET_PLATFORM}
      args:
        INSTALL_PHOTOMATIX: "${INSTALL_PHOTOMATIX}"
    ports:
      - "${SSH_USER_PORT}:22"
    environment:
      HOST_USER: "${USER}"
      HOST_GROUP: "${USER_GROUP}"
      HOST_UID:  "${UID}"
      HOST_GID:  "${GID}"
      USER_SHELL: "${USER_SHELL}"
    volumes:
      - "${HOME}/.ssh:/home/${USER}/.ssh:ro"
      - "${BUILD_DIR}/.known_hosts:/home/${USER}/.ssh/known_hosts"
      - "${HOME}/.zshenv:/home/${USER}/.zshenv"
      - ${HOME}/.gitconfig:/home/${USER}/.gitconfig:ro
      - "${HOME}/.claude.devenv:/home/${USER}/.claude"
      - ${HOME}/.claude/CLAUDE.md:/home/${USER}/.claude/CLAUDE.md:ro
      - ${HOME}/.claude_inbox:/home/${USER}/.claude_inbox
${SANDBOX_VOLUME}
    restart: unless-stopped
EOF

# ── Launch or stop container ──────────────────────────────────────────────────
if [[ $IS_START -eq 1 ]]
then
    docker compose -f ${BUILD_DIR}/config/devenv_compose.yml up --build -d --wait || {
        echo "error: docker compose up failed" >&2
        exit 1
    }
    # because it runs detached, checking for a successful entrypoint
    # execution takes the following loop
    for i in {1..10}; do
        sleep 1
        status=$(docker inspect -f '{{.State.Status}}' devenv-${USER} 2>/dev/null)
        [[ "$status" == "running" ]] && { echo "container is running"; exit 0; }
        [[ "$status" == "exited"  ]] && break
    done
    echo "error: container exited unexpectedly" >&2
    docker logs devenv-${USER}
    exit 1
else
    docker compose -f ${BUILD_DIR}/config/devenv_compose.yml down --remove-orphans
fi

#___oOo___
