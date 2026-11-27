CLI Reference
=============

The ``priorisea`` command-line interface and the daemon have a 1-to-1
endpoint mapping: every CLI command maps to an RPC handled by the daemon
over the Flux data-plane. Commands are grouped below by subcommand.

Daemon
------

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

Database
--------

The database is managed through the ``priorisea database`` subcommand.

.. code-block:: bash

    priorisea database create [PATH] [USER_PERIOD]

``create`` [PATH] [USER_PERIOD]
    Create the database file. The optional ``PATH`` argument sets the database
    path (default ``/var/lib/priorisea/priorisea.db``) and ``USER_PERIOD`` sets
    the maximum age (in seconds) of jobs considered when computing user scores
    (default 90 days, i.e. ``7776000`` seconds). This command produces no
    output. The path passed to ``create`` must then be reused when starting the
    daemon so that both point to the same database file:

    .. code-block:: bash

        priorisea database create /path/to/database.db
        priorisea daemon start /path/to/database.db

Plugin
------

The running Flux plugin is managed through the ``priorisea plugin`` subcommand.

.. code-block:: bash

    priorisea plugin sync
    priorisea plugin debug [--json]

``sync``
    Synchronise the plugin with the database. It propagates the current database
    state (formulas, time windows, scores) to the running plugin and should be
    run after every change to the formulas or the scores. Users with no score are
    not synced with the plugin. This command produces no output.

    .. code-block:: bash

        priorisea plugin sync

``debug``
    Print the plugin's current formula and user scores, which is useful to verify
    that the plugin is in sync with the database. Without ``--json`` the formula
    is printed (as a ``Name``/``Expression``/``Defaults`` table) followed by the
    scores as a ``UID``/``Score`` table; with ``--json`` they are printed as a
    single JSON object with the keys ``formula`` and ``scores`` (a mapping of
    user id to score).

    .. code-block:: bash

        priorisea plugin debug

    .. code-block:: text

        Name          Expression                                          Defaults
        job_priority  <var:prev_priority> * 0.5 + <var:user_score> * 0.5

        UID   Score
        1000  0.63
        1001  0.11

    With ``--json``:

    .. code-block:: json

        {
          "formula": {
            "name": "job_priority",
            "expr": "<var:prev_priority> * 0.5 + <var:user_score> * 0.5",
            "defaults": {}
          },
          "scores": {
            "1000": 0.63,
            "1001": 0.11
          }
        }

The plugin is loaded into Flux using:

.. code-block:: bash

    flux jobtap load sea_priority.so

Formula
-------

Administrators interact with the formulas through the ``priorisea formula``
subcommand. A formula must evaluate to a ``double``.

.. code-block:: bash

    priorisea formula get NAME [--json]
    priorisea formula list [NAME...] [--json]
    priorisea formula update NAME -e EXPR [-d NAME VALUE]...
    priorisea formula reset NAME...

``get`` NAME
    Print the formula named ``NAME``. ``--json`` outputs the formula as a JSON
    object with the keys ``name``, ``expr`` and ``defaults``.

    .. code-block:: bash

        priorisea formula get job_priority --json

    .. code-block:: json

        {
          "name": "job_priority",
          "expr": "<var:prev_priority> * 0.5 + <var:user_score> * 0.5",
          "defaults": {}
        }

    Without ``--json`` the formula is printed as a table:

    .. code-block:: text

        Name          Expression                                          Defaults
        job_priority  <var:prev_priority> * 0.5 + <var:user_score> * 0.5

``list`` [NAME...]
    List the configured formulas. When one or more names are given, only those
    are printed. ``--json`` outputs a JSON array (one object per formula, with
    the same keys as ``get``).

    .. code-block:: bash

        priorisea formula list

    .. code-block:: text

        Name          Expression                                          Defaults
        job_priority  <var:prev_priority> * 0.5 + <var:user_score> * 0.5
        job_score     <var:duration_ratio> + <var:timewin_score>
        job_weight    max(0, 1 - <var:age_days> / 90)

