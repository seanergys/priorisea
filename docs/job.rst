Job Score
=========

To reflect the performance of a job, a job score can be computed post-run. A custom
formula is used to compute the job score. See :doc:`../formula`.

The job scores can be computed/updated using ``priorisea job update`` [#]_. The ``--new``
flag can be passed to the update command. Internally, PrioriSea keeps the date since the last
``priorisea job update --new`` command. This date is used to update the score of jobs who finished
running since the last time the ``--new`` flag was used.

Limitations
-----------

Since computing a job score can be computationally expensive, the job score is not computed
immediately after a job finishes running. Instead, it should be updated periodically using the
``priorisea job update`` command. Moreover, updating the job score formula does not affect
the job score of jobs that have already been computed. To update them again, the list of job ids
to update can be passed directly to the ``priorisea job update`` command.

.. [#] The score of the user who ran the job is derived from the job scores and is
   updated separately. See :doc:`../user`

Commands
--------

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
    Synchronise the plugin afterwards (see :doc:`../plugin`) for the change to
    take effect.

    .. code-block:: bash

        priorisea job update --new
