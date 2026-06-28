#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __ 
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
# 
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

set -euo pipefail

# ── Required env vars ────────────────────────────────────────────────────────
: "${HOST_USER:?HOST_USER must be set}"
: "${HOST_UID:?HOST_UID must be set}"
: "${HOST_GID:?HOST_GID must be set}"

# ── Create group / user matching the host ────────────────────────────────────
groupadd --gid "${HOST_GID}" "${HOST_USER}" 2>/dev/null || true
useradd  --uid "${HOST_UID}" \
         --gid "${HOST_GID}" \
         --home-dir "/home/${HOST_USER}" \
         --create-home \
         --shell /bin/bash \
         "${HOST_USER}" 2>/dev/null || true

# Grant passwordless sudo (optional — remove if not needed)
echo "${HOST_USER} ALL=(ALL) NOPASSWD:ALL" > "/etc/sudoers.d/${HOST_USER}"
chmod 0440 "/etc/sudoers.d/${HOST_USER}"

# ── Start sshd in the foreground ─────────────────────────────────────────────
exec /usr/sbin/sshd -D -e

#___oOo___
