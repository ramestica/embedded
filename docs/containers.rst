.. toctree::
   :maxdepth: 2

Docker containers for development
=================================

Use external drive for docker images
------------------------------------

By default Docker stores images in ~/Library/Containers/... which will
eat your internal drive. Redirect it to your external:

.. code-block:: bash

   pkill Docker             
   mkdir -p /Volumes/external/docker-data
   # edit or create the Docker daemon config:
   mkdir -p ~/.docker
   cat > ~/.docker/daemon.json <<EOF
   {
   "data-root": "/Volumes/external/docker-data"
   }
   EOF

Then restart Docker.

Xilinx ISE
----------

Download ISE 14.7. Go to the AMD downloads page (requires free
account)::

  https://www.xilinx.com/support/download/index.html/content/xilinx/en/downloadNav/vivado-design-tools/archive-ise.html

Download the "Full Installer for Linux" — it's the one you'll use even
on macOS since we're running it in Docker. It's large (~6-7GB), the
filename looks like::
  
  Xilinx_ISE_DS_Lin_14.7_1015_1.tar

Put it somewhere accessible.

Since you can't redistribute ISE, you build your own image using the
installer. The most commonly used base is:

.. code-block:: bash
                
   git clone https://github.com/baltazar-arch/xilinx-ise-docker
   cd xilinx-ise-docker
   docker build -t xilinx-ise:14.7 .

Check the repo's README for exactly where to place the installer
tarball — usually alongside the Dockerfile.

Vefiry it works:

.. code-block:: bash
                
   docker run --rm xilinx-ise:14.7 \ bash -c "source /opt/Xilinx/14.7/ISE_DS/settings64.sh && xst --help"

Orchestrated setup and workflow sequencing strategy
---------------------------------------------------

Here's the full sequence, in order, from a fresh clone:

1. First-time bootstrap — generate the lockfiles (only needed once, or whenever dependencies change):
bashmake update-lockfiles
This builds devenv:lockgen (via lockgen-image), then generates containers/devenv/config/uv.lock and conan.lock at the repo root, writing them back to your host through the bind mount. Commit both files:
bashgit add containers/devenv/config/uv.lock conan.lock
git commit -m "Add generated uv.lock and conan.lock"

2. Build the real devenv image (now that the lockfiles it depends on exist):
bashmake all
This runs cmake --build build/Release --parallel 12, which drives cmake --build build/Release --target devenv_image under the hood via your emb_container/emb_module machinery — building the devenv stage of Dockerfile.devenv, consuming uv.lock/conan.lock in frozen/pinned mode.

3. Launch the container:
bashbuild/Release/bin/start_devenv_container -s ~/src

4. Inside the container, on first login, Conan profiles (default, cortex-m4) and the Python venv are already active. For C/C++ work:

bash# Host build
conan install . --profile:build=default --profile:host=default \
    -s build_type=Release --lockfile=conan.lock --build=missing
cmake --preset conan-release
cmake --build --preset conan-release

# Cross build for TM4C123GXL
conan install . --profile:build=default --profile:host=cortex-m4 \
    -s build_type=Release --lockfile=conan.lock --build=missing
cmake --preset conan-release-cortex-m4   # confirm exact name via `cmake --list-presets`
cmake --build --preset conan-release-cortex-m4
Ongoing workflow after the first bootstrap:

Changing Python deps → edit pyproject.toml → make update-lockfiles → make all (rebuild image) → commit updated uv.lock.
Changing C/C++ deps → edit conanfile.py → make update-lockfiles → commit updated conan.lock (image itself doesn't need rebuilding for this, since conan install --lockfile happens at first login inside the running container, not at image-build time).
Day-to-day, once everything's bootstrapped: just make all + start_devenv_container — update-lockfiles is a deliberate, occasional step, not part of the normal build loop.

One thing worth deciding: do you want make all to depend on the lockfiles existing (fail early with a clear message if uv.lock/conan.lock are missing), or leave it as-is where a missing lockfile just surfaces as a COPY failure deep in the Docker build? I can add a cheap existence-check target if you'd like a clearer first-time-setup error message.
