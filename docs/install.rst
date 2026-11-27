Install
=======

PrioriSea is available as a **Spack** package `priorisea` on `SeanerStack
<https://mirror-gitlab.seanergys.fz-juelich.de/software/seanerstack>`__

On **Docker**, the software can be built from the ``Dockerfile`` `in the repository
<https://gitlab.seanergys.fz-juelich.de/software/priorisea/-/blob/main/Dockerfile?ref_type=heads>`__

To **build from source**, the following dependencies are required:

- cmake
- pkg-config
- jansson
- seanerbus
- flux-core
- sqlite3
- CLI11
- catch2 (v3)

Optional dependencies (used in testing):

- demobus (seanerbus)
- naming-service (seanerbus)

The software can then be built by running:

.. code-block:: bash

    git submodule update --init --recursive
    cmake -S . -B build
    cmake --build build -j
    ctest --test-dir build
    cmake --install build --prefix=/your/install/path
