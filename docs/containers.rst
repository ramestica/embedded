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

