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

GETOPT_BIN="getopt"
if [[ "$OSTYPE" == "darwin"* ]]; then
    GETOPT_BIN="$(brew --prefix gnu-getopt)/bin/getopt"
fi

show_help() {
    cat << EOF
Usage:
    $(basename $0) [OPTIONS]

SYNOPSYS:
    $(basename $0) -s ~/src

OPTIONS:
    -s, --sabndbox VAL      Host path to user's sandbox
    -h, --help              Show this help message

EOF
}

TEMP=$("$GETOPT_BIN" -o 's:h' --long 'sandbox:,help' -n "$0" -- "$@")

# Check if getopt encountered an error
if [ $? -ne 0 ]
then
    echo "Error parsing arguments. Use -h or --help for usage information." >&2
    exit 1
fi

# Note the quotes around "$TEMP": they are essential!
eval set -- "$TEMP"
unset TEMP

# Parse the arguments
while true; do
    case "$1" in
        '-s'|'--sandbox')
            export USER_SANDBOX=$(realpath "$2")
            shift 2
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
            echo "Internal error! Unknown option: $1" >&2
            exit 1
            ;;
    esac
done

declare -A ssh_ports
ssh_ports[ramestica]=8026

export SSH_USER_PORT=${ssh_ports[$USER]}

if [[ -z "${USER_SANDBOX:-}" ]]
then
    echo "USER_SANDBOX is unset or empty"
    exit 1
fi
if [ ! -d ${USER_SANDBOX} ]
then
    echo USER_SANDBOX=${USER_SANDBOX} is not a directory
    exit 1
fi

if [ -z "${UID:-}" ]
then
    UID=$(id -u)
fi
USER_GROUP=devenv
GID=2000
export UID GID USER_GROUP

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
docker compose -f ${SCRIPT_DIR}/config/devenv_compose.yml up --build -d

#___oOo___
