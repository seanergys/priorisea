Command Line Interface
======================

The ``priorisea`` command-line interface (CLI) is a layer of abstraction between the administrator and the daemon,
to help humans communicate with the daemon

The following command tree is available:

- :ref:`daemon <cli-daemon>`
- :ref:`database <cli-database>`
- :ref:`formula <cli-formula>`
- :ref:`job <cli-job>`
- :ref:`plugin <cli-plugin>`
- :ref:`timewin <cli-timewin>`
- :ref:`user <cli-user>`

.. _cli-daemon:

daemon
------

Manage the daemon (see :doc:`daemon`).

.. code-block:: bash

    priorisea daemon start [PATH] [-s URL] [-b]
    priorisea daemon debug [--json]
    priorisea daemon update-user-period USER_PERIOD

``start`` [PATH]
    Start the daemon. The optional ``PATH`` argument sets the database path
    (default ``/var/lib/priorisea/priorisea.db``). ``-s, --seanerbus`` sets the
    SeanerBUS endpoint (default ``demobus://localhost:5398``) and ``-b,
    --background`` forks the daemon into the background, printing its PID.
``debug``
    Print debug information about the running daemon. Without ``--json`` the
    version, SeanerBUS URL, ``t_inactive`` and ``user_period`` are shown as a
    table; with ``--json`` they are printed as a JSON object.
``update-user-period`` USER_PERIOD
    Update the user period, i.e. the maximum age (in seconds) of jobs
    considered when computing user scores.

.. _cli-database:

database
--------

Manage the database.

.. code-block:: bash

    priorisea database create [PATH] [USER_PERIOD]

``create`` [PATH] [USER_PERIOD]
    Create the database file. The optional ``PATH`` argument sets the database
    path (default ``/var/lib/priorisea/priorisea.db``) and ``USER_PERIOD`` sets
    the maximum age (in seconds) of jobs considered when computing user scores
    (default 90 days).

.. _cli-formula:

formula
-------

Manage the scoring formulas (see :doc:`formula`).

.. code-block:: bash

    priorisea formula get FORMULA [--json]
    priorisea formula list [FORMULA...] [--json]
    priorisea formula update FORMULA -e EXPR [-d NAME VALUE]...
    priorisea formula reset FORMULA...

``get`` FORMULA
    Get the formula named on the command line.
``list`` [FORMULA...]
    List the configured formulas. When one or more formula names are given,
    only those are printed.
``update`` FORMULA EXPR
    Update a formula. ``FORMULA`` is the formula name and ``-e, --expr`` the
    expression to evaluate. Use ``-d, --default`` (given as repeated
    ``NAME VALUE`` pairs) to define the default value used when a variable
    becomes null.

    .. code-block:: bash

        priorisea formula update job_priority "<var:prev_priority> * 2" --default var:prev_priority 5
``reset`` FORMULA...
    Reset the named formula(s) to their default values.

.. _cli-job:

job
---

Manage jobs.

.. code-block:: bash

    priorisea job get JOBID [-f FORMAT] [--json]
    priorisea job list [JOBID...] [-f FORMAT] [-n N] [--json]
    priorisea job update [JOBID...] [-n]

``get`` JOBID
    Print a single job's score. ``-f, --format`` controls how the job id is
    printed (``dec``, ``hex``, ``kvs``, ``dothex``, ``words``, ``f58``
    (default), ``f58plain``). ``--json`` outputs the job as a JSON object.
``list`` [JOBID...]
    List job scores. When one or more job ids are given, only those are
    printed. ``-f, --format`` formats the job ids as for ``get`` and
    ``-n, --limit`` restricts the output to the ``N`` most recent jobs ranked
    by inactivity time. ``--json`` outputs a JSON array.
``update`` [JOBID...]
    Recompute job scores. Without arguments only the explicitly listed
    ``JOBID`` values are updated; with ``-n, --new`` all jobs that became
    inactive since the last ``--new`` update are considered.

.. _cli-plugin:

plugin
------

Manage the running Flux plugin.

.. code-block:: bash

    priorisea plugin sync
    priorisea plugin debug [--json]

``sync``
    Synchronise the plugin with the database.
``debug``
    Print the plugin's current formula and user scores. Without ``--json`` the
    formula is printed followed by the scores as a table; with ``--json`` they
    are printed as a single JSON object.

.. _cli-timewin:

timewin
-------

Manage the recurring time windows that influence the job score (see
:doc:`timewin`).

.. code-block:: bash

    priorisea timewin get NAME [--json]
    priorisea timewin list [NAME...] [--json]
    priorisea timewin create NAME -c CRON -d DURATION -s SCORE
    priorisea timewin update NAME -c CRON -d DURATION -s SCORE
    priorisea timewin delete NAME...

``get`` NAME
    Get the time window named on the command line.
``list`` [NAME...]
    List the configured time windows. When one or more names are given, only
    those are printed.
``create`` NAME
    Create a time window. ``-c, --cron`` sets the cron expression, ``-d,
    --duration`` the duration in seconds and ``-s, --score`` the score; all
    three are required.
``update`` NAME
    Update an existing time window. ``-c, --cron``, ``-d, --duration`` and
    ``-s, --score`` are all required and replace the stored values.
``delete`` NAME...
    Delete the named time window(s).

.. _cli-user:

user
----

Manage users.

.. code-block:: bash

    priorisea user get USERNAME [--json]
    priorisea user list [USERNAME...] [--json]
    priorisea user update [USERNAME...] [-a]

``get`` USERNAME
    Print a single user's score.
``list`` [USERNAME...]
    List users. When one or more usernames are given, only those are printed.
    Users without a score are shown with a ``/`` in the score column.
    ``--json`` outputs a JSON array.
``update`` [USERNAME...]
    Recompute user scores. Without arguments only the explicitly listed users
    are updated; with ``-a, --all`` every user is updated.
