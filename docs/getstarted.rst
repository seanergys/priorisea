Get started
===========

Build and install
-----------------

To build the software from source, the following dependencies are required:

- cmake
- jansson
- libfmt
- seanerbus
- flux-core
- flux-security

Optional dependencies (used in testing):

- demobus
- seanerbus' naming-service

The software can then be built by running:

.. code-block:: bash

    git submodule update --init --recursive
    cmake -S . -B build
    cmake --build build -j
    ctest --test-dir build
    cmake --install build --prefix=/your/install/path

A ``Dockerfile`` is also available to build the software in a container.

First steps
-----------

PrioriSea requires Flux to be running, as it uses the Flux KVS to persist data. The job manager
plugin should be loaded first using ``flux jobtap load sea_priority.so``. Afterward the database
and the plugin memory can be populated using ``priorisea init``. This defines the default
formulas (see :doc:`../formula`) and the default decay expression (see :doc:`../userscore`). This
is verified by running ``priorisea formula list`` and ``priorisea decay get``.