``update`` NAME
    Update the formula named ``NAME``. Sets the expression to
    evaluate and ``-d, --default`` (given as repeated ``NAME VALUE`` pairs)
    defines the value used when a variable becomes null.

    .. code-block:: bash

        priorisea formula update job_priority "<var:prev_priority> * 2" --default var:prev_priority 5

    The command produces no output on success. Synchronise the plugin afterwards
    (see :doc:`plugin`) for the change to take effect.

``reset`` NAME...
    Reset the named formula(s) to their default values. This command produces no
    output. Synchronise the plugin afterwards (see :doc:`plugin`) for the
    change to take effect.

Job
---

Job scores are managed through the ``priorisea job`` subcommand.

.. code-block:: bash

    priorisea job get JOBID [-f FORMAT] [-d] [--json]
    priorisea job list [JOBID...] [-f FORMAT] [-n N] [-o ORDER] [-s SORT] [-d] [--json]
    priorisea job update [JOBID...] [-n]

``get`` JOBID
    Print a single job. ``JOBID`` is the job identifier. ``-f, --format``
    controls how the job id is printed (``dec``, ``hex``, ``kvs``, ``dothex``,
    ``words``, ``f58`` (default), ``f58plain``). ``-d, --detail`` shows
    additional details (the score formula, its defaults and variables, the
    weight and its variables); without it only the job id, user id, inactivity
    time, score and duration are shown. ``--json`` outputs the job as a JSON
    object.

    .. code-block:: bash

        priorisea job get f2eLtp9D

    .. code-block:: text

        Job ID     User ID  t_inactive (s)    Score  Duration (s)
        f2eLtp9D   1000     1793867738.697386 0.75   120.0

    With ``-d`` the table gains the ``Score formula``, ``Score defaults``,
    ``Score variables``, ``Weight`` and ``Weight variables`` columns. With
    ``--json`` the job is printed as:

    .. code-block:: json

        {
          "jobid": "f2eLtp9D",
          "userid": 1000,
          "t_inactive": 1793867738.697386,
          "score": 0.75,
          "duration": 120.0,
          "s_expr": "<var:duration_ratio> + <var:timewin_score>",
          "s_defaults": {},
          "s_values": {},
          "weight": 0.9,
          "w_values": {}
        }

    (``weight`` and ``w_values`` are omitted from the JSON when the job was not
    used to compute its user score.)

``list`` [JOBID...]
    List jobs, ranked by inactivity time. When one or more job ids are given,
    only those are printed. ``-n, --limit`` restricts the output to the ``N``
    most recent jobs, ``-o, --order`` sets the ordering (``asc`` or ``desc``,
    default ``desc``) and ``-s, --sort`` selects the column to sort by
    (``userid``, ``t_inactive``, ``score`` or ``duration``, default
    ``t_inactive``). ``-d, --detail`` and ``-f, --format`` behave as for
    ``get``. ``--json`` outputs a JSON array.

    .. code-block:: bash

        priorisea job list -n 3

    .. code-block:: text

        Job ID     User ID  t_inactive (s)    Score  Duration (s)
        f2eLtp9D   1000     1793868997.734377 0.75   120.0
        f2bdk8Gf   1001     1793867738.697386 0.40   60.0
        fAbWk4d3u  1000     1793867738.593584 0.90   300.0

``update`` [JOBID...]
    Recompute job scores. Without arguments only the explicitly listed ``JOBID``
    values are updated; with ``-n, --new`` all jobs that became inactive since
    the last ``--new`` update are considered. This command produces no output.
    Synchronise the plugin afterwards (see :doc:`plugin`) for the change to
    take effect.

    .. code-block:: bash

        priorisea job update --new

User
----

User scores are managed through the ``priorisea user`` subcommand. Usernames are
derived from the user id (``userid``); the JSON output always uses the numeric
``userid``.

.. code-block:: bash

    priorisea user get USERNAME [-d] [--json]
    priorisea user list [USERNAME...] [-o ORDER] [--json]
    priorisea user update [USERNAME...] [-a]

