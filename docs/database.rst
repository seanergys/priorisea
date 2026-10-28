Database
========

All PrioriSea operations are persisted in an SQLite database administrated by the
daemon. The database stores the configuration and the state required to compute
priorities:

- the scoring formulas (``job_priority``, ``job_score``, ``job_weight``)
- the recurring time windows
- the jobs and their scores
- the users and their scores
- the database info, namely the inactivity threshold and the user period

The database is created once using the ``priorisea database`` subcommand:

.. code-block:: bash

    priorisea database create /path/to/database.db

The path passed to ``priorisea database create`` must then be reused when starting
the daemon so that both point to the same database file:

.. code-block:: bash

    priorisea daemon start /path/to/database.db

The user period (the maximum age of jobs considered when computing user scores) is
set when creating the database and can later be changed with
``priorisea daemon update-user-period``.
