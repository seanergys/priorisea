Daemon
======

The PrioriSea daemon is the main component of the PrioriSea software. It fulfills three roles:

- **Express the business logic.** Formulas (:doc:`formula`), users score
  (:doc:`user`) and time windows (:doc:`timewin`) are all defined through
  the Daemon.
- **Store and read state.** Every change is persisted in the database (that should first be created)
  and every value can be read back for inspection.
- **Synchronize the plugin.** The daemon can send data to the priority plugin when a certain
  message is sent via the data-plane. This help keeping the plugin and the database consistent
  (see :doc:`architecture`).

The PrioriSea daemon and CLI have a 1-to-1 endpoint mapping (i.e. for every possible RPC call to the
daemon there exists a corresponding CLI command). The daemon commands are documented in Commands
below.

Commands
--------

The daemon is managed through the ``priorisea daemon`` subcommand.

.. code-block:: bash

    priorisea daemon start [PATH] [-s URL] [-b]
    priorisea daemon debug [--json]
    priorisea daemon update-user-period USER_PERIOD

``start`` [PATH]
    Start the daemon. The optional ``PATH`` argument sets the database path
    (default ``/var/lib/priorisea/priorisea.db``). ``-s, --seanerbus`` sets the
    SeanerBUS endpoint (default ``demobus://localhost:5398``) and ``-b,
    --background`` forks the daemon into the background, printing its PID. This
    command produces no output when run in the foreground.

    .. code-block:: bash

        priorisea daemon start /path/to/database.db

``debug``
    Print debug information about the running daemon. Without ``--json`` the
    version, SeanerBUS URL, ``t_inactive`` and ``user_period`` are shown as a
    table; with ``--json`` they are printed as a JSON object.

    .. code-block:: bash

        priorisea daemon debug

    .. code-block:: text

        Version      Seanerbus URL                 t_inactive  user_period
        1            demobus://localhost:5398      0.0         7776000.0

    With ``--json``:

    .. code-block:: json

        {
          "version": 1,
          "seanerbus_url": "demobus://localhost:5398",
          "info": {
            "t_inactive": 0.0,
            "user_period": 7776000.0
          }
        }

.. _daemon-update-user-period:

``update-user-period`` USER_PERIOD
    Update the user period, i.e. the maximum age (in seconds) of jobs
    considered when computing user scores. This command produces no output.

    .. code-block:: bash

        priorisea daemon update-user-period 7776000