``get`` USERNAME
    Print the user ``USERNAME``. ``-d, --detail`` shows additional details: the
    ``Formula`` (the ``job_weight`` formula used to aggregate the user's jobs),
    its ``Formula defaults``, ``t_executed`` (timestamp of the last executed
    job) and ``t_inactive_min`` (the user's shortest inactivity time), followed
    by the table of the user's jobs. Without ``-d`` only the username, user id
    and score are shown. Users without a score are shown with a ``/`` in the
    score column. ``--json`` outputs the user (and, with ``-d``, its jobs) as a
    JSON object.

    .. code-block:: bash

        priorisea user get alice -d

    .. code-block:: text

        Username  User ID  Score  Formula                         Formula defaults  t_executed  t_inactive_min
        alice     1000     0.63   max(0, 1 - <var:age_days> / 90)                   1700000000  42.5

    With ``--json`` the user is printed as:

    .. code-block:: json

        {
          "user": {
            "userid": 1000,
            "score": 0.63,
            "w_expr": "max(0, 1 - <var:age_days> / 90)",
            "w_defaults": {},
            "t_executed": 1700000000.0,
            "t_inactive_min": 42.5
          },
          "jobs": [
            {
              "jobid": "72057594037936128",
              "userid": 1000,
              "t_inactive": 42.5,
              "score": 0.75,
              "duration": 120.0,
              "s_expr": "<var:duration_ratio> + <var:timewin_score>",
              "s_defaults": {},
              "s_values": {},
              "weight": 0.9,
              "w_values": {}
            }
          ]
        }

``list`` [USERNAME...]
    List users. When one or more usernames are given, only those are printed.
    ``-o, --order`` sets the ordering (``asc`` or ``desc``, default ``desc``).
    Users without a score are shown with a ``/`` in the score column. ``--json``
    outputs a JSON array (each entry uses the same keys as ``get``'s ``user``
    object).

    .. code-block:: bash

        priorisea user list

    .. code-block:: text

        Username  User ID  Score
        alice     1000     0.63
        bob       1001     /

``update`` [USERNAME...]
    Recompute user scores. Without arguments only the explicitly listed users
    are updated; with ``-a, --all`` every user is updated. This command produces
    no output. Synchronise the plugin afterwards (see :doc:`plugin`) for the
    change to take effect.

    .. code-block:: bash

        priorisea user update --all

Time window
-----------

Time windows are managed through the ``priorisea timewin`` subcommand.

.. code-block:: bash

    priorisea timewin get NAME [--json]
    priorisea timewin list [NAME...] [--json]
    priorisea timewin create NAME -c CRON -d DURATION -s SCORE
    priorisea timewin update NAME -c CRON -d DURATION -s SCORE
    priorisea timewin delete NAME...

``get`` NAME
    Print the time window named ``NAME``. ``--json`` outputs the time window as
    a JSON object with the keys ``name``, ``cron``, ``duration`` (an integer, in
    seconds) and ``score``.

    .. code-block:: bash

        priorisea timewin get carbon --json

    .. code-block:: json

        {
          "name": "carbon",
          "cron": "0 0 12 * * * *",
          "duration": 3600,
          "score": 100.0
        }

``list`` [NAME...]
    List the configured time windows. When one or more names are given, only
    those are printed. Without ``--json`` the windows are printed as a table:

    .. code-block:: bash

        priorisea timewin list

    .. code-block:: text

        Name    Cron             Duration  Score
        carbon  0 0 12 * * * *   3600      100.0

``create`` NAME
    Create a time window. ``NAME`` is its unique name; ``-c, --cron`` sets the
    cron expression, ``-d, --duration`` the duration in seconds and ``-s,
    --score`` the score; all three are required. This command produces no
    output. Synchronise the plugin afterwards (see :doc:`plugin`) for the
    change to take effect.

    .. code-block:: bash

        priorisea timewin create carbon -c "0 0 12 * * * *" -d 3600 -s 100.0

``update`` NAME
    Update an existing time window named ``NAME``. ``-c, --cron``, ``-d,
    --duration`` and ``-s, --score`` are all required and replace the stored
    values. This command produces no output.

    .. code-block:: bash

        priorisea timewin update carbon -c "0 0 12 * * * *" -d 7200 -s 50.0

``delete`` NAME...
    Delete the named time window(s). This command produces no output.

    .. code-block:: bash

        priorisea timewin delete carbon
