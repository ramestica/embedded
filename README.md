# embedded

`Makefile` here drives two independent things:

1. **The embedded application** (`application/`) — compiled via CMake,
   configured via Conan. Needs a C/C++ toolchain and Conan on whichever
   machine runs it.
2. **The `devenv` dev-container image** (`containers/devenv/`) — a Docker
   image with every toolchain the application needs pre-installed
   (Conan, uv, the ARM GNU cross toolchain, pinned `tmux`, etc.), reachable
   over SSH. Building/running *this* needs Docker, but explicitly does
   **not** need Conan on the host — see "Reconfiguring on host" below.

## Reconfiguring on host

```
make configure                          # TARGET=host (native), BUILD_TYPE=Release by default
make configure TARGET=cortex-m4         # cross build for the target hardware
make configure BUILD_TYPE=Debug TARGET=host
make build                              # implies configure
make install PREFIX=/opt/foo            # implies build
make compile-commands                   # merge every configured target's compile_commands.json
                                         # at the repo root, for eglot/clangd
make clean                              # clean object files for the current BUILD_TYPE/TARGET
make distclean                          # wipe build/ entirely
make help
```

Each `BUILD_TYPE`/`TARGET` pair configures into its own
`build/<BUILD_TYPE>/<TARGET>/`. `configure` first runs `conan install`
against `containers/devenv/config/conanfile.py` (the same recipe the
`devenv` image warms into its Conan cache), so **Conan must be on `PATH`
on the host** for this — install it the same way the container does:
```
curl -LsSf https://astral.sh/uv/install.sh | sh
uv tool install "conan>=2.0,<3.0"
```

`TARGET=cortex-m4` additionally needs the `arm-none-eabi` GNU toolchain on
the host. This Makefile doesn't install one — see "In-container: where the
software actually gets built" below.

## Building/running the devenv image

```
make devenv-configure     # stages Dockerfile.devenv + container scripts into
                           # build/devenv/ — no Conan/toolchain required
make lockgen-image        # builds the `lockgen` stage (devenv:lockgen), only
                           # used to regenerate lockfiles:
make update-lockfiles      #   runs `uv lock` / `conan lock create` inside
                           #   devenv:lockgen against the bind-mounted repo
```

`lockgen-image`/`update-lockfiles` are for maintaining
`containers/devenv/config/{uv.lock,conan.lock}`, not for day-to-day use.
To actually build and start the real `devenv` image:
```
build/devenv/bin/devenv_container -s <host-src-dir>:<container-dst-dir>
build/devenv/bin/devenv_container -x                # stop
build/devenv/bin/devenv_container -S /bin/zsh -s ~/src:/home/ramestica/src
```
This runs `docker compose up --build -d --wait`, which builds the `devenv`
stage of `Dockerfile.devenv` (rebuilding whenever the Dockerfile or its
staged config changes) and starts it with SSH exposed on the port mapped
for the current user in `devenv_container.sh`'s `ssh_ports` table (`8026`
for `ramestica`, matching the `Host devenv` entry in `~/.ssh/config`).
Once it's up: `ssh devenv`.

## In-container: where the software actually gets built

`make configure`/`make build` on the bare host is mainly for local
edit/compile-commands convenience (`TARGET=host`) — the host is not
guaranteed to have every dependency the application needs, and definitely
doesn't have the ARM cross toolchain the Dockerfile installs
(`Dockerfile.devenv`, "ARM bare-metal cross toolchain" step). `devenv` is
the reproducible, authoritative build environment: same Conan/uv versions,
pre-warmed Conan package cache (`Dockerfile.devenv`'s `devenv` stage runs
`conan install` for both the `default` and `cortex-m4` profiles at image
build time), and the cross toolchain already on `PATH`.

To actually build the software, once `ssh devenv`'d in, `cd` into the
mounted repo (the `<container-dst-dir>` passed to `devenv_container -s`)
and run the same `make build`/`make build TARGET=cortex-m4` as above —
it'll resolve Conan/CMake the same way, just against toolchains guaranteed
to be present and version-pinned.
