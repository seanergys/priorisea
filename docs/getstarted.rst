Get started
===========

Build and install
-----------------

To build the software from source, the following dependencies are required:

- cmake
- pkg-config
- jansson
- seanerbus
- flux-core
- sqlite3
- CLI11
- catch2 (v3)

Optional dependencies (used in testing):

- seanerbroker (seanerbus)
- naming-service (seanerbus)

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

PrioriSea requires Flux to be running, as it uses the Flux data-plane to handle communication between
the daemon and the CLI.
#The job manager plugin should be loaded first using ``flux jobtap load sea_priority.so``.
The database should be created first using ``priorisea database create /path/to/database.db``.
Afterward the daemon should be started with ``priorisea daemon start /path/to/database.db``.
Once this is done, you can start using PrioriSea. If you want to enable the priority plugin,
it should be loaded using ``flux jobtap load sea_priority.so`` then synced using the
``priorisea plugin sync`` command. Run that command each time you update:
- the ``job_priority`` formula
- the users score
