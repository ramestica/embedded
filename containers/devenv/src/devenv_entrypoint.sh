# ******************************************************************************
#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __ 
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
# 
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0
#
# ******************************************************************************

set -euo pipefail

: "${HOST_USER:?HOST_USER must be set}"
: "${HOST_GROUP:?HOST_GROUP must be set}"
: "${HOST_UID:?HOST_UID must be set}"
: "${HOST_GID:?HOST_GID must be set}"

groupadd --gid "${HOST_GID}" ${HOST_GROUP} 2>/dev/null || true
useradd  --uid "${HOST_UID}" \
         --gid "${HOST_GID}" \
         --home-dir "/home/${HOST_USER}" \
         --create-home \
         --shell /bin/zsh \
         "${HOST_USER}" 2>/dev/null || true

chown "$HOST_UID:$HOST_GID" "/home/$HOST_USER"

# XDG base dirs aren't created by useradd/skel, but some GUI apps (e.g.
# luminance-hdr's sqlite db) assume they already exist and won't create
# missing parent directories themselves.
mkdir -p "/home/${HOST_USER}/.local/share" "/home/${HOST_USER}/.config"
chown "$HOST_UID:$HOST_GID" "/home/${HOST_USER}/.local" "/home/${HOST_USER}/.local/share" "/home/${HOST_USER}/.config"

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

# ── useradd copies skel files only if user dir does not exist already ───────
if [[ "$(basename $USER_SHELL)" == "bash" ]]
then 
    SKEL_DIR=/etc/skel
    SKEL_FILES=".bashrc .profile"
elif [[ "$(basename $USER_SHELL)" == "zsh" ]]
then
    SKEL_DIR=/etc/skel
    SKEL_FILES=".zshrc"
fi
for i in ${SKEL_FILES} 
do
    dest_name="${i#.}"         # strip a leading dot if present (no-op otherwise)
    dest_name=".${dest_name}"  # ensure exactly one leading dot
    cp "${SKEL_DIR}/${i}" "/home/${HOST_USER}/${dest_name}"
    chown "${HOST_UID}:${HOST_GID}" "/home/${HOST_USER}/${dest_name}"
done

# ── Per shell tweaks ──────────────────────────────────────────────────────────
case $(basename ${USER_SHELL}) in
    'zsh')
        sed -i "1i# tramp requires the following when remote shell is zsh" /home/${HOST_USER}/.zshrc
        sed -i "1a[[ \$TERM == "dumb" ]] && unsetopt zle && PS1='$ ' && return" /home/${HOST_USER}/.zshrc
        echo "RPROMPT='%{%F{142}%}[%{%F{214}%}devenv%{%F{142}%}]%{%f%}'" >> /home/${HOST_USER}/.zshrc
        su - "${HOST_USER}" -c "cp -r /etc/skel/.oh-my-zsh /home/${HOST_USER}/.oh-my-zsh"
    ;;
esac

usermod -p '*' "${HOST_USER}"

echo "${HOST_USER} ALL=(ALL) NOPASSWD:ALL" > "/etc/sudoers.d/${HOST_USER}"
chmod 0440 "/etc/sudoers.d/${HOST_USER}"

exec /usr/sbin/sshd -D -e

#___oOo___
