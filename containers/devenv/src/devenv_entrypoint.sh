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

: "${HOST_USER:?HOST_USER must be set}"
: "${HOST_UID:?HOST_UID must be set}"
: "${HOST_GID:?HOST_GID must be set}"

groupadd --gid "${HOST_GID}" "${HOST_USER}" 2>/dev/null || true
useradd  --uid "${HOST_UID}" \
         --gid "${HOST_GID}" \
         --home-dir "/home/${HOST_USER}" \
         --create-home \
         --shell /bin/bash \
         "${HOST_USER}" 2>/dev/null || true

cp -rn /etc/skel/. "/home/${HOST_USER}/" 2>/dev/null || true

for entry in /etc/skel/.[!.]* /etc/skel/*; do
    name="$(basename "$entry")"
    target="/home/${HOST_USER}/${name}"
    [ -e "$target" ] && chown -R "${HOST_UID}:${HOST_GID}" "$target"
done

# Seed a per-user, writable Conan cache from the baked shared cache via
# hardlink copy: instant, zero extra disk for unchanged files, and every
# new package the user builds writes independently without touching the
# shared original.
USER_CONAN_HOME="/home/${HOST_USER}/.conan2"
if [ ! -d "${USER_CONAN_HOME}" ]; then
    cp -al /usr/local/share/conan2 "${USER_CONAN_HOME}"
    chown -R "${HOST_UID}:${HOST_GID}" "${USER_CONAN_HOME}"
fi

sed -i "1iexport USER_SANDBOX=/home/${HOST_USER}/sandbox" "/home/${HOST_USER}/.bashrc"

usermod -p '*' "${HOST_USER}"

echo "${HOST_USER} ALL=(ALL) NOPASSWD:ALL" > "/etc/sudoers.d/${HOST_USER}"
chmod 0440 "/etc/sudoers.d/${HOST_USER}"

exec /usr/sbin/sshd -D -e

#___oOo___
