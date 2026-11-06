Time Window
===========

Time windows are recurring time frames that can influence a job score. Administrators can manage these
time windows using the ``priorisea timewin`` subcommand. Time windows are defined by:

- a unique name
- a cron expression
- a duration in seconds
- a score value [#]_

As such, each time the cron expression is triggered, the timeframe last ``duration`` seconds. If a job
is submitted to run during this timeframe, the job gets influenced by the time window score.
The sum of the time window scores can then be used to influence the job score. See :doc:`formula`.

For example, if an administrator knows that jobs ran between 12am. and 1pm. produce less
carbon footprint, they can define a time window to grant 100 points to jobs submitted during
that time:
``priorisea timewin create carbon -c "0 0 12 * * * *" -d 3600 -s 100.0``.

At its core, PrioriSea uses `croncpp <https://github.com/mariusbancila/croncpp>`__ to parse the cron expression.
As such, a six or seven fields expression is expected.

Commands
--------

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
    output. Synchronise the plugin afterwards (see :doc:`../plugin`) for the
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

.. [#] The score of time windows can be either a positive or negative value.
